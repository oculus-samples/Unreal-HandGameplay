// Copyright (c) Meta Platforms, Inc. and affiliates.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "HandTrackingFilterComponent.generated.h"

struct HANDTRACKINGFILTER_API FHandTrackingFilterData
{
	double Time;
	FTransform Transform;
	FVector Velocity;
	FQuat AngularVelocity;
};

UENUM(BlueprintType)
enum class EHandTrackingDataQuality : uint8
{
	None,
	Good,
	Bad
};

USTRUCT(BlueprintType)
struct HANDTRACKINGFILTER_API FHandTrackingFilterCalculatedData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FVector Acceleration;

	UPROPERTY(BlueprintReadOnly)
	float AccelerationScalar;

	UPROPERTY(BlueprintReadOnly)
	float AngularVelocityScalar;

	UPROPERTY(BlueprintReadOnly)
	float CameraDistance;

	UPROPERTY(BlueprintReadOnly)
	float Distance;

	UPROPERTY(BlueprintReadOnly)
	FVector Velocity;

	/// hand confidence information
	UPROPERTY(BlueprintReadOnly)
	EHandTrackingDataQuality QualityOverride;

	/// if the data should be ignored
	UPROPERTY(BlueprintReadOnly)
	bool BadData;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class HANDTRACKINGFILTER_API UHandTrackingFilterComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	UHandTrackingFilterComponent();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(EEndPlayReason::Type const EndPlayReason) override;

	FHandTrackingFilterData LastFrameData;

	double LastBadDataTime = -99999;
	FVector LastGoodVelocity = FVector::ZeroVector;
	FQuat LastGoodAngularVelocity = FQuat::Identity;
	FTransform LastSetTransform = FTransform::Identity;

	bool DoFirstPassFilter(FHandTrackingFilterData const& LastData, FHandTrackingFilterData const& ThisFrameInitData, FTransform& NewTransform);
	FTransform DoFilteringImpl(FTransform HandTransform, bool bForceBadData);
	FTransform IntegrateFilterData(FTransform MitigatedTransform, FHandTrackingFilterData const& Data, float DeltaTime, bool BadData);
	void DoFiltering(FVector& Location, FRotator& Orientation, bool bForceBadData);
	void ExtrapolateTransform(float DeltaTime, FVector& FakeLocation, FQuat& FakeRotation);
	EHandTrackingDataQuality GetDataQualityOverride() const;
	FQuat SmoothRotation(FQuat StartRot, FQuat TargetRot);
	FVector SmoothPosition(FVector StartPos, FVector TargetPos);

	double LastFrozenMovementTime = -99999;
	double LastFrozenRotationTime = -99999;

	UPROPERTY(Transient)
	USceneComponent* PreFilterComponent = nullptr;

public:
	/** Percentage to de-jitter the position by */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Jitter Mitigation")
	float SmoothPositionFactor = 0.875f;

	/** Max distance that should be considered "no movement" (cm) */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Jitter Mitigation")
	float MinSmoothPositionDistance = 0.2f;

	/** Max distance that should be considered "jittery movement" (cm) */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Jitter Mitigation")
	float MaxSmoothPositionDistance = 1.0f;

	/** Minimum percentage to de-jitter the position by */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Jitter Mitigation")
	float SmoothRotationFactorMin = 0.0f;

	/** Maximum percentage to de-jitter the position by */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Jitter Mitigation")
	float SmoothRotationFactorMax = 0.75f;

	/** Max angle that should be considered "no rotation" (cosine units) */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Jitter Mitigation")
	float SmoothRotationDotMin = FMath::Cos(FMath::DegreesToRadians(3.0f));

	/** Max angle that should be considered "jittery rotation" (cosine units) */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Jitter Mitigation")
	float SmoothRotationDotMax = FMath::Cos(FMath::DegreesToRadians(0.5f));

	/** Acceleration limit to trigger the filter (cm/s^2)*/
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Filter Triggers")
	float MaxAcceleration = 100000.0f;

	/** Speed limit to trigger the filter (cm/s) */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Filter Triggers")
	float MaxSpeed = 2000.0f;

	/** Distance per frame limit to trigger the filter (cm) */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Filter Triggers")
	float MaxDistancePerFrame = 9999999.0f;

	/** Angular velocity limit to trigger the filter (rad/s) */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Filter Triggers")
	float MaxAngularVelocity = 3.0f;

	/** Time it will take to interpolate from filtered data to good data (s) */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Filter Effects")
	float BadTransformFadeTime = 0.1f;

	/** Radius from the HMD to trigger the filter (cm) */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Filter Triggers")
	float IgnoreCameraLocationRadius = 10.0f;

	/** Radius from the HMD to trigger the filter (cm) */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Filter Triggers")
	bool bCameraRadiusIgnoreConfidence = true;

	/** Speed to clamp the extrapolated velocity (cm/s) */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Filter Effects")
	float MaxFakeVelocity = 0.8f;

	/** Per-frame multiplier on the extrapolated velocity (%/frame) */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (UIMin = 0, UIMax = 1), Category = "Filter Effects")
	float VelocityDamping = 0.96f;

	/** DEBUG - Disables usage of confidence to determine filtering */
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool bIgnoreConfidence = false;

	/** DEBUG - Enable for the filter to always return a zero transform */
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool bForceZeroTransform = false;

	// Minimum distance the hand must move to be recognized as a new "tick" of tracking
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float MinTrackingDistance = 0.001f;

	// How quickly to integrate presumed good velocity data into the extrapolation velocity
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float GoodVelocityBlendRate = 0.5f;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCalculatedDataEvent, FHandTrackingFilterCalculatedData const &, Data);

	/**
	 * @brief Called every tick with the data calculated by the filter. Used for debugging and tuning.
	 */
	UPROPERTY(BlueprintAssignable)
	FOnCalculatedDataEvent OnCalculatedData;

	/**
	 * @return The pre-filter component
	 */
	UFUNCTION(BlueprintCallable)
	USceneComponent* GetPreFilterComponent() const { return PreFilterComponent; }

	/**
	 * @param Component A scene component that should be updated with the pre-filtered transform data.
	 * Used for accessing pre-filtered hand tracking data.
	 */
	UFUNCTION(BlueprintCallable)
	void SetPreFilterComponent(USceneComponent* Component);
};
