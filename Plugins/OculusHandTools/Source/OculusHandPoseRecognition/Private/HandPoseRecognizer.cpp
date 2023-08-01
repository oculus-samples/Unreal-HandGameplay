// Copyright (c) Meta Platforms, Inc. and affiliates.

#include "HandPoseRecognizer.h"
#include "OculusHandPoseRecognitionModule.h"
#include <limits>

UHandPoseRecognizer::UHandPoseRecognizer(const FObjectInitializer& ObjectInitializer):
	Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = true;

	// Recognition default parameters
	Side = EOculusXRHandType::None;
	RecognitionInterval = 0.0f;
	DefaultConfidenceFloor = 0.5;
	DampingFactor = 0.0f;

	// Current hand pose being recognized
	TimeSinceLastRecognition = 0.0f;
	CurrentHandPose = -1;
	CurrentHandPoseDuration = 0.0f;
	CurrentHandPoseConfidence = 0.0f;
	CurrentHandPoseError = std::numeric_limits<float>::max();

	// Encoded hand pose logged index
	LoggedIndex = 0;
}

void UHandPoseRecognizer::BeginPlay()
{
	Super::BeginPlay();

	// We decode the hand poses
	for (auto PatternIndex = 0; PatternIndex < Poses.Num(); ++PatternIndex)
	{
		if (!Poses[PatternIndex].Decode())
		{
			UE_LOG(LogHandPoseRecognition, Error, TEXT("UHandPoseRecognizer(%s) encoded pose at index %d is invalid."),
				*GetName(),
				PatternIndex);
		}
	}
}

FRotator UHandPoseRecognizer::GetWristRotator(FQuat ComponentQuat) const
{
	auto ComponentRotator = ComponentQuat.Rotator();

	auto const World = GetWorld();
	if (!World) return ComponentRotator;

	auto const PlayerController = World->GetFirstPlayerController();
	if (!PlayerController) return ComponentRotator;

	auto const ComponentRotationToCamera = PlayerController->PlayerCameraManager->GetTransform().InverseTransformRotation(ComponentQuat).Rotator();
	ComponentRotator.Yaw = ComponentRotationToCamera.Yaw;

	return ComponentRotator;
}

void UHandPoseRecognizer::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (Side == EOculusXRHandType::None)
	{
		// Recognizer is disabled
		return;
	}

	// Recognition is throttled
	TimeSinceLastRecognition += DeltaTime;
	if (TimeSinceLastRecognition < RecognitionInterval)
	{
		return;
	}

	// Ignore low confidence cases
	if (UOculusXRInputFunctionLibrary::GetTrackingConfidence(Side) == EOculusXRTrackingConfidence::Low)
	{
		return;
	}

	// Updating tracked hand.
	// Note that the wrist rotation pitch and roll are world relative, and the yaw is hmd relative.
	Pose.UpdatePose(Side, GetWristRotator(GetComponentQuat()));

	// Finding closest pattern
	auto ClosestHandPose = -1;
	auto ClosestHandPoseConfidence = DefaultConfidenceFloor;
	auto ClosestHandPoseError = std::numeric_limits<float>::max();
	auto HighestConfidence = 0.0f;

	for (auto PatternIndex = 0; PatternIndex < Poses.Num(); ++PatternIndex)
	{
		// Skip patterns that are not for this side
		if (Poses[PatternIndex].GetHandType() != Side)
			continue;

		// Computing confidence (we ignore the wrist yaw by default)
		auto RawError = 0.0f;
		auto const Confidence = Poses[PatternIndex].ComputeConfidence(Pose, &RawError);
		// UE_LOG(LogHandPoseRecognition, Error, TEXT("%s confidence %0.0f raw error %0.0f"), *Poses[PatternIndex].PoseName, Confidence, RawError);

		// We always record the smallest error, in case no pattern matches
		if (HighestConfidence < Confidence)
			HighestConfidence = Confidence;

		// We update the best pattern match (first: best match so far has lower confidence than the current pose)
		if (ClosestHandPoseConfidence < Confidence)
		{
			// Second: checking for custom pose error ceiling
			if (Poses[PatternIndex].CustomConfidenceFloor > 0.0f && Confidence < Poses[PatternIndex].CustomConfidenceFloor)
				continue;

			ClosestHandPoseConfidence = Confidence;
			ClosestHandPoseError = RawError;

			ClosestHandPose = PatternIndex;
		}
	}

	// If we have no match, we report the highest confidence seen.
	if (ClosestHandPose == -1)
	{
		ClosestHandPoseConfidence = HighestConfidence;
	}

	if (CurrentHandPose == ClosestHandPose)
	{
		// Same pose as before is being held
		CurrentHandPoseDuration += TimeSinceLastRecognition;
		TimeSinceLastRecognition = 0.0;
		CurrentHandPoseConfidence = DampingFactor * CurrentHandPoseConfidence + (1.0f - DampingFactor) * ClosestHandPoseConfidence;
		CurrentHandPoseError = DampingFactor * CurrentHandPoseError + (1.0f - DampingFactor) * ClosestHandPoseError;
	}
	else
	{
		// Change of pose
		CurrentHandPose = ClosestHandPose;
		CurrentHandPoseDuration = 0.0f;
		CurrentHandPoseConfidence = ClosestHandPoseConfidence;
		CurrentHandPoseError = ClosestHandPoseError;
	}
}

bool UHandPoseRecognizer::GetRecognizedHandPose(int& Index, FString& Name, float& Duration, float& Error, float& Confidence)
{
	Index = CurrentHandPose;
	Duration = CurrentHandPoseDuration;
	Error = CurrentHandPoseError;
	Confidence = CurrentHandPoseConfidence;

	if (Index >= 0 && Index < Poses.Num())
	{
		Name = Poses[Index].PoseName;
		return true;
	}
	Name = TEXT("None");
	return false;
}

void UHandPoseRecognizer::LogEncodedHandPose()
{
	Pose.Encode();
	UE_LOG(LogHandPoseRecognition, Warning, TEXT("HAND POSE %d: %s"), LoggedIndex++, *Pose.CustomEncodedPose);
}
