// Copyright (c) Meta Platforms, Inc. and affiliates.

#include "CameraHandInput.h"

#include "OculusXRInputFunctionLibrary.h"
#include "Components/PoseableMeshComponent.h"
#include "HandPose.h"
#include "QuatUtil.h"

#define ConvertBoneToFinger UOculusXRInputFunctionLibrary::ConvertBoneToFinger
#define GetFingerTrackingConfidence UOculusXRInputFunctionLibrary::GetFingerTrackingConfidence

UCameraHandInput::UCameraHandInput(FObjectInitializer const& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PostPhysics;
	bIsActive = false;

	for (auto& Quat : BoneRotations)
	{
		Quat = FQuat::Identity;
	}
	for (auto& Quat : BoneVelocities)
	{
		Quat = FQuat::Identity;
	}
	for (auto& Time : BoneLastFrozenTimes)
	{
		Time = -99999;
	}
}

void UCameraHandInput::BeginPlay()
{
	Super::BeginPlay();
}

void UCameraHandInput::SetHand(EControllerHand InHand)
{
	Hand = InHand == EControllerHand::Left ? EOculusXRHandType::HandLeft : EOculusXRHandType::HandRight;
}

void UCameraHandInput::SetUpBoneMap(UPoseableMeshComponent* HandMesh, TArray<FHandBoneMapping>& BoneMap)
{
	if (HandMesh == nullptr || HandMesh->SkeletalMesh == nullptr)
		return;
	
	// set the bone ids for fast lookup
	for (auto& BoneMapping : BoneMap)
	{
		auto BoneName = BoneMapping.BoneName;
		auto const BoneIndex = HandMesh->SkeletalMesh->RefSkeleton.FindBoneIndex(BoneName);
		BoneMapping.BoneId = BoneIndex;

		if (BoneIndex < 0)
		{
			UE_LOG(LogHandInput, Warning, TEXT("Hand Tracking: Unable to find bone named '%s' in hand skeleton."),
				*BoneMapping.BoneName.ToString());
		}
		else
		{
			BoneMapping.ReferenceTransform = HandMesh->SkeletalMesh->RefSkeleton.GetRefBonePose()[BoneIndex];
		}
	}
}

void UCameraHandInput::SetPoseableMeshComponent(UPoseableMeshComponent* PoseableMeshComponent)
{
	HandMesh = PoseableMeshComponent;
	UpdateMeshVisibility();

	if (ensureMsgf(HandMesh, TEXT("SetPoseableMeshComponent failed")) && ensureMsgf(HandMesh->SkeletalMesh,
		TEXT("SetPoseableMeshComponent failed")))
	{
		SetUpBoneMap(HandMesh, BoneMap);
		GripBoneId = HandMesh->SkeletalMesh->RefSkeleton.FindBoneIndex(GripBoneName);
		OnInitializeMesh.Broadcast(this);
	}
}

void UCameraHandInput::SetupInput(UInputComponent* Input)
{
	if (Hand == EOculusXRHandType::HandLeft)
	{
		Input->BindAxisKey("OculusHand_Left_IndexPinchStrength", this, &UCameraHandInput::IndexFingerPinchUpdate);
		Input->BindAxisKey("OculusHand_Left_MiddlePinchStrength", this, &UCameraHandInput::MiddleFingerPinchUpdate);
		Input->BindAxisKey("OculusHand_Left_RingPinchStrength", this, &UCameraHandInput::RingFingerPinchUpdate);
		Input->BindAxisKey("OculusHand_Left_PinkPinchStrength", this, &UCameraHandInput::PinkyFingerPinchUpdate);
	}
	else
	{
		Input->BindAxisKey("OculusHand_Right_IndexPinchStrength", this, &UCameraHandInput::IndexFingerPinchUpdate);
		Input->BindAxisKey("OculusHand_Right_MiddlePinchStrength", this, &UCameraHandInput::MiddleFingerPinchUpdate);
		Input->BindAxisKey("OculusHand_Right_RingPinchStrength", this, &UCameraHandInput::RingFingerPinchUpdate);
		Input->BindAxisKey("OculusHand_Right_PinkPinchStrength", this, &UCameraHandInput::PinkyFingerPinchUpdate);
	}

	bInputIsInitialized = true;
}

