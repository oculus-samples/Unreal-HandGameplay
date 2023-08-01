// Copyright (c) Meta Platforms, Inc. and affiliates.

#include "HandTrackingFilterComponent.h"

#include "MotionControllerComponent.h"
#include "OculusXRInputFunctionLibrary.h"
#include "Camera/CameraComponent.h"
#include "QuatUtil.h"
#include "XRMotionControllerBase.h"

DECLARE_LOG_CATEGORY_EXTERN(LogHandTrackingFilter, Log, All);

DEFINE_LOG_CATEGORY(LogHandTrackingFilter);

// use real time because this can get hit multiple times per frame
#define NOW FPlatformTime::Seconds()

UHandTrackingFilterComponent::UHandTrackingFilterComponent()
{
	bAutoActivate = true;
}

void UHandTrackingFilterComponent::BeginPlay()
{
	Super::BeginPlay();

	LastFrameData = FHandTrackingFilterData{NOW, GetComponentTransform()};

	if (auto Controller = Cast<UMotionControllerComponent>(GetAttachParent()))
	{
		UOculusXRInputFunctionLibrary::HandMovementFilter.AddWeakLambda(this,
			[this, ThisHand = Controller->GetTrackingSource()]
		(
			EControllerHand Hand,
			FVector* Location,
			FRotator* Orientation,
			bool* Success
		)
			{
				if (Hand == ThisHand && UOculusXRInputFunctionLibrary::IsHandTrackingEnabled())
				{
					if (IsInGameThread() && PreFilterComponent)
					{
						PreFilterComponent->SetActive(*Success);
						if (*Success)
						{
							PreFilterComponent->SetRelativeRotation(*Orientation);
							PreFilterComponent->SetRelativeLocation(*Location);
						}
					}

					DoFiltering(*Location, *Orientation, !*Success);
					*Success = true;
				}
			});
	}
}

void UHandTrackingFilterComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UOculusXRInputFunctionLibrary::HandMovementFilter.RemoveAll(this);
}

EHandTrackingDataQuality UHandTrackingFilterComponent::GetDataQualityOverride() const
{
	if (bIgnoreConfidence)
		return EHandTrackingDataQuality::None;

	if (auto const Controller = Cast<UMotionControllerComponent>(GetAttachParent()))
	{
		auto Hand = EControllerHand::Left;
		FXRMotionControllerBase::GetHandEnumForSourceName(Controller->MotionSource, Hand);
		auto const DeviceHand = Hand == EControllerHand::Left ? EOculusXRHandType::HandLeft : EOculusXRHandType::HandRight;
		return Controller->IsTracked() ?
			UOculusXRInputFunctionLibrary::GetTrackingConfidence(DeviceHand) == EOculusXRTrackingConfidence::High ?
			EHandTrackingDataQuality::Good :
			EHandTrackingDataQuality::None :
			EHandTrackingDataQuality::Bad;
	}

	return EHandTrackingDataQuality::None;
}

FVector UHandTrackingFilterComponent::SmoothPosition(FVector StartPos, FVector TargetPos)
{
	if (SmoothPositionFactor > 0.99f)
	{
		// Updating disabled
		return StartPos;
	}

	auto const Diff = TargetPos - StartPos;
	auto const Dist = Diff.Size();

	if (Dist < MinSmoothPositionDistance)
	{
		// Not enough of a change to update
		UE_LOG(LogHandTrackingFilter, Verbose, TEXT("%s - SmoothPos - Not enough of a change to update"), *GetName());
		LastGoodVelocity = FVector::ZeroVector;
		return StartPos;
	}

	if (Dist >= MaxSmoothPositionDistance)
	{
		// Clamp max distance from target
		UE_LOG(LogHandTrackingFilter, Verbose, TEXT("%s - SmoothPos - Clamp max distance from target"), *GetName());
		auto const Dir = Diff / Dist;
		auto const MoveDist = Dist - MaxSmoothPositionDistance;
		return StartPos + Dir * MoveDist;
	}

	UE_LOG(LogHandTrackingFilter, Verbose, TEXT("%s - SmoothPos - Smooth"), *GetName());
	auto const SmoothFactor = 1.0f - SmoothPositionFactor;
	return FMath::Lerp(StartPos, TargetPos, SmoothFactor);
}

