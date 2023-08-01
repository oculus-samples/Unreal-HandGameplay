// Copyright (c) Meta Platforms, Inc. and affiliates.

#include "PoseableHandComponent.h"
#include "OculusHandPoseRecognitionModule.h"

void UPoseableHandComponent::BeginPlay()
{
	if (SkeletalMesh)
	{
		bCustomPoseableHandMesh = true;
	}

	Super::BeginPlay();
}

void UPoseableHandComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (CustomPoseWeightCurrent != CustomPoseWeightTarget)
	{
		CustomPoseWeightCurrent = FMath::FInterpConstantTo(CustomPoseWeightCurrent, CustomPoseWeightTarget, DeltaTime, CustomPoseWeightLerpSpeed);
	}

	if (CustomPoseWeightCurrent > 0.f)
	{
		UpdateBonePose(EOculusXRBone::Thumb_0, Thumb_0);
		UpdateBonePose(EOculusXRBone::Thumb_1, Thumb_1);
		UpdateBonePose(EOculusXRBone::Thumb_2, Thumb_2);
		UpdateBonePose(EOculusXRBone::Thumb_3, Thumb_3);

		UpdateBonePose(EOculusXRBone::Index_1, Index_1);
		UpdateBonePose(EOculusXRBone::Index_2, Index_2);
		UpdateBonePose(EOculusXRBone::Index_3, Index_3);

		UpdateBonePose(EOculusXRBone::Middle_1, Middle_1);
		UpdateBonePose(EOculusXRBone::Middle_2, Middle_2);
		UpdateBonePose(EOculusXRBone::Middle_3, Middle_3);

		UpdateBonePose(EOculusXRBone::Ring_1, Ring_1);
		UpdateBonePose(EOculusXRBone::Ring_2, Ring_2);
		UpdateBonePose(EOculusXRBone::Ring_3, Ring_3);

		UpdateBonePose(EOculusXRBone::Pinky_0, Pinky_0);
		UpdateBonePose(EOculusXRBone::Pinky_1, Pinky_1);
		UpdateBonePose(EOculusXRBone::Pinky_2, Pinky_2);
		UpdateBonePose(EOculusXRBone::Pinky_3, Pinky_3);
	}
}

void UPoseableHandComponent::UpdateBonePose(EOculusXRBone Bone, ERecognizedBone PosedBone)
{
	auto BoneIndex = (int)Bone;

	if (bCustomPoseableHandMesh && BoneNameMappings.Contains(Bone))
	{
		auto const MappedBoneIndex = SkeletalMesh->GetRefSkeleton().FindBoneIndex(BoneNameMappings[Bone]);
		if (MappedBoneIndex != INDEX_NONE)
		{
			BoneIndex = MappedBoneIndex;
		}
	}

	if (BoneSpaceTransforms.Num() > BoneIndex)
	{
		auto const TrackedRotation = BoneSpaceTransforms[BoneIndex].GetRotation();
		auto const PosedRotation = CustomPose.GetRotator(PosedBone).Quaternion();
		auto const PosedWeight = CustomPose.GetWeight(PosedBone) * CustomPoseWeightCurrent;

		if (PosedWeight > 0)
		{
			auto const NewRotation = FQuat::Slerp(TrackedRotation, PosedRotation, PosedWeight);
			BoneSpaceTransforms[BoneIndex].SetRotation(NewRotation);
		}
	}
}


void UPoseableHandComponent::SetPose(FString PoseString, float LerpSpeedOverride)
{
	CustomPose.CustomEncodedPose = PoseString;


	if (CustomPose.Decode())
	{
		CustomPoseWeightTarget = 1.f;
		CustomPoseWeightCurrent = 0.f;
		CustomPoseWeightLerpSpeed = LerpSpeedOverride > 0 ? LerpSpeedOverride : DefaultLerpSpeed;
	}
	else
	{
		UE_LOG(LogHandPoseRecognition, Warning, TEXT("Pose string '%s' can not be decoded."), *PoseString);
	}
}

void UPoseableHandComponent::ClearPose(float LerpSpeedOverride)
{
	CustomPoseWeightTarget = 0.f;
	CustomPoseWeightLerpSpeed = LerpSpeedOverride > 0 ? LerpSpeedOverride : DefaultLerpSpeed;
}
