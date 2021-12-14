// Copyright (c) Meta Platforms, Inc. and affiliates.

#include "HandRecognitionGameMode.h"
#include "OculusHandPoseRecognitionModule.h"
#include "OculusInputFunctionLibrary.h"
#include "OculusHandComponent.h"
#include "HandPose.h"

int AHandRecognitionGameMode::LoggedIndex = 0;

void AHandRecognitionGameMode::LogHandPose(FString Side)
{
	auto const World = GetWorld();
	if (!World)
	{
		UE_LOG(LogHandPoseRecognition, Error, TEXT("LogHandPose failed to find a valid world."));
		return;
	}

	auto const Controller = World->GetFirstPlayerController();
	if (!Controller)
	{
		UE_LOG(LogHandPoseRecognition, Error, TEXT("LogHandPose failed to find a player controller."));
		return;
	}

	auto const Pawn = Controller->GetPawn();
	if (!Pawn)
	{
		UE_LOG(LogHandPoseRecognition, Error, TEXT("LogHandPose failed to find a pawn."));
		return;
	}

	// We use the first matching OculusHandComponent.
	auto HandType = EOculusHandType::None;
	if (Side.Equals(TEXT("left"), ESearchCase::IgnoreCase))
		HandType = EOculusHandType::HandLeft;
	else if (Side.Equals(TEXT("right"), ESearchCase::IgnoreCase))
		HandType = EOculusHandType::HandRight;

	if (HandType == EOculusHandType::None)
	{
		UE_LOG(LogHandPoseRecognition, Error, TEXT("LogHandPose requires \"left\" or \"right\" parameter, but received \"%s\""), *Side);
		return;
	}

	TArray<UOculusHandComponent*> OculusHandComponents;
	Pawn->GetComponents(OculusHandComponents);
	for (auto HandComponent : OculusHandComponents)
	{
		if (HandComponent->SkeletonType == HandType)
		{
			FHandPose Pose;
			Pose.UpdatePose(HandType, HandComponent->GetComponentRotation());
			Pose.Encode();
			UE_LOG(LogHandPoseRecognition, Warning, TEXT("HAND POSE %d: %s"), LoggedIndex++, *Pose.CustomEncodedPose);
			return;
		}
	}

	UE_LOG(LogHandPoseRecognition, Error, TEXT("LogHandPose did not find a valid OculusHandComponent on the %s side"), *Side);
}