bool UCameraHandInput::IsActive()
{
	return UOculusXRInputFunctionLibrary::IsHandTrackingEnabled();
}

void UCameraHandInput::TickComponent(float DeltaTime, ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// there must be a better way to do this that isn't polling
	if (!bInputIsInitialized)
	{
		if (auto const InputComponent = GetOwner()->InputComponent)
		{
			SetupInput(InputComponent);
		}
	}

	auto const bIsActiveThisFrame = IsActive();
	if (bIsActive != bIsActiveThisFrame)
	{
		bIsActive = bIsActiveThisFrame;
		UpdateMeshVisibility();
	}

	if (!bIsActiveThisFrame)
	{
		return;
	}

	UpdateTracking();
	UpdateSkeleton();
	if (bAlwaysUpdateGrab || IsTracked())
	{
		UpdateGrabInput();
	}
	UpdatePointingInput();

	if (HandMesh)
	{
		for (auto&& BoneMapping : BoneMap)
		{
			BoneCache[BoneMapping.MappedBone] = HandMesh->GetSocketTransform(BoneMapping.BoneName);
		}
	}

	/*
	float Size = GetGrippingAxis() * 20.0f;
	FColor Color = IsPinching() ? FColor::Green : FColor::Red;
	DrawDebugSphere(GetWorld(), HandComponent->GetComponentLocation(), Size, 32, Color);
	*/

	// GEngine->AddOnScreenDebugMessage(-1, 0, FColor::Yellow, FString::Printf(TEXT("%i %f"), bInputIsInitialized ? 1 : 0, GetHighestPinchValue()));
}

void UCameraHandInput::UpdateTracking()
{
	auto const IsTrackedThisFrame = IsTracked();
	if (IsTrackedThisFrame && !WasTrackedLastFrame)
	{
		TimeWhenTrackingLastGained = GetWorld()->GetTimeSeconds();
	}

	WasTrackedLastFrame = IsTrackedThisFrame;
}

bool UCameraHandInput::IsTracked() const
{
	return UOculusXRInputFunctionLibrary::GetTrackingConfidence(Hand) == EOculusXRTrackingConfidence::High;
}

void UCameraHandInput::FilterBoneRotation(EOculusXRBone Bone, FQuat LastRotation, FQuat& Rotation)
{
	auto const Now = GetWorld()->GetTimeSeconds();
	auto& LastFrozenTime = BoneLastFrozenTimes[Bone];

	auto& LastVelocity = BoneVelocities[Bone];
	auto const Finger = ConvertBoneToFinger(Bone);

	auto const ActualAngularDistance = LastRotation.AngularDistance(Rotation);
	if (MaxBoneSmoothingAngularDistance > ActualAngularDistance)
	{
		auto const Alpha = FMath::Max(FMath::GetRangePct(MinBoneAngularDistance, MaxBoneSmoothingAngularDistance,
			ActualAngularDistance), 0.0f);
		Rotation = FQuat::Slerp(LastRotation, Rotation, Alpha);
		LastFrozenTime = Now;
	}
	else
	{
		auto const Alpha = FMath::Clamp((Now - LastFrozenTime) / BoneUnfreezeTime, 0.0f, 1.0f);
		Rotation = FQuat::Slerp(LastRotation, Rotation, Alpha);
	}

	if (bAlwaysClampBoneSpeed || GetFingerTrackingConfidence(Hand, Finger) != EOculusXRTrackingConfidence::High)
	{
		auto const DeltaSeconds = GetWorld()->GetDeltaSeconds();
		auto const AngularDistance = LastRotation.AngularDistance(Rotation);
		auto const MaxAngularDistance = MaxBoneAngularSpeed * DeltaSeconds;
		if (MaxAngularDistance < AngularDistance)
		{
			Rotation = Scale(LastVelocity, DeltaSeconds) * LastRotation;
			LastVelocity = Scale(LastVelocity, BoneVelocityDamping);
		}
		else
		{
			LastVelocity = Scale(Rotation * LastRotation.Inverse(), 1.0f / DeltaSeconds);
		}
	}
}

