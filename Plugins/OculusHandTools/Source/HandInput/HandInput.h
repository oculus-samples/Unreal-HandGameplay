// Copyright (c) Meta Platforms, Inc. and affiliates.

#pragma once

#include "HandInput.generated.h"

UENUM(BlueprintType)
enum class EHandTrackingMode : uint8
{
	Controller,
	Camera,
	Unknown
};

UINTERFACE()
class HANDINPUT_API UHandInput : public UInterface
{
	GENERATED_BODY()
};

class HANDINPUT_API IHandInput
{
	GENERATED_BODY()
	
public:
	virtual void SetHand(EControllerHand Hand) = 0;
	virtual bool IsActive() = 0;
	virtual FTransform GetGripBoneTransform(EBoneSpaces::Type BoneSpace) = 0;
	virtual bool IsPointing() = 0;
	virtual EHandTrackingMode GetTrackingMode() = 0;
};

DECLARE_LOG_CATEGORY_EXTERN(LogHandInput, Log, All);
