// Copyright (c) Meta Platforms, Inc. and affiliates.

#pragma once

#include "HandInput.h"

#include "OculusXRInputFunctionLibrary.h"
#include "EnumMap.h"

#include "CameraHandInput.generated.h"

class UTransformBufferComponent;

namespace OculusInput
{
	enum class EOculusXRHandButton;
}

class UPoseableMeshComponent;
class UThrowingComponent;
class UHandComponentBase;

UENUM(BlueprintType, meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = "true"))
enum class EHandDigit : uint8
{
	None = 0,
	Thumb = 1 << 0,
	Index = 1 << 1,
	Middle = 1 << 2,
	Ring = 1 << 3,
	Pinky = 1 << 4,

	Count = 6
};

USTRUCT(BlueprintType)
struct HANDINPUT_API FHandBoneMapping
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	FName BoneName;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Transient)
	int BoneId;

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	EOculusXRBone MappedBone;

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	FQuat RotationOffset = FQuat::Identity;

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	EHandDigit Digit;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Transient)
	FTransform ReferenceTransform;
};

UCLASS(meta = (BlueprintSpawnableComponent))
class HANDINPUT_API UCameraHandInput : public UActorComponent, public IHandInput
{
	GENERATED_BODY()

public:
	UCameraHandInput(FObjectInitializer const& ObjectInitializer);

	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable)
	void SetPoseableMeshComponent(UPoseableMeshComponent* PoseableMeshComponent);
	void SetupInput(UInputComponent* Input);
	void ForceMeshHidden(bool Hidden);

	UFUNCTION(BlueprintCallable)
	static bool ApplyPoseToMesh(
		FString PoseString, UPoseableMeshComponent* HandMesh,
		TArray<FHandBoneMapping> const& BoneMappings);

	UFUNCTION(BlueprintCallable)
	static void SetUpBoneMap(UPoseableMeshComponent* HandMesh, UPARAM(ref) TArray<FHandBoneMapping>& BoneMap);

	// IHandInput
	UFUNCTION(BlueprintCallable)
	virtual void SetHand(EControllerHand InHand) override;
	virtual bool IsActive() override;
	virtual FTransform GetGripBoneTransform(EBoneSpaces::Type BoneSpace) override;
	virtual bool IsPointing() override { return bIsPointing; }
	virtual EHandTrackingMode GetTrackingMode() override { return EHandTrackingMode::Camera; }
	// ~IHandInput

	UFUNCTION(BlueprintPure)
	EOculusXRHandType GetHand() const;

	UFUNCTION(BlueprintPure)
	UPoseableMeshComponent* GetMesh() const;

	UFUNCTION(BlueprintPure)
	FName GetGripBoneName() const;

	UPROPERTY(EditAnywhere, Category = "Hand Input")
	FName GripBoneName;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Hand Input")
	bool bDynamicScalingEnabled = true;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Hand Input")
	float GrabThreshold = 0.8f;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Hand Input")
	float ReleaseThreshold = 0.3f;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Hand Input")
	float GrabThresholdForGrip = 0.6f;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Hand Input")
	float ReleaseThresholdForGrip = 0.3f;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Hand Input")
	float ThrowGestureReleaseFactor = 1.0f;

	// How long to delay before releasing the grip when a previously gripped hand loses tracking and is
	// regained in a non-gripped pose
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Hand Input")
	float GripReleaseDelayOnTrackingResumed = .5f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Hand Input")
	bool bDropDelayEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hand Input")
	TArray<FHandBoneMapping> BoneMap;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Hand Input")
	bool bAlwaysUpdateGrab = false;

	UPROPERTY(BlueprintReadWrite, Category = "Hand Input", Transient)
	UPoseableMeshComponent* HandMesh = nullptr;

	// grip release delay with good tracking
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Hand Input")
	float GlobalDropDelay = 0.05f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Hand Input")
	bool bGlobalDropDelayEnabled = true;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Hand Input")
	float GlobalDropDelayReductionFactorWhenThrowing = 2.f;

	/// minimum distance for a bone to not be frozen
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Bone Jitter Mitigation")
	float MinBoneAngularDistance = 0.01f;

	/// maximum distance for a bone to be smoothed
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Bone Jitter Mitigation")
	float MaxBoneSmoothingAngularDistance = 0.25f;

	/// amount of time to smooth back from frozen to unfrozen
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Bone Jitter Mitigation")
	float BoneUnfreezeTime = 0.1f;

	/// max speed for a bone to extrapolate
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Bone Data Filter")
	float MaxBoneAngularSpeed = 0.5f;

	/// when enabled, bone speed will be clamped even if data is high quality
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Bone Data Filter")
	bool bAlwaysClampBoneSpeed = false;

	/// per-frame damping of extrapolated bone speed
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Bone Data Filter")
	float BoneVelocityDamping = 0.95f;

	/// when enabled, bone filtering will be active
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Bone Data Filter")
	bool bBoneRotationFilteringEnabled = true;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Hand Input")
	FVector2D PointingAxisJointRotationRange = FVector2D(14.f, 18.f);

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Hand Input")
	FVector2D GrippingAxisJointRotationRange = FVector2D(55.f, 44.f);

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Hand Input")
	FVector2D ThumbUpAxisJointRotationRange = FVector2D(21.8f, 22.7f);

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInitializeMesh, UCameraHandInput *, CameraHandInput);

	UPROPERTY(BlueprintAssignable, Category = "Hand Input")
	FOnInitializeMesh OnInitializeMesh;

	UFUNCTION(BlueprintPure)
	bool IsPinching() const { return bIsPinching; }

	UFUNCTION(BlueprintPure)
	bool IsInGrabPose() const { return bIsInGrabPose; }

	UFUNCTION(BlueprintPure)
	FTransform GetBoneTransformWorld(EOculusXRBone Bone) { return BoneCache[Bone]; }

	UFUNCTION(BlueprintPure)
	bool IsTracked() const;

	UFUNCTION(BlueprintPure)
	float GetPointingAxis();

	UFUNCTION(BlueprintPure)
	float GetGrippingAxis();

	UFUNCTION(BlueprintPure)
	float GetThumbUpAxis();

	UFUNCTION(BlueprintCallable)
	bool SetPose(FString PoseString);

