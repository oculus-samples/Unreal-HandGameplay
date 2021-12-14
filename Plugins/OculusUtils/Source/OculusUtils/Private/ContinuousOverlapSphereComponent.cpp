// Copyright (c) Meta Platforms, Inc. and affiliates.

#include "ContinuousOverlapSphereComponent.h"

#include "OculusUtilsModule.h"

void UContinuousOverlapSphereComponent::SetLocation_Direct(FVector Location)
{
	if (GetAttachParent() != nullptr && !IsUsingAbsoluteLocation())
	{
		auto const ParentToWorld = GetAttachParent()->GetSocketTransform(GetAttachSocketName());
		Location = ParentToWorld.InverseTransformPosition(Location);
	}
	SetRelativeLocation_Direct(Location);
	UpdateComponentToWorld();
}

void UContinuousOverlapSphereComponent::OnUpdateTransform(EUpdateTransformFlags UpdateTransformFlags, ETeleportType Teleport)
{
	// Prevent infinite loop
	if (bIsInOnUpdateTransform)
		return;
	
	bIsInOnUpdateTransform = true;
	
	auto const NewTransform = GetComponentTransform();

	auto const Delta = NewTransform.GetLocation() - LastLocation;
	if (bIsInitialized && Teleport == ETeleportType::None && !Delta.IsNearlyZero())
	{
		SetLocation_Direct(LastLocation);
		MoveComponent(Delta, NewTransform.GetRotation(), true);
	}

	bIsInitialized = true;
	LastLocation = NewTransform.GetLocation();
	
	Super::OnUpdateTransform(UpdateTransformFlags, Teleport);
	
	bIsInOnUpdateTransform = false;
}