void UHandTrackingFilterComponent::SetPreFilterComponent(USceneComponent* Component)
{
	PreFilterComponent = Component;
}

FQuat UHandTrackingFilterComponent::SmoothRotation(FQuat StartRot, FQuat TargetRot)
{
	if (SmoothRotationFactorMin > 0.99f)
	{
		// Updating disabled
		return StartRot;
	}

	auto const CosAngle = StartRot | TargetRot;
	if (CosAngle > SmoothRotationDotMax)
	{
		// Not enough of a change to update
		UE_LOG(LogHandTrackingFilter, Verbose, TEXT("%s - SmoothRotation - Not enough of a change to update"), *GetName());
		return StartRot;
	}

	float SmoothFactor;
	if (CosAngle <= SmoothRotationDotMin)
	{
		UE_LOG(LogHandTrackingFilter, Verbose, TEXT("%s - SmoothRotation - CosAngle %f <= SmoothRotationDotMin %f"), *GetName(), CosAngle, SmoothRotationDotMin);
		SmoothFactor = 1.0f - SmoothRotationFactorMin;
	}
	else
	{
		UE_LOG(LogHandTrackingFilter, Verbose, TEXT("%s - SmoothRotation - CosAngle %f > SmoothRotationDotMin %f"), *GetName(), CosAngle, SmoothRotationDotMin);
		auto const Weight = (CosAngle - SmoothRotationDotMin) / (SmoothRotationDotMax - SmoothRotationDotMin);
		SmoothFactor = 1.0f - FMath::Lerp(SmoothRotationFactorMin,
			SmoothRotationFactorMax, Weight);
	}

	UE_LOG(LogHandTrackingFilter, Verbose, TEXT("%s - SmoothRotation - %f"), *GetName(), SmoothFactor);
	return FQuat::Slerp(StartRot, TargetRot, SmoothFactor);
}

bool UHandTrackingFilterComponent::DoFirstPassFilter(
	FHandTrackingFilterData const& LastData,
	FHandTrackingFilterData const& ThisFrameInitData,
	FTransform& NewTransform)
{
	auto const LastLocation = LastData.Transform.GetLocation();
	auto const NewLocation = ThisFrameInitData.Transform.GetLocation();
	auto const DeltaLocation = NewLocation - LastLocation;
	auto const DistanceSquared = DeltaLocation.SizeSquared();

	// if there hasn't been a tracking update, extrapolate
	if (DistanceSquared < MinTrackingDistance * MinTrackingDistance)
	{
		UE_LOG(LogHandTrackingFilter, Verbose, TEXT("%s - DoFirstPassFilter - if the location hasn't changed (%f), there hasn't been a tracking update"), *GetName(), DistanceSquared);
		NewTransform = LastSetTransform;
		return true;
	}

	auto const NewRotation = ThisFrameInitData.Transform.GetRotation();

	auto const SmoothedPosition = SmoothPosition(LastSetTransform.GetLocation(), NewLocation);
	NewTransform.SetLocation(SmoothedPosition);

	auto const SmoothedRotation = SmoothRotation(LastSetTransform.GetRotation(), NewRotation);
	NewTransform.SetRotation(SmoothedRotation);

	return false;
}

