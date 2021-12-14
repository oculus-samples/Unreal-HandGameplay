// Copyright (c) Meta Platforms, Inc. and affiliates.

#pragma once

#include "Components/SphereComponent.h"

#include "ContinuousOverlapSphereComponent.generated.h"

/**
 * Used similarly to SphereComponent, but modified to use continuous "collision" for triggering overlaps.
 * Useful for fast-moving spheres with non-overlap collision disabled.
 */
UCLASS(BlueprintType, meta=(BlueprintSpawnableComponent))
class OCULUSUTILS_API UContinuousOverlapSphereComponent : public USphereComponent
{
	GENERATED_BODY()

	FVector LastLocation;
	bool bIsInitialized = false;
	bool bIsInOnUpdateTransform = false;
	
	void SetLocation_Direct(FVector Location);

public:
	virtual void OnUpdateTransform(EUpdateTransformFlags UpdateTransformFlags, ETeleportType Teleport) override;
};
