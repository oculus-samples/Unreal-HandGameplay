// Copyright (c) Meta Platforms, Inc. and affiliates.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HandPose.h"
#include "OculusXRHandComponent.h"
#include "OculusXRInputFunctionLibrary.h"
#include "HandPoseRecognizer.generated.h"

/**
 * Actor component that recognizes hand poses.
 *
 * @warning Must be attached to a component that moves with hands.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class OCULUSHANDPOSERECOGNITION_API UHandPoseRecognizer : public USceneComponent
{
	GENERATED_BODY()

public:
	/** Sets default values for this component's properties. */
	UHandPoseRecognizer(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	/** Called when the game starts. */
	virtual void BeginPlay() override;

public:
	/** Called every frame. */
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** Hand side recognized (set to None to disable the component). */
	UPROPERTY(Category = "Hand Pose Recognition", EditAnywhere, BlueprintReadWrite)
	EOculusXRHandType Side;

	/** Minimum time interval between hand recognitions (throttling). */
	UPROPERTY(Category = "Hand Pose Recognition", EditAnywhere, BlueprintReadWrite)
	float RecognitionInterval;

	/** Minimum confidence level needed to recognize a pose.  Can be overridden for each individual pose. */
	UPROPERTY(Category = "Hand Pose Recognition", EditAnywhere, BlueprintReadWrite)
	float DefaultConfidenceFloor;

	/** Fraction of the current confidence level that we keep per unit of time. */
	UPROPERTY(Category = "Hand Pose Recognition", EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float DampingFactor;

	/** Recognized hand pose patterns. */
	UPROPERTY(Category = "Hand Pose Recognition", EditAnywhere, BlueprintReadWrite)
	TArray<FHandPose> Poses;

	/**
	 * Call to get the currently recognized hand pose.
	 * @param Index - Index of the recognized pose.
	 * @param Name - Name of the recognized pose.
	 * @param Duration - How long this pose has been held.
	 * @param Error - Raw recognition error.
	 * @param Confidence - Confidence level from 0.0 to 1.0.
	 * @return A boolean that indicates if a pose is currently recognized.
	 */
	UFUNCTION(BlueprintCallable)
	UPARAM(DisplayName = "Pose Recognized")
	bool GetRecognizedHandPose(int& Index, FString& Name, float& Duration, float& Error, float& Confidence);

	/** Access to the last hand pose information. */
	const FHandPose& GetCurrentPose() const
	{
		return Pose;
	}

	/**
	 * Call to log the current hand pose.
	 * This is used to create reference poses that can then be tweaked.
	 */
	UFUNCTION(BlueprintCallable)
	void LogEncodedHandPose();

protected:
	/** Structure storing the current bone rotators. */
	FHandPose Pose;

	/**
	 * Returns the wrist rotation with yaw adjusted relative to the player's gaze.
	 * @param ComponentQuat - Quaternion representing the controller rotation.
	 * @return The adjusted rotator.
	 */
	FRotator GetWristRotator(FQuat ComponentQuat) const;

private:
	/** Recognition state. */
	float TimeSinceLastRecognition;
	int CurrentHandPose;
	float CurrentHandPoseDuration;
	float CurrentHandPoseConfidence;
	float CurrentHandPoseError;

	/** Index incremented every time LogEncodedHandPose() is called, to help identify reference poses in the logs. */
	int LoggedIndex;
};
