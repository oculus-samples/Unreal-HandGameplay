// Copyright (c) Meta Platforms, Inc. and affiliates.

#include "ThrowingComponent.h"

#include "UObject/UObjectGlobals.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "TransformBufferComponent.h"

static TAutoConsoleVariable<int> CVarDebugDrawThrowingVector(
	TEXT("throw.DebugDrawThrowingVector"),
	0,
	TEXT("Shows throwing vector for camera-tracked hands. 1- draw when throwing. 2- draw always"),
	ECVF_Cheat);

UThrowingComponent::UThrowingComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UThrowingComponent::Initialize(USceneComponent* AttachParent)
{
	auto const UniqueName = [&](auto&& Base) { return MakeUniqueObjectName(GetOwner(), UTransformBufferComponent::StaticClass(), Base); };
	
	HighConfidenceTransformBuffer = NewObject<UTransformBufferComponent>(GetOwner(), UniqueName(TEXT("HandHighConfidenceTransformBuffer")));
	HighConfidenceTransformBuffer->RegisterComponent();
	HighConfidenceTransformBuffer->AttachToComponent(AttachParent, FAttachmentTransformRules::KeepRelativeTransform);
	HighConfidenceTransformBuffer->UpdateMode = ETransformBufferUpdateMode::Manual;
	HighConfidenceTransformBuffer->MaxBufferTimeSeconds = 1.f;
	GetOwner()->AddInstanceComponent(HighConfidenceTransformBuffer);
	
	AllTransformsTransformBuffer = NewObject<UTransformBufferComponent>(GetOwner(), UniqueName(TEXT("HandAllTransformsTransformBuffer")));
	AllTransformsTransformBuffer->RegisterComponent();
	AllTransformsTransformBuffer->AttachToComponent(AttachParent, FAttachmentTransformRules::KeepRelativeTransform);
	AllTransformsTransformBuffer->UpdateMode = ETransformBufferUpdateMode::Manual;
	AllTransformsTransformBuffer->MaxBufferTimeSeconds = 1.f;
	GetOwner()->AddInstanceComponent(AllTransformsTransformBuffer);
}

void UThrowingComponent::Update(bool IsTracked)
{
	if (!HighConfidenceTransformBuffer || !AllTransformsTransformBuffer)
	{
		return;
	}

	if (IsTracked)
	{
		HighConfidenceTransformBuffer->BufferCurrentData();
	}
	AllTransformsTransformBuffer->BufferCurrentData();

	FTransformBufferData TransformBufferDataNow;
	AllTransformsTransformBuffer->GetBufferData(0, TransformBufferDataNow);
	if (IsTracked != WasTrackedLastFrame)
	{
		auto const TimeNow = GetWorld()->GetTimeSeconds();

		if (IsTracked)
		{
			MostRecentTrackingGainTime = TimeNow;
			MostRecentTrackingGainTransform = TransformBufferDataNow.Transform;
			MostRecentTrackingGainVelocity = TransformBufferDataNow.Velocity;
		}
		else
		{
			MostRecentTrackingLossTime = TimeNow;
			MostRecentTrackingLossTransform = TransformBufferDataNow.Transform;
			MostRecentTrackingLossVelocity = TransformBufferDataNow.Velocity;
		}

		WasTrackedLastFrame = IsTracked;
	}
}

FVector UThrowingComponent::GetThrowVector(FVector LookDirection) const
{
	if (bSelectBestThrowVectorFromPast)
	{
		auto BestThrowVector = FVector::ZeroVector;
		auto BestThrowVectorScore = FLT_MIN;
		check(NumThrowVectorSamples > 0);
		auto const TimeStep = OldestPossibleThrowVectorSeconds / NumThrowVectorSamples;
		for (auto i = 0; i < NumThrowVectorSamples; ++i)
		{
			auto const TimeInPast = i * TimeStep;
			auto const ThrowVector = GetThrowVectorInPast(TimeInPast, LookDirection);
			auto const Score = GetScoreForThrowVector(ThrowVector, LookDirection, i / (float)NumThrowVectorSamples);
			if (Score > BestThrowVectorScore)
			{
				BestThrowVector = ThrowVector;
				BestThrowVectorScore = Score;
			}
		}

		return BestThrowVector;
	}
	return GetThrowVectorInPast(ThrowLatencyAdjustmentTimeSeconds, LookDirection);
}

