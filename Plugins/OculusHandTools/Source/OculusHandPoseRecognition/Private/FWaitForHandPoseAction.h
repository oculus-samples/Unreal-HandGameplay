// Copyright (c) Meta Platforms, Inc. and affiliates.

#pragma once

#include "CoreMinimal.h"
#include "UObject/WeakObjectPtr.h"
#include "Engine/LatentActionManager.h"
#include "LatentActions.h"
#include "HandPoseRecognizer.h"

/** Latent action for blueprint node that waits for a pose to be recognized. */
class FWaitForHandPoseAction : public FPendingLatentAction
{
public:
	FName ExecutionFunction;
	int32 OutputLink;
	FWeakObjectPtr CallbackTarget;

	TWeakObjectPtr<UHandPoseRecognizer> HandPoseRecognizer;
	float PoseMinDuration;
	float TimeToWait;
	int* PoseIndex;
	FString* PoseName;

	EWaitForHandPoseExitType* OutExecs;

	FWaitForHandPoseAction(const FLatentActionInfo& LatentInfo, UHandPoseRecognizer* HandPoseRecognizer, float PoseMinDuration, float TimeToWait, int* PoseIndex, FString* PoseName, EWaitForHandPoseExitType* OutExecs)
		: ExecutionFunction(LatentInfo.ExecutionFunction)
		, OutputLink(LatentInfo.Linkage)
		, CallbackTarget(LatentInfo.CallbackTarget)
		, HandPoseRecognizer(HandPoseRecognizer)
		, PoseMinDuration(PoseMinDuration)
		, TimeToWait(TimeToWait)
		, PoseIndex(PoseIndex)
		, PoseName(PoseName)
		, OutExecs(OutExecs)
	{
	}

	virtual void UpdateOperation(FLatentResponse& Response) override
	{
		// Time out?
		if (TimeToWait > 0.0f)
		{
			TimeToWait -= Response.ElapsedTime();
			if (TimeToWait <= 0.0f)
			{
				// We have timed out while waiting.
				*PoseIndex = -1;
				PoseName->Empty();
				*OutExecs = EWaitForHandPoseExitType::TimeOut;
				Response.FinishAndTriggerIf(true, ExecutionFunction, OutputLink, CallbackTarget);
			}
		}

		// Did we recognize a pose?
		int Index;
		FString Name;
		float Duration;
		float Error;
		float Confidence;

		if (HandPoseRecognizer->GetRecognizedHandPose(Index, Name, Duration, Error, Confidence) && Duration >= PoseMinDuration)
		{
			*PoseIndex = Index;
			*PoseName = Name;
			*OutExecs = EWaitForHandPoseExitType::PoseSeen;
			Response.FinishAndTriggerIf(true, ExecutionFunction, OutputLink, CallbackTarget);
		}
	}

#if WITH_EDITOR
	// Returns a human readable description of the latent operation's current state
	virtual FString GetDescription() const override
	{
		FString Msg = TEXT("Waiting for Hand Pose");
		return Msg;
	}
#endif
};



