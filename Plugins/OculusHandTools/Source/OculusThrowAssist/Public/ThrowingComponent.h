// Copyright (c) Meta Platforms, Inc. and affiliates.

#pragma once

#include "CoreMinimal.h"

#include "Components/ActorComponent.h"
#include "ThrowingComponent.generated.h"

class UTransformBufferComponent;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class OCULUSTHROWASSIST_API UThrowingComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UThrowingComponent();

	/**
	 * @brief Called to initialize the throw calculator.
	 * @param AttachParent Usually the hand to be tracked for throwing objects.
	 */
	UFUNCTION(BlueprintCallable)
	void Initialize(USceneComponent* AttachParent);

	/**
	 * @param LookDirection The world-space direction the player is looking in (to assist with aiming).
	 * @return The world-space velocity of the calculated throw.
	 */
	UFUNCTION(BlueprintPure)
	FVector GetThrowVector(FVector LookDirection) const;

	/**
	 * @brief Tick the throw calculator with new info from its parent transform.
	 * @param IsTracked Whether or not the current data is considered high quality.
	 */
	UFUNCTION(BlueprintCallable)
	void Update(bool IsTracked);

	/// How much time to look back in the past when making a throw to account for input and render latency
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Throwing")
	float ThrowLatencyAdjustmentTimeSeconds = 0.04f;

	/// minimum amount of time with uninterrupted tracking needed in order to make a high-confidence throw
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Throwing: High Confidence")
	float HighConfidenceThrowMinTrackingTime = 0.08f;

	/// minimum throw vector speed for a low-confidence throw
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Throwing: Low Confidence")
	float LowConfidenceThrowMinSpeed = 50.f;

	/// factor that determines how much to weigh the head forward vector with low confidence throwing
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Throwing: Low Confidence")
	float LowConfidenceHeadForwardFactor = 0.4f;

	/// speed factor for very low confidence throws
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Throwing: Very Low Confidence")
	float VeryLowConfidenceSpeedFactor = 5.f;

	/// factor that determines how much to weigh the very low confidence vector
	/// (the point that tracking was lost to the current point)
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Throwing: Very Low Confidence")
	float VeryLowConfidenceVectorFactor = 0.3f;

	/// factor that determines how much to weigh the head forward vector with very low confidence throwing
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Throwing: Very Low Confidence")
	float VeryLowConfidenceHeadForwardFactor = 0.7f;

	/// High-confidence transform buffer for the hands
	UPROPERTY(Transient)
	UTransformBufferComponent* HighConfidenceTransformBuffer;

	/// Transform buffer for the hands
	UPROPERTY(Transient)
	UTransformBufferComponent* AllTransformsTransformBuffer;

	/// Whether to choose the "best" throw vector or just a simple look back.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Throwing: Throw Vector Selection")
	bool bSelectBestThrowVectorFromPast = true;

	/// How far back to track throw vector samples.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Throwing: Throw Vector Selection")
	float OldestPossibleThrowVectorSeconds = 1.0f;

	/// How many throw vector samples to track.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Throwing: Throw Vector Selection")
	int NumThrowVectorSamples = 10;

	/// Score weight for recency.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Throwing: Throw Vector Selection")
	float ThrowVectorSelectionRecencyScoring = 1.0f;

	/// Score weight for direction relative to look direction.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Throwing: Throw Vector Selection")
	float ThrowVectorSelectionDirectionScoring = 1.0f;

	/// Score weight for speed.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Throwing: Throw Vector Selection")
	float ThrowVectorSelectionSpeedScoring = 1.0f;

private:
	FVector GetThrowVectorInPast(float SecondsAgo, FVector LookDirection) const;
	float GetScoreForThrowVector(FVector ThrowVector, FVector LookDirection, float TimeInPastNormalized) const;
	float GetTimeWithGoodTracking() const;

	bool WasTrackedLastFrame = false;

	float MostRecentTrackingLossTime;
	FTransform MostRecentTrackingLossTransform;
	FVector MostRecentTrackingLossVelocity;

	float MostRecentTrackingGainTime;
	FTransform MostRecentTrackingGainTransform;
	FVector MostRecentTrackingGainVelocity;
};
