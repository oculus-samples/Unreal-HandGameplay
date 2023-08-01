// Copyright (c) Meta Platforms, Inc. and affiliates.

#include "Interactable.h"
#include "InteractableSelector.h"
#include "TransformString.h"
#include "OculusInteractableModule.h"

AInteractable::AInteractable()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Defaults.
	FarFieldSelectionDelayMs = 100.0f;
}

void AInteractable::BeginPlay()
{
	Super::BeginPlay();
}

void AInteractable::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	while (Selectors.Num() > 0)
	{
		auto Selector = Selectors.Pop(false);
		Selector->SetSelectedInteractable(nullptr, false); // Remove with no notification.
	}

	Super::EndPlay(EndPlayReason);
}

void AInteractable::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AInteractable::BeginSelection_Implementation(AInteractableSelector* Selector)
{
	// Meant to be subclassed, but do not forget to call Super::BeginSelection_Implementation.
	if (!Selectors.Contains(Selector))
	{
		Selectors.Add(Selector);
	}
}

void AInteractable::EndSelection_Implementation(AInteractableSelector* Selector)
{
	// Meant to be subclassed, but do not forget to call Super::EndSelection_Implementation.
	if (Selectors.Contains(Selector))
	{
		Selectors.Remove(Selector);
	}
}

bool AInteractable::IsSelected() const
{
	return Selectors.Num() > 0;
}

const TArray<AInteractableSelector*>& AInteractable::GetSelectors() const
{
	return Selectors;
}

void AInteractable::SetInteractablePhysicsSimulation_Implementation(bool SimulatePhysics)
{
	auto RootPrimitive = Cast<UPrimitiveComponent>(GetRootComponent());
	if (RootPrimitive)
	{
		RootPrimitive->SetSimulatePhysics(SimulatePhysics);
	}
}

bool AInteractable::IsMovable_Implementation()
{
	auto const RootPrimitive = Cast<UPrimitiveComponent>(GetRootComponent());
	if (RootPrimitive)
	{
		return RootPrimitive->Mobility == EComponentMobility::Movable;
	}

	return false;
}

void AInteractable::SelectGrabPose(EHandSide Side, bool& GrabPoseFound, FString& GrabPoseName, FTransform& GrabTransform, FString& GrabHandPose)
{
	auto& GrabPoses = Side == EHandSide::HandLeft ? GrabPosesLeftHand : GrabPosesRightHand;

	GrabPoseFound = GrabPoses.Num() > 0;
	if (GrabPoseFound)
	{
		// Random selection for this first pass.
		auto const RandomGrabIndex = FMath::RandRange(0, GrabPoses.Num() - 1);

		auto const EncodedTransform = GrabPoses[RandomGrabIndex].RelativeHandTransform;

		if (FTransformString::StringToTransform(EncodedTransform, GrabTransform))
		{
			GrabPoseName = GrabPoses[RandomGrabIndex].PoseName;
			GrabHandPose = GrabPoses[RandomGrabIndex].HandPose;
		}
		else
		{
			UE_LOG(LogInteractable, Warning, TEXT("Invalid grab transform on %s, %s side at index %d: \"%s\""),
				*GetHumanReadableName(),
				Side == EHandSide::HandLeft ? TEXT("left") : TEXT("right"),
				RandomGrabIndex,
				*EncodedTransform);
			GrabPoseFound = false;
		}
	}
}
