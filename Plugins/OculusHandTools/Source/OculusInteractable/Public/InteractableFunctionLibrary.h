// Copyright (c) Meta Platforms, Inc. and affiliates.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "InteractableFunctionLibrary.generated.h"

UCLASS()
class OCULUSINTERACTABLE_API UInteractableFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Locate nearby Interactables and give their relative position and orientation to
	 * some reference object.
	 * @param ReferenceActor 
	 * @param RadiusAroundActor 
	 */
	UFUNCTION(BlueprintCallable, Category = "Interactable")
	static void LogNearbyInteractables(AActor* ReferenceActor, float RadiusAroundActor);

private:
	static ECollisionChannel InteractableTraceChannel;
};