FVector UThrowingComponent::GetThrowVectorInPast(float SecondsAgo, FVector LookDirection) const
{
#if !UE_BUILD_SHIPPING
	auto ArrowColor = FColor::Green;
#endif

	FVector ThrowVector;
	FTransformBufferData TransformBufferData;

	auto const TimeWithGoodTracking = GetTimeWithGoodTracking();

	// High confidence throwing
	if (TimeWithGoodTracking >= HighConfidenceThrowMinTrackingTime)
	{
		HighConfidenceTransformBuffer->GetBufferData(SecondsAgo, TransformBufferData);
		ThrowVector = TransformBufferData.Velocity;
	}
	else
	{
		// check whether we can get a decent vector from the all transforms buffer
		AllTransformsTransformBuffer->GetBufferData(SecondsAgo, TransformBufferData);

		// Low confidence throwing
		if (TransformBufferData.Velocity.Size() > LowConfidenceThrowMinSpeed &&
			FVector::DotProduct(TransformBufferData.Velocity, LookDirection) > 0)
		{
			auto const ThrowSpeed = TransformBufferData.Velocity.Size();
			ThrowVector = TransformBufferData.Velocity.GetSafeNormal() * (1 - LowConfidenceHeadForwardFactor) +
				LookDirection * LowConfidenceHeadForwardFactor;
			ThrowVector *= ThrowSpeed;

#if !UE_BUILD_SHIPPING
			ArrowColor = FColor::Yellow;
#endif
		}
		// Very low confidence throwing
		else
		{
			auto const TrackingLossVector = TransformBufferData.Transform.GetLocation() -
				MostRecentTrackingLossTransform.GetLocation();
			ThrowVector = TrackingLossVector.GetSafeNormal() * VeryLowConfidenceVectorFactor +
				LookDirection * VeryLowConfidenceHeadForwardFactor;
			ThrowVector *= TrackingLossVector.Size() * VeryLowConfidenceSpeedFactor;

#if !UE_BUILD_SHIPPING
			ArrowColor = FColor::Red;
#endif
		}
	}

#if !UE_BUILD_SHIPPING

	if (CVarDebugDrawThrowingVector.GetValueOnAnyThread() > 0)
	{
		DrawDebugDirectionalArrow(
			GetWorld(),
			TransformBufferData.Transform.GetLocation(),
			TransformBufferData.Transform.GetLocation() + ThrowVector * 0.2f,
			1.0f,
			ArrowColor,
			false,
			4.0f,
			0,
			1.f);
	}

#endif

	return ThrowVector;
}

float UThrowingComponent::GetTimeWithGoodTracking() const
{
	if (!WasTrackedLastFrame)
	{
		return 0.f;
	}

	auto const TimeNow = GetWorld()->GetTimeSeconds();
	return TimeNow - MostRecentTrackingGainTime;
}

float UThrowingComponent::GetScoreForThrowVector(FVector ThrowVector, FVector LookDirection, float TimeInPastNormalized) const
{
	// recency
	auto const RecencyScore = (1 - TimeInPastNormalized) * ThrowVectorSelectionRecencyScoring;

	// direction
	auto DirectionScore = FVector::DotProduct(LookDirection, ThrowVector.GetSafeNormal()) / 2.f + 0.5f;
	DirectionScore *= ThrowVectorSelectionDirectionScoring;

	// speed
	auto SpeedScore = FMath::GetMappedRangeValueClamped(FVector2D(0, 200), FVector2D(0, 1), ThrowVector.Size());
	SpeedScore *= ThrowVectorSelectionSpeedScoring;

	return RecencyScore + DirectionScore + SpeedScore;
}

