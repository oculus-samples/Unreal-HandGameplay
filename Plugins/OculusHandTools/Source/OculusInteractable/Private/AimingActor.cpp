// Copyright (c) Meta Platforms, Inc. and affiliates.

#include "AimingActor.h"

// Sets default values
AAimingActor::AAimingActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AAimingActor::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void AAimingActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called when the actor is activated
void AAimingActor::Activate_Implementation()
{
	SetActorHiddenInGame(false);
}

// Called when the actor is deactivated
void AAimingActor::Deactivate_Implementation(AInteractable* Selected)
{
	SetActorHiddenInGame(true);
}