void UCameraHandInput::SetBoneRotation(UPoseableMeshComponent* HandMesh, FHandBoneMapping BoneMapping,
	FQuat BoneRotation)
{
	if (BoneMapping.BoneId == -1 || HandMesh == nullptr || HandMesh->BoneSpaceTransforms.Num() <= BoneMapping.BoneId)
	{
		return;
	}

	BoneRotation = BoneMapping.RotationOffset * BoneRotation;
	BoneRotation.Normalize();
	HandMesh->BoneSpaceTransforms[BoneMapping.BoneId] = FTransform(
		BoneRotation,
		BoneMapping.ReferenceTransform.GetTranslation(),
		BoneMapping.ReferenceTransform.GetScale3D());

	if (BoneMapping.MappedBone == EOculusXRBone::Wrist_Root)
	{
		HandMesh->BoneSpaceTransforms[BoneMapping.BoneId].SetLocation(FVector::ZeroVector);
	}
}

void UCameraHandInput::UpdateSkeleton()
{
	if (!HandMesh)
	{
		return;
	}

	auto const bHasCustomGestureThisFrame = bHasCustomGesture;
	if (bHasCustomGesture)
	{
		// custom gestures have to be applied every frame, so reset the flag here
		bHasCustomGesture = false;
		bHadCustomGestureLastFrame = true;
	}
	else if (bHadCustomGestureLastFrame)
	{
		// reset the grip bone
		auto&& Pose = HandMesh->SkeletalMesh->RefSkeleton.GetRefBonePose();
		if (ensureMsgf(Pose.IsValidIndex(GripBoneId), TEXT("GripBoneId is %i"), GripBoneId))
		{
			HandMesh->BoneSpaceTransforms[GripBoneId] = Pose[GripBoneId];
		}
		bHadCustomGestureLastFrame = false;
	}

	if (bHasCustomGestureThisFrame && DigitsMaskedFromCustomGesture == 0)
	{
		// get the bone rotations anyway, since we need them for gesture detection (eg. dropping)
		for (auto Index = 0; Index != static_cast<int>(EOculusXRBone::Bone_Max); Index += 1)
		{
			auto const Bone = static_cast<EOculusXRBone>(Index);
			auto const Rotation = UOculusXRInputFunctionLibrary::GetBoneRotation(Hand, Bone);
			RawLocalSpaceRotations[Bone] = Rotation;
		}

		return;
	}

	// Update finger rotations
	for (auto Index = 0; Index != static_cast<int>(EOculusXRBone::Bone_Max); Index += 1)
	{
		auto const Bone = static_cast<EOculusXRBone>(Index);
		auto& LastRotation = BoneRotations[Bone];
		auto Rotation = UOculusXRInputFunctionLibrary::GetBoneRotation(Hand, Bone);
		RawLocalSpaceRotations[Bone] = Rotation;
		if (bBoneRotationFilteringEnabled)
		{
			FilterBoneRotation(Bone, LastRotation, Rotation);
		}
		LastRotation = Rotation;
	}

	for (auto BoneMapping : BoneMap)
	{
		if (BoneMapping.BoneId >= 0)
		{
			// skip updating if we're in a custom gesture and the bone's digit isn't masked out
			if (bHasCustomGestureThisFrame)
			{
				if (BoneMapping.Digit == EHandDigit::None)
				{
					continue;
				}
				if ((static_cast<uint8>(BoneMapping.Digit) & DigitsMaskedFromCustomGesture) != static_cast<uint8>(BoneMapping.Digit))
				{
					continue;
				}
			}

			auto const BoneRotation = BoneRotations[BoneMapping.MappedBone];
			SetBoneRotation(HandMesh, BoneMapping, BoneRotation);
		}
	}

	HandMesh->MarkRefreshTransformDirty();

	if (bDynamicScalingEnabled)
	{
		auto const Scale = UOculusXRInputFunctionLibrary::GetHandScale(Hand);
		HandMesh->SetRelativeScale3D(FVector(Scale));
	}
}

