// Copyright (c) Meta Platforms, Inc. and affiliates.

#include "InteractableFunctionLibrary.h"
#include "OculusInteractableModule.h"
#include "TransformString.h"

ECollisionChannel UInteractableFunctionLibrary::InteractableTraceChannel = ECC_GameTraceChannel1;

void UInteractableFunctionLibrary::LogNearbyInteractables(AActor* ReferenceActor, float RadiusAroundActor)
{
	auto World = ReferenceActor ? ReferenceActor->GetWorld() : nullptr;
	if (!World)
		return;

	auto CollisionSphere = FCollisionShape::MakeSphere(RadiusAroundActor);
	TArray<FHitResult> Hits;
	auto ReferenceTransform = ReferenceActor->GetTransform();
	auto Loc = ReferenceTransform.GetLocation();

	World->SweepMultiByChannel(Hits, Loc, Loc, FQuat::Identity, InteractableTraceChannel, CollisionSphere);
	if (Hits.Num() > 0)
	{
		UE_LOG(LogInteractable, Error, TEXT("Interactables near %s radius %0.2f"),
			*ReferenceActor->GetHumanReadableName(), RadiusAroundActor);

		for (const auto& Hit : Hits)
		{
			auto ActorTransform = Hit.GetActor()->GetTransform();
			FTransform RelativeActorTransform;
			RelativeActorTransform.SetLocation(ReferenceTransform.InverseTransformPosition(ActorTransform.GetLocation()));
			RelativeActorTransform.SetRotation(ReferenceTransform.InverseTransformRotation(ActorTransform.GetRotation()));
			RelativeActorTransform.SetScale3D(ActorTransform.GetScale3D());

			FString RelativeActorTransformString;
			FTransformString::TransformToString(RelativeActorTransform, RelativeActorTransformString);

			UE_LOG(LogInteractable, Warning, TEXT("Interactable %s %s"),
				*Hit.GetActor()->GetHumanReadableName(),
				*RelativeActorTransformString);
		}
	}
}