protected:
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	EOculusXRHandType Hand = EOculusXRHandType::None;
	bool bIsActive = false;

	void UpdateTracking();
	bool WasTrackedLastFrame = false;
	float TimeWhenTrackingLastGained = -1;

	void FilterBoneRotation(EOculusXRBone Bone, FQuat LastRotation, FQuat& Rotation);
	static void SetBoneRotation(UPoseableMeshComponent* HandMesh, FHandBoneMapping BoneMapping, FQuat BoneRotation);
	void UpdateSkeleton();

	bool bInputIsInitialized = false;
	void UpdateGrabInput();
	float GetHighestPinchValue();
	void IndexFingerPinchUpdate(float Value);
	void MiddleFingerPinchUpdate(float Value);
	void RingFingerPinchUpdate(float Value);
	void PinkyFingerPinchUpdate(float Value);
	void FingerPinchUpdate(int FingerIndex, float Value);
	float FingerPinchValues[4] = {0};
	bool bIsPinching = false;
	float HighestPinchValueLastFrame = 0;
	bool bIsInGrabPose = false;

	void UpdatePointingInput();
	bool bIsPointing = false;

	int GripBoneId = -1;

	bool bHasCustomGesture = false;
	bool bHadCustomGestureLastFrame = false;
	uint8 DigitsMaskedFromCustomGesture = 0;

	bool bForceMeshHidden = false;
	void UpdateMeshVisibility() const;

	// cache bone transforms from hand tracking for use by gameplay code
	TEnumMap<EOculusXRBone, FTransform> BoneCache;
	TEnumMap<EOculusXRBone, FQuat> RawLocalSpaceRotations;

	// cache bone rotations from hand tracking for smoothing
	TEnumMap<EOculusXRBone, FQuat> BoneRotations;
	TEnumMap<EOculusXRBone, FQuat> BoneVelocities;
	TEnumMap<EOculusXRBone, float> BoneLastFrozenTimes;
};