void UCameraHandInput::UpdateGrabInput()
{
	// Hand pose grab input
	auto const GrabAxisValue = GetGrippingAxis();
	if (bIsInGrabPose)
	{
		if (GrabAxisValue < ReleaseThresholdForGrip)
		{
			bIsInGrabPose = false;
		}
	}
	else if (GrabAxisValue > GrabThresholdForGrip)
	{
		bIsInGrabPose = true;
	}

	// pinch axis input
	auto const HighestPinchValueThisFrame = GetHighestPinchValue();

	if (bIsPinching)
	{
		if (HighestPinchValueLastFrame > ReleaseThreshold && HighestPinchValueThisFrame <= ReleaseThreshold)
		{
			bIsPinching = false;
		}
	}
	else
	{
		if (HighestPinchValueLastFrame < GrabThreshold && HighestPinchValueThisFrame >= GrabThreshold)
		{
			bIsPinching = true;
		}
	}

	HighestPinchValueLastFrame = HighestPinchValueThisFrame;
}

void UCameraHandInput::UpdatePointingInput()
{
	bIsPointing = GetPointingAxis() > 0.5f;
}

void UCameraHandInput::IndexFingerPinchUpdate(float Value) { FingerPinchUpdate(0, Value); }
void UCameraHandInput::MiddleFingerPinchUpdate(float Value) { FingerPinchUpdate(1, Value); }
void UCameraHandInput::RingFingerPinchUpdate(float Value) { FingerPinchUpdate(2, Value); }
void UCameraHandInput::PinkyFingerPinchUpdate(float Value) { FingerPinchUpdate(3, Value); }

void UCameraHandInput::FingerPinchUpdate(int FingerIndex, float Value)
{
	// don't update if the hand, finger, or thumb have low confidence tracking
	if (!IsTracked())
	{
		return;
	}

	if (GetFingerTrackingConfidence(Hand, EOculusXRFinger::Thumb) ==
		EOculusXRTrackingConfidence::Low)
	{
		return;
	}

	auto const OculusFinger = static_cast<EOculusXRFinger>(FingerIndex + 1);
	if (GetFingerTrackingConfidence(Hand, OculusFinger) == EOculusXRTrackingConfidence::Low)
	{
		return;
	}

	FingerPinchValues[FingerIndex] = Value;
}

float UCameraHandInput::GetHighestPinchValue()
{
	// only look at the index and middle fingers
	return FMath::Max(FingerPinchValues[0], FingerPinchValues[1]);
}

FTransform UCameraHandInput::GetGripBoneTransform(EBoneSpaces::Type BoneSpace)
{
	auto const TransformSpace = BoneSpace == EBoneSpaces::ComponentSpace ?
		RTS_Component :
		RTS_World;

	if (HandMesh)
	{
		return HandMesh->GetSocketTransform(GripBoneName, TransformSpace);
	}
	return FTransform::Identity;
}

EOculusXRHandType UCameraHandInput::GetHand() const
{
	return Hand;
}

UPoseableMeshComponent* UCameraHandInput::GetMesh() const
{
	return HandMesh;
}

FName UCameraHandInput::GetGripBoneName() const
{
	return GripBoneName;
}

void UCameraHandInput::ForceMeshHidden(bool Hidden)
{
	if (Hidden != bForceMeshHidden)
	{
		bForceMeshHidden = Hidden;
		UpdateMeshVisibility();
	}
}

void UCameraHandInput::UpdateMeshVisibility() const
{
	auto const bVisible = bIsActive && !bForceMeshHidden;
	if (HandMesh)
	{
		HandMesh->SetHiddenInGame(!bVisible);
	}
}

// these are all a mess and we really need a generic (data-driven) system to handle this
float UCameraHandInput::GetPointingAxis()
{
	auto const SummedJoints =
		RawLocalSpaceRotations[EOculusXRBone::Index_1].GetNormalized().GetAngle() +
		RawLocalSpaceRotations[EOculusXRBone::Index_2].GetNormalized().GetAngle() +
		RawLocalSpaceRotations[EOculusXRBone::Index_3].GetNormalized().GetAngle();

	return FMath::GetMappedRangeValueClamped(PointingAxisJointRotationRange, FVector2D(0.f, 1.f), SummedJoints);
}

