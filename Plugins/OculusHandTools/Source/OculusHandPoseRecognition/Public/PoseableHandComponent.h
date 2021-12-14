// Copyright (c) Meta Platforms, Inc. and affiliates.

#pragma once

#include "CoreMinimal.h"
#include "OculusHandComponent.h"
#include "HandPose.h"

#include "PoseableHandComponent.generated.h"

UCLASS(Blueprintable, meta = (BlueprintSpawnableComponent), ClassGroup = OculusHand)
class OCULUSHANDPOSERECOGNITION_API UPoseableHandComponent : public UOculusHandComponent
{
	GENERATED_BODY()

public:
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable)
	void SetPose(FString PoseString, float LerpSpeedOverride = -1.f);

	UFUNCTION(BlueprintCallable)
	void ClearPose(float LerpSpeedOverride = -1.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DefaultLerpSpeed = 10.f;

private:
	void UpdateBonePose(EBone Bone, ERecognizedBone PosedBone);

	float CustomPoseWeightCurrent;
	float CustomPoseWeightTarget;
	float CustomPoseWeightLerpSpeed;

	FHandPose CustomPose;
};
