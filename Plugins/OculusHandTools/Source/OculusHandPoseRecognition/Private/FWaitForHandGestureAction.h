// Copyright (c) Meta Platforms, Inc. and affiliates.

#pragma once

#include "CoreMinimal.h"
#include "UObject/WeakObjectPtr.h"
#include "Engine/LatentActionManager.h"
#include "LatentActions.h"
#include "HandGestureRecognizer.h"

/** Latent action for blueprint node that waits for a gesture to be recognized. */
class FWaitForHandGestureAction : public FPendingLatentAction
{
public:
	FName ExecutionFunction;
	int32 OutputLink;
	FWeakObjectPtr CallbackTarget;

	TWeakObjectPtr<UHandGestureRecognizer> HandGestureRecognizer;
	float TimeToWait;
	EGestureConsumptionBehavior Behavior;
	int* GestureIndex;
	FString* GestureName;
	FVector* GestureDirection;
	float* GestureOuterDuration;
	float* GestureInnerDuration;

	EWaitForHandGestureExitType* OutExecs;

	FWaitForHandGestureAction(const FLatentActionInfo& LatentInfo, UHandGestureRecognizer* HandGestureRecognizer, float TimeToWait, EGestureConsumptionBehavior Behavior, int* GestureIndex, FString* GestureName, FVector* GestureDirection, float* GestureOuterDuration, float* GestureInnerDuration, EWaitForHandGestureExitType* OutExecs)
		: ExecutionFunction(LatentInfo.ExecutionFunction)
		, OutputLink(LatentInfo.Linkage)
		, CallbackTarget(LatentInfo.CallbackTarget)
		, HandGestureRecognizer(HandGestureRecognizer)
		, TimeToWait(TimeToWait)
		, Behavior(Behavior)
		, GestureIndex(GestureIndex)
		, GestureName(GestureName)
		, GestureDirection(GestureDirection)
		, GestureOuterDuration(GestureOuterDuration)
		, GestureInnerDuration(GestureInnerDuration)
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
				*GestureIndex = -1;
				GestureName->Empty();
				*OutExecs = EWaitForHandGestureExitType::TimeOut;
				Response.FinishAndTriggerIf(true, ExecutionFunction, OutputLink, CallbackTarget);
			}
		}

		// Did we recognize a gesture?
		int Index;
		FString Name;
		FVector Direction;
		float OuterDuration, InnerDuration;

		if (HandGestureRecognizer->GetRecognizedHandGesture(Behavior, Index, Name, Direction, OuterDuration, InnerDuration))
		{
			*GestureIndex = Index;
			*GestureName = Name;
			*GestureDirection = Direction;
			*GestureOuterDuration = OuterDuration;
			*GestureInnerDuration = InnerDuration;
			*OutExecs = EWaitForHandGestureExitType::GestureSeen;
			Response.FinishAndTriggerIf(true, ExecutionFunction, OutputLink, CallbackTarget);
		}
	}

#if WITH_EDITOR
	// Returns a human readable description of the latent operation's current state
	virtual FString GetDescription() const override
	{
		FString Msg = TEXT("Waiting for Hand Gesture");
		return Msg;
	}
#endif
};