float UCameraHandInput::GetGrippingAxis()
{
	auto const SummedJoints =
		RawLocalSpaceRotations[EOculusXRBone::Middle_1].GetNormalized().GetAngle() +
		RawLocalSpaceRotations[EOculusXRBone::Middle_2].GetNormalized().GetAngle() +
		RawLocalSpaceRotations[EOculusXRBone::Middle_3].GetNormalized().GetAngle() +
		RawLocalSpaceRotations[EOculusXRBone::Ring_1].GetNormalized().GetAngle() +
		RawLocalSpaceRotations[EOculusXRBone::Ring_2].GetNormalized().GetAngle() +
		RawLocalSpaceRotations[EOculusXRBone::Ring_3].GetNormalized().GetAngle() +
		RawLocalSpaceRotations[EOculusXRBone::Pinky_1].GetNormalized().GetAngle() +
		RawLocalSpaceRotations[EOculusXRBone::Pinky_2].GetNormalized().GetAngle() +
		RawLocalSpaceRotations[EOculusXRBone::Pinky_3].GetNormalized().GetAngle();

	return FMath::GetMappedRangeValueClamped(GrippingAxisJointRotationRange, FVector2D(0.f, 1.f), SummedJoints);
}

float UCameraHandInput::GetThumbUpAxis()
{
	auto const SummedJoints =
		RawLocalSpaceRotations[EOculusXRBone::Thumb_0].GetNormalized().GetAngle() +
		RawLocalSpaceRotations[EOculusXRBone::Thumb_1].GetNormalized().GetAngle() +
		RawLocalSpaceRotations[EOculusXRBone::Thumb_2].GetNormalized().GetAngle() +
		RawLocalSpaceRotations[EOculusXRBone::Thumb_3].GetNormalized().GetAngle();

	return FMath::GetMappedRangeValueClamped(ThumbUpAxisJointRotationRange, FVector2D(0.f, 1.f), SummedJoints);
}

bool UCameraHandInput::ApplyPoseToMesh(
	FString PoseString, UPoseableMeshComponent* HandMesh,
	TArray<FHandBoneMapping> const& BoneMappings)
{
	auto BoneToRecognizedBone = [](EOculusXRBone Bone)
	{
		switch (Bone)
		{
		case EOculusXRBone::Wrist_Root: return Wrist;
		case EOculusXRBone::Thumb_0: return Thumb_0;
		case EOculusXRBone::Thumb_1: return Thumb_1;
		case EOculusXRBone::Thumb_2: return Thumb_2;
		case EOculusXRBone::Thumb_3: return Thumb_3;
		case EOculusXRBone::Index_1: return Index_1;
		case EOculusXRBone::Index_2: return Index_2;
		case EOculusXRBone::Index_3: return Index_3;
		case EOculusXRBone::Middle_1: return Middle_1;
		case EOculusXRBone::Middle_2: return Middle_2;
		case EOculusXRBone::Middle_3: return Middle_3;
		case EOculusXRBone::Ring_1: return Ring_1;
		case EOculusXRBone::Ring_2: return Ring_2;
		case EOculusXRBone::Ring_3: return Ring_3;
		case EOculusXRBone::Pinky_0: return Pinky_0;
		case EOculusXRBone::Pinky_1: return Pinky_1;
		case EOculusXRBone::Pinky_2: return Pinky_2;
		case EOculusXRBone::Pinky_3: return Pinky_3;
		default: return static_cast<ERecognizedBone>(-1);
		}
	};

	FHandPose Pose;
	Pose.CustomEncodedPose = PoseString;
	if (HandMesh == nullptr || Pose.Decode() == false)
	{
		return false;
	}

	for (auto Mapping : BoneMappings)
	{
		auto const Bone = BoneToRecognizedBone(Mapping.MappedBone);
		if (Bone != static_cast<ERecognizedBone>(-1))
		{
			auto Rotator = Bone == Wrist ? FRotator::ZeroRotator : Pose.GetRotator(Bone);
			SetBoneRotation(HandMesh, Mapping, Rotator.Quaternion());
		}
	}

	HandMesh->RefreshBoneTransforms();
	return true;
}

bool UCameraHandInput::SetPose(FString PoseString)
{
	if (ApplyPoseToMesh(PoseString, HandMesh, BoneMap))
	{
		bHasCustomGesture = true;
		return true;
	}

	return false;
}

#undef ConvertBoneToFinger
#undef GetFingerTrackingConfidence

#ifdef EOculusXRFinger
#undef EOculusXRFinger
#endif
