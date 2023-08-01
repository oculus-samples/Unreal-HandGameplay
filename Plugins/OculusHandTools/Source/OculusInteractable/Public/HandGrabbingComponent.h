// Copyright (c) Meta Platforms, Inc. and affiliates.

#pragma once

#include "CoreMinimal.h"

#include "Interactable.h"
#include "OculusXRInputFunctionLibrary.h"

#include "HandGrabbingComponent.generated.h"

class OCULUSINTERACTABLE_API AInteractable;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class OCULUSINTERACTABLE_API UHandGrabbingComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Grabbing")
	AInteractable* TryGrab(FTransform GrabTransform);

	UFUNCTION(BlueprintCallable, Category = "Grabbing")
	AInteractable* TryRelease(bool bReenablePhysics = true);

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Grabbing")
	EOculusXRHandType Hand = EOculusXRHandType::HandLeft;

	UPROPERTY(BlueprintReadOnly, Transient, Category = "Grabbing")
	AInteractable* GrabbedActor = nullptr;

	UPROPERTY(BlueprintReadOnly, Transient, Category = "Grabbing")
	bool bGrabbedActorHasPhysics = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grabbing")
	float GrabCapsuleHeight = 30.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grabbing")
	float GrabCapsuleRadius = 20.f;

private:
	UFUNCTION()
	void HandleHeldActorDestroyed(AActor* DestroyedActor);
};
