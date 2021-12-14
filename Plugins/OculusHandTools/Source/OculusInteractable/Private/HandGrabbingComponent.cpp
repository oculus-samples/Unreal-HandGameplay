// Copyright (c) Meta Platforms, Inc. and affiliates.

#include "HandGrabbingComponent.h"

#include "Interactable.h"

AInteractable* UHandGrabbingComponent::TryGrab(FTransform GrabTransform)
{
	auto HitResults = TArray<FOverlapResult>{};
	auto QueryParams = FCollisionQueryParams::DefaultQueryParam;
	auto ResponseParams = FCollisionResponseParams(ECR_Overlap);

	GetWorld()->OverlapMultiByChannel(HitResults, GrabTransform.GetLocation(), GrabTransform.GetRotation(), ECC_WorldDynamic, FCollisionShape::MakeCapsule(GrabCapsuleRadius, GrabCapsuleHeight / 2.f), FCollisionQueryParams::DefaultQueryParam, ResponseParams);

	if (HitResults.Num() > 0)
	{
		auto ClosestInteractable = (AInteractable*)nullptr;
		auto DistanceToClosestInteractable = 0.0f;
		for (auto&& HitResult : HitResults)
		{
			if (auto HitActor = HitResult.GetActor())
			{
				auto Interactable = Cast<AInteractable>(HitActor);
				if (Interactable && Interactable->IsMovable())
				{
					auto Distance = FVector::Dist(GrabTransform.GetLocation(), Interactable->GetActorLocation());
					if (ClosestInteractable == nullptr || Distance < DistanceToClosestInteractable)
					{
						ClosestInteractable = Interactable;
						DistanceToClosestInteractable = Distance;
					}
				}
			}
		}

		if (ClosestInteractable)
		{
			auto InteractableRoot = ClosestInteractable->GetRootComponent();

			if (auto OtherHand = Cast<UHandGrabbingComponent>(InteractableRoot->GetAttachParent()))
			{
				OtherHand->TryRelease();
			}

			if (InteractableRoot != nullptr)
			{
				InteractableRoot->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);

				if (auto GrabbedPrimitive = Cast<UPrimitiveComponent>(InteractableRoot))
				{
					if (GrabbedPrimitive->IsSimulatingPhysics())
					{
						bGrabbedActorHasPhysics = true;
						GrabbedPrimitive->SetSimulatePhysics(false);
					}
				}

				InteractableRoot->AttachToComponent(this, FAttachmentTransformRules::KeepWorldTransform);
				ClosestInteractable->Interaction1();

				GrabbedActor = ClosestInteractable;
				GrabbedActor->OnDestroyed.AddDynamic(this, &UHandGrabbingComponent::HandleHeldActorDestroyed);
			}
		}
	}

	return GrabbedActor;
}

AInteractable* UHandGrabbingComponent::TryRelease(bool bReenablePhysics)
{
	auto const ReleasedActor = GrabbedActor;
	if (GrabbedActor != nullptr)
	{
		GrabbedActor->OnDestroyed.RemoveDynamic(this, &UHandGrabbingComponent::HandleHeldActorDestroyed);

		auto InteractableRoot = GrabbedActor->GetRootComponent();
		if (InteractableRoot != nullptr)
		{
			InteractableRoot->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);

			auto GrabbedPrimitive = Cast<UPrimitiveComponent>(InteractableRoot);
			if (bReenablePhysics && bGrabbedActorHasPhysics && GrabbedPrimitive != nullptr)
			{
				GrabbedPrimitive->SetSimulatePhysics(true);
			}
		}

		GrabbedActor = nullptr;
		bGrabbedActorHasPhysics = false;
	}
	return ReleasedActor;
}

void UHandGrabbingComponent::HandleHeldActorDestroyed(AActor* DestroyedActor)
{
	if (DestroyedActor == GrabbedActor)
	{
		TryRelease();
	}
}
