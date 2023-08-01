// Copyright (c) Meta Platforms, Inc. and affiliates.

#include "HandGestureRecognizer.h"
#include "OculusHandPoseRecognitionModule.h"
#include "Kismet/GameplayStatics.h"

UHandGestureRecognizer::UHandGestureRecognizer(const FObjectInitializer& ObjectInitializer /* = FObjectInitializer::Get() */):
	Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = true;
	RecognitionInterval = 0.0f;
	RecognitionSkippedFrames = 1;
	bHasRecognizedGesture = false;
	TimeSinceLastRecognition = 0.0f;
	SkippedFramesSinceLastRecognition = 0;
}

void UHandGestureRecognizer::BeginPlay()
{
	Super::BeginPlay();

	// We must be attached to a HandPoseRecognizer component
	TArray<USceneComponent*> Parents;
	GetParentComponents(Parents);
	FString ImmediateParentClassName;
	if (Parents.Num() >= 1)
	{
		ImmediateParentClassName = Parents[0]->GetClass()->GetName();
		HandPoseRecognizer = Cast<UHandPoseRecognizer>(Parents[0]);
	}

	auto TurnOffGestureRecognition = false;
	if (!HandPoseRecognizer)
	{
		UE_LOG(LogHandPoseRecognition, Error, TEXT("UHandGestureRecognizer called %s MUST be attached to a UHandPoseRecognizer not a %s."),
			*GetName(), *ImmediateParentClassName);
		TurnOffGestureRecognition = true;
	}

	if (HandPoseRecognizer->Side == EOculusXRHandType::None)
	{
		UE_LOG(LogHandPoseRecognition, Warning, TEXT("UHandGestureRecognizer called %s is attached to a disabled UHandPoseRecognizer."),
			*GetName());
		TurnOffGestureRecognition = true;
	}

	if (TurnOffGestureRecognition)
	{
		SetComponentTickEnabled(false);
		return;
	}

	// We decode the hand gestures
	for (auto GestureIndex = 0; GestureIndex < Gestures.Num(); ++GestureIndex)
	{
		if (!Gestures[GestureIndex].ProcessEncodedGestureString(HandPoseRecognizer))
		{
			UE_LOG(LogHandPoseRecognition, Error, TEXT("UHandGestureRecognizer gesture at index %d is invalid."), GestureIndex);
		}
	}
}

void UHandGestureRecognizer::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!HandPoseRecognizer) return;

	// Recognition is throttled
	TimeSinceLastRecognition += DeltaTime;
	if (TimeSinceLastRecognition < RecognitionInterval)
	{
		SkippedFramesSinceLastRecognition++;
		return;
	}

	if (SkippedFramesSinceLastRecognition < RecognitionSkippedFrames)
	{
		SkippedFramesSinceLastRecognition++;

		// For better recognition of gesture strength (speed of transition from first to last pose),
		// it is necessary to skip at least one cycle.
		// UE_LOG(LogOculusHandPoseRecognition, Error, TEXT("*** Skipping Frame (skipped %d)"), SkippedFramesSinceLastRecognition);

		return;
	}

	TimeSinceLastRecognition = 0.0f;
	SkippedFramesSinceLastRecognition = 0;

	// We need the current game time for recognizing the transition time between the first and last poses.
	auto const World = GetWorld();
	check(World != nullptr);
	auto const CurrentTime = UGameplayStatics::GetTimeSeconds(World);

	// Currently recognized hand pose
	int PoseIndex;
	FString PoseName;
	float PoseDuration;
	float PoseError;
	float PoseConfidence;
	HandPoseRecognizer->GetRecognizedHandPose(PoseIndex, PoseName, PoseDuration, PoseError, PoseConfidence);

	// By getting the relative location this way we have:
	// X+ is forward, Y+ is right, Z+ is up
	auto const ActorRelativeLocation = GetComponentTransform().GetRelativeTransform(GetOwner()->GetTransform()).GetLocation();

	// We process all hand gestures
	CompletedGestures.Reset();

	for (auto GestureIndex = 0; GestureIndex < Gestures.Num(); ++GestureIndex)
	{
		if (Gestures[GestureIndex].Step(PoseIndex, PoseDuration, DeltaTime, CurrentTime, ActorRelativeLocation))
		{
			CompletedGestures.Push(GestureIndex);
		}
	}

	// Updating property that indicates that at least one gesture is ready.
	bHasRecognizedGesture = CompletedGestures.Num() > 0;
}

bool UHandGestureRecognizer::GetRecognizedHandGesture(
	EGestureConsumptionBehavior Behavior,
	int& Index, FString& Name,
	FVector& GestureDirection,
	float& GestureOuterDuration, float& GestureInnerDuration)
{
	if (CompletedGestures.Num() == 0)
	{
		// There are no completed gestures at this time
		Index = -1;
		Name = "None";
		GestureDirection = FVector::ZeroVector;
		GestureOuterDuration = 0.0f;
		GestureInnerDuration = 0.0f;
		return false;
	}

	Index = CompletedGestures.Pop(false);
	Name = Gestures[Index].GestureName;
	GestureDirection = Gestures[Index].GetGestureDirection();
	GestureOuterDuration = Gestures[Index].ComputeOuterDuration();
	GestureInnerDuration = Gestures[Index].ComputeInnerDuration();

	// UE_LOG(LogOculusHandPoseRecognition, Warning, TEXT("Recognized gesture %d:%s."), Index, *Name);

	// Reset gesture(s) according to preference
	if (Behavior == EGestureConsumptionBehavior::Reset)
	{
		ResetHandGesture(Index);
	}
	else if (Behavior == EGestureConsumptionBehavior::ResetAll)
	{
		ResetAllHandGestures();
		CompletedGestures.Reset();
	}

	// Updating property that indicates that at least one other gesture is ready.
	bHasRecognizedGesture = CompletedGestures.Num() > 0;

	return true;
}

EGestureState UHandGestureRecognizer::GetGestureRecognitionState(int Index)
{
	if (Index >= 0 && Index < Gestures.Num())
	{
		return Gestures[Index].GetGestureState();
	}

	return EGestureState::GestureNotStarted;
}

void UHandGestureRecognizer::ResetHandGesture(int& Index)
{
	if (Index >= 0 && Index < Gestures.Num())
	{
		Gestures[Index].Reset();
	}
}

void UHandGestureRecognizer::ResetAllHandGestures()
{
	for (auto& Gesture : Gestures)
	{
		Gesture.Reset();
	}
}

void UHandGestureRecognizer::DumpAllGestureStates() const
{
	UE_LOG(LogHandPoseRecognition, Warning, TEXT("Gesture states for %s"), *GetName());

	for (auto GestureIndex = 0; GestureIndex < Gestures.Num(); ++GestureIndex)
	{
		Gestures[GestureIndex].DumpGestureState(GestureIndex, HandPoseRecognizer);
	}
}
