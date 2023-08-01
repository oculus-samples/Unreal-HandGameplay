// Copyright (c) Meta Platforms, Inc. and affiliates.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AimingActor.generated.h"

UCLASS()
class OCULUSINTERACTABLE_API AAimingActor : public AActor
{
	GENERATED_BODY()

public:
	/** Sets default values for this actor's properties. */
	AAimingActor();

protected:
	/** Called when the game starts or when spawned. */
	virtual void BeginPlay() override;

public:
	/** Called every frame. */
	virtual void Tick(float DeltaTime) override;

	/**
	 * Called by the selector when the aiming actor is activated.
	 * By default, it makes the actor visible.
	 */
	UFUNCTION(BlueprintNativeEvent)
	void Activate();

	/**
	 * Called by the selector when the aiming actor is deactivated.
	 * By default, it makes the actor invisible.
	 * @param Selected - Interactable selected, if any.
	 */
	UFUNCTION(BlueprintNativeEvent)
	void Deactivate(AInteractable* Selected);
};