FTransform UHandTrackingFilterComponent::DoFilteringImpl(FTransform HandTransform, bool bForceBadData)
{
	auto CalculatedData = FHandTrackingFilterCalculatedData();

	auto const LastData = LastFrameData;
	auto ThisFrameInitData = FHandTrackingFilterData{NOW, HandTransform};

	auto const DeltaTime = ThisFrameInitData.Time - LastData.Time;

	auto MitigatedTransform = ThisFrameInitData.Transform;
	auto const EarlyOut = DoFirstPassFilter(LastData, ThisFrameInitData, MitigatedTransform);
	if (!bForceBadData && EarlyOut)
	{
		LastSetTransform = MitigatedTransform;
		return MitigatedTransform;
	}

	UE_LOG(LogHandTrackingFilter, Verbose, TEXT("%s - DoFilteringImpl - dt %lf"), *GetName(), DeltaTime);

	auto& Data = LastFrameData;
	Data = ThisFrameInitData;

	auto const DeltaLocation = MitigatedTransform.GetLocation() - LastSetTransform.GetLocation();
	auto const Distance = DeltaLocation.Size();
	CalculatedData.Distance = Distance;

	Data.Velocity = DeltaLocation / DeltaTime;
	CalculatedData.Velocity = Data.Velocity;

	auto const DeltaVelocity = Data.Velocity - LastData.Velocity;
	auto const Acceleration = CalculatedData.Acceleration = DeltaVelocity / DeltaTime;
	CalculatedData.AccelerationScalar = Acceleration.Size();

	auto DeltaRotation = Data.Transform.GetRotation() * LastSetTransform.GetRotation().Inverse();
	Data.AngularVelocity = Scale(DeltaRotation, 1 / DeltaTime);
	CalculatedData.AngularVelocityScalar = Data.AngularVelocity.GetAngle();

	auto BadData = CalculatedData.AccelerationScalar > MaxAcceleration ||
		Distance > MaxDistancePerFrame ||
		Data.Velocity.SizeSquared() > MaxSpeed * MaxSpeed ||
		CalculatedData.AngularVelocityScalar > MaxAngularVelocity;

	auto const QualityOverride = GetDataQualityOverride();
	if (QualityOverride == EHandTrackingDataQuality::Good)
		BadData = false;
	else if (QualityOverride == EHandTrackingDataQuality::Bad)
		BadData = true;

	auto const Camera = GetOwner()->FindComponentByClass<UCameraComponent>();
	if (Camera != nullptr)
	{
		CalculatedData.CameraDistance = FVector::Dist(Data.Transform.GetLocation(), Camera->GetComponentLocation());
		if ((QualityOverride != EHandTrackingDataQuality::Good || bCameraRadiusIgnoreConfidence) && CalculatedData.CameraDistance < IgnoreCameraLocationRadius)
			BadData = true;
	}
	else
	{
		CalculatedData.CameraDistance = -1;
	}

	if (bForceBadData)
	{
		UE_LOG(LogHandTrackingFilter, Verbose, TEXT("%s - DoFilteringImpl - Bad Data Forced"), *GetName());
		BadData = true;
	}

	auto const OriginalDistance = Distance;
	auto const LastSetTransformActual = LastSetTransform;
	auto NewTransform = IntegrateFilterData(MitigatedTransform, Data, DeltaTime, BadData);
	if (FVector::Dist(LastSetTransformActual.GetLocation(), NewTransform.GetLocation()) > OriginalDistance)
	{
		UE_LOG(LogHandTrackingFilter, Verbose, TEXT("%s - DoFilteringImpl - over 5 oh no - %s %f"),
			*GetName(),
			BadData ? TEXT("bad data") : TEXT("good data"),
			OriginalDistance);
	}

	CalculatedData.QualityOverride = QualityOverride;
	CalculatedData.BadData = BadData;

	OnCalculatedData.Broadcast(CalculatedData);

	if (UE_LOG_ACTIVE(LogHandTrackingFilter, VeryVerbose))
	{
		auto CalculatedDataText = FString(TEXT(""));
		FHandTrackingFilterCalculatedData::StaticStruct()->ExportText(CalculatedDataText, &CalculatedData, nullptr,
			this,
			PPF_ExportsNotFullyQualified | PPF_Copy | PPF_Delimited | PPF_IncludeTransient | PPF_ExternalEditor, nullptr);
		UE_LOG(LogHandTrackingFilter, VeryVerbose, TEXT("%s - OnCalculatedData - (%i,%i,%i) %s"),
			*GetName(),
			(int)NewTransform.GetLocation().X,
			(int)NewTransform.GetLocation().Y,
			(int)NewTransform.GetLocation().Z,
			*CalculatedDataText);
	}

	return NewTransform;
}

