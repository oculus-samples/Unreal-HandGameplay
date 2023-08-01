// Copyright (c) Meta Platforms, Inc. and affiliates.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HandGesture.h"
#include "HandPoseRecognizer.h"
#include "HandGestureRecognizer.generated.h"

/**
 * What to do with a gesture after it has been returned by GetRecognizedHandGesture().
 */
UENUM(BlueprintType)
enum class EGestureConsumptionBehavior : uint8
{
	Reset,
	ResetAll,
	NoReset
};

/**
 * Bones for gesture strength settings.
 */
UENUM(BlueprintType)
enum class EGestureStrengthBone : uint8
{
	Thumb0 = 0,
	Thumb1,
	Thumb2,
	Thumb3,
	Index1,
	Index2,
	Index3,
	Middle1,
	Middle2,
	Middle3,
	Ring1,
	Ring2,
	Ring3,
	Pinky0,
	Pinky1,
	Pinky2,
	Pinky3,
	Wrist
};

/**
 * Angles for gesture strength settings.
 */
UENUM(BlueprintType)
enum class EGestureStrengthBoneAngle : uint8
{
	Pitch = 0,
	Yaw,
	Roll
};

/**
 * Actor component that recognizes gestures (i.e. poses over time).
 * 
 * @warning Must be attached to a UHandPoseRecognizer.
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class OCULUSHANDPOSERECOGNITION_API UHandGestureRecognizer : public USceneComponent
{
	GENERATED_BODY()

public:
	/** Sets default values for this component's properties. */
	UHandGestureRecognizer(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	/** Called when the game starts. */
	virtual void BeginPlay() override;

	/** Parent hand pose recognizer. */
	UHandPoseRecognizer* HandPoseRecognizer = nullptr;

public:
	/** Called every frame. */
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** Minimum time interval between hand recognitions (throttling). */
	UPROPERTY(Category = "Hand Gesture Recognition", EditAnywhere, BlueprintReadWrite)
	float RecognitionInterval;

	/** Minimum number of frames to skip between hand recognitions (throttling). */
	UPROPERTY(Category = "Hand Gesture Recognition", EditAnywhere, BlueprintReadWrite)
	int RecognitionSkippedFrames;

	/** Collection of hand gestures to recognize. */
	UPROPERTY(Category = "Hand Gesture Recognition", EditAnywhere, BlueprintReadWrite)
	TArray<FHandGesture> Gestures;

	/**
	 * Call to check if there is a recognized gesture pending.
	 * @return A boolean that indicates if there's at least one pending gesture recognized.
	 */
	UPROPERTY(Category = "Hand Gesture Recognition", BlueprintReadOnly)
	bool bHasRecognizedGesture;

	/**
	 * Call to get the currently recognized gesture.
	 * @param Behavior - Auto-reset of the behavior returned.
	 * @param Index - Index of the recognized gesture.
	 * @param Name - Name of the recognized gesture.
	 * @param GestureDirection - Gesture vector, relative to owning actor.
	 * @param GestureOuterDuration - Overall gesture duration.
	 * @param GestureInnerDuration - Inner gesture duration.
	 * @return A boolean that indicates if a gesture is currently recognized.
	 */
	UFUNCTION(BlueprintCallable)
	UPARAM(DisplayName = "Gesture Recognized")
	bool GetRecognizedHandGesture(
		EGestureConsumptionBehavior Behavior,
		int& Index, FString& Name,
		FVector& GestureDirection,
		float& GestureOuterDuration, float& GestureInnerDuration);

	/**
	 * Call to get the state of recognition of a specific gesture.
	 * @param Index - Index of the gesture to query.
	 * @return Current state of gesture recognition.
	 */
	UFUNCTION(BlueprintCallable)
	UPARAM(DisplayName = "Gesture State")
	EGestureState GetGestureRecognitionState(int Index);

	/**
	 * Resets the specified hand gesture by index.
	 * @param Index - Where to store the index of the recognized gesture.
	 */
	UFUNCTION(BlueprintCallable)
	void ResetHandGesture(int& Index);

	/** Resets all hand gestures. */
	UFUNCTION(BlueprintCallable)
	void ResetAllHandGestures();

	/** For debugging purposes: a state dump of all hand gestures. */
	UFUNCTION(BlueprintCallable)
	void DumpAllGestureStates() const;

private:
	/** Recognition state. */
	int SkippedFramesSinceLastRecognition;
	float TimeSinceLastRecognition;

	/** Gestures that were completed in the last tick. */
	TArray<int> CompletedGestures;
};
