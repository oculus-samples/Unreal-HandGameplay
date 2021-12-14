// Copyright (c) Meta Platforms, Inc. and affiliates.

#include "HandRecognitionFunctionLibrary.h"
#include "Engine.h"
#include "FWaitForHandPoseAction.h"
#include "FWaitForHandGestureAction.h"
#include "FRecordHandPoseAction.h"

void UHandRecognitionFunctionLibrary::WaitForHandPose(
	UObject* WorldContextObject,
	UHandPoseRecognizer* HandPoseRecognizer,
	float PoseMinDuration, float TimeToWait,
	EWaitForHandPoseExitType& OutExecs,
	int& PoseIndex, FString& PoseName,
	FLatentActionInfo LatentInfo)
{
	if (auto World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		auto& LatentActionManager = World->GetLatentActionManager();
		if (LatentActionManager.FindExistingAction<FWaitForHandPoseAction>(LatentInfo.CallbackTarget, LatentInfo.UUID) == nullptr)
		{
			LatentActionManager.AddNewAction(LatentInfo.CallbackTarget, LatentInfo.UUID,
				new FWaitForHandPoseAction(LatentInfo, HandPoseRecognizer, PoseMinDuration, TimeToWait, &PoseIndex, &PoseName, &OutExecs));
		}
	}
}

void UHandRecognitionFunctionLibrary::WaitForHandGesture(
	UObject* WorldContextObject,
	UHandGestureRecognizer* HandGestureRecognizer,
	float TimeToWait, EGestureConsumptionBehavior Behavior,
	EWaitForHandGestureExitType& OutExecs,
	int& GestureIndex, FString& GestureName, FVector& GestureDirection, float& GestureOuterDuration, float& GestureInnerDuration,
	FLatentActionInfo LatentInfo)
{
	if (auto World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		auto& LatentActionManager = World->GetLatentActionManager();
		if (LatentActionManager.FindExistingAction<FWaitForHandGestureAction>(LatentInfo.CallbackTarget, LatentInfo.UUID) == nullptr)
		{
			LatentActionManager.AddNewAction(LatentInfo.CallbackTarget, LatentInfo.UUID,
				new FWaitForHandGestureAction(LatentInfo, HandGestureRecognizer, TimeToWait, Behavior, &GestureIndex, &GestureName, &GestureDirection, &GestureOuterDuration, &GestureInnerDuration, &OutExecs));
		}
	}
}

void UHandRecognitionFunctionLibrary::RecordHandPose(UObject* WorldContextObject, UHandPoseRecognizer* Recognizer, const ERecordHandPoseEntryType& InExecs, ERecordHandPoseExitType& OutExecs, FLatentActionInfo LatentInfo)
{
	if (auto World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		auto& LatentActionManager = World->GetLatentActionManager();
		if (LatentActionManager.FindExistingAction<FRecordHandPoseAction>(LatentInfo.CallbackTarget, LatentInfo.UUID) == nullptr)
		{
			LatentActionManager.AddNewAction(LatentInfo.CallbackTarget, LatentInfo.UUID, new FRecordHandPoseAction(LatentInfo, Recognizer, &InExecs, &OutExecs));
		}
	}
}