void UHandTrackingFilterComponent::DoFiltering(FVector& Location, FRotator& Orientation, bool bForceBadData)
{
	if (IsActive() == false)
		return;

	if (bForceZeroTransform)
	{
		Location = FVector::ZeroVector;
		Orientation = FRotator::ZeroRotator;
		return;
	}

	auto ParentTransform = GetAttachParent()->GetAttachParent()->GetComponentTransform();
	auto RelativeTransform = FTransform(Orientation, Location);
	auto WorldTransform = DoFilteringImpl(RelativeTransform * ParentTransform, bForceBadData);
	auto NewRelativeTransform = WorldTransform * ParentTransform.Inverse();
	Location = NewRelativeTransform.GetLocation();
	Orientation = NewRelativeTransform.Rotator();
}

void UHandTrackingFilterComponent::ExtrapolateTransform(float DeltaTime, FVector& FakeLocation, FQuat& FakeRotation)
{
	UE_LOG(LogHandTrackingFilter, Verbose, TEXT("%s - ExtrapolateTransform - LastGoodVelocity = %f"), *GetName(), LastGoodVelocity.Size());
	FakeLocation = LastSetTransform.GetLocation() + LastGoodVelocity * DeltaTime;
	FakeRotation = Scale(LastGoodAngularVelocity, DeltaTime) * LastSetTransform.GetRotation();
}

FTransform UHandTrackingFilterComponent::IntegrateFilterData(
	FTransform MitigatedTransform,
	FHandTrackingFilterData const& Data, float DeltaTime,
	bool BadData)
{
	if (BadData)
	{
		LastBadDataTime = Data.Time;
		LastGoodVelocity *= VelocityDamping; // damp the velocity so it doesn't fly off
		LastGoodAngularVelocity = Scale(LastGoodAngularVelocity, VelocityDamping);
		UE_LOG(LogHandTrackingFilter, Verbose, TEXT("%s - IntegrateFilterData - Bad Data"), *GetName());
	}
	else
	{
		LastGoodVelocity = FMath::Lerp(LastGoodVelocity, Data.Velocity.GetClampedToMaxSize(MaxFakeVelocity), GoodVelocityBlendRate);
		LastGoodAngularVelocity = Scale(Data.AngularVelocity,
			FMath::Max(Data.AngularVelocity.GetAngle() / MaxAngularVelocity, 1.0f));
		UE_LOG(LogHandTrackingFilter, Verbose, TEXT("%s - IntegrateFilterData - Good Data"), *GetName());
	}

	FVector FakeLocation;
	FQuat FakeRotation;
	ExtrapolateTransform(DeltaTime, FakeLocation, FakeRotation);

	if (BadData)
	{
		// when the data is bad it's possible for the MitigatedTransform to be invalid (containing NaNs)
		// so we set LastSetTransform directly instead of using the lerps
		LastSetTransform.SetLocation(FakeLocation);
		LastSetTransform.SetRotation(FakeRotation);
	}
	else
	{
		auto const FadePercent = FMath::Clamp((Data.Time - LastBadDataTime) / BadTransformFadeTime, 0.0, 1.0);
		LastSetTransform.SetLocation(FMath::LerpStable(FakeLocation, MitigatedTransform.GetLocation(), FadePercent));
		LastSetTransform.SetRotation(FQuat::Slerp(FakeRotation, MitigatedTransform.GetRotation(), FadePercent));
	}

	return LastSetTransform;
}

#undef NOW
