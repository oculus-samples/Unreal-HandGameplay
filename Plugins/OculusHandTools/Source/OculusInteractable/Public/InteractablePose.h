// Copyright (c) Meta Platforms, Inc. and affiliates.

#pragma once

#include "CoreMinimal.h"
#include "InteractablePose.generated.h"

/** A struct that represents a hand pose. */
USTRUCT(BlueprintType)
struct OCULUSINTERACTABLE_API FInteractablePose
{
	GENERATED_BODY()

	/** Name for this pose. */
	UPROPERTY(Category = "Interactable Grab Pose", EditAnywhere, BlueprintReadWrite)
	FString PoseName;

	/** Hand pose. */
	UPROPERTY(Category = "Interactable Grab Pose", EditAnywhere, BlueprintReadWrite)
	FString HandPose;

	/** Relative hand transform. */
	UPROPERTY(Category = "Interactable Grab Pose", EditAnywhere, BlueprintReadWrite)
	FString RelativeHandTransform;
};
