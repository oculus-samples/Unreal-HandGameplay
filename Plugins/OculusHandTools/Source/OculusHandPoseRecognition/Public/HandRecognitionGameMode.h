// Copyright (c) Meta Platforms, Inc. and affiliates.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Misc/CoreMisc.h"
#include "HandRecognitionGameMode.generated.h"

/**
 * A game mode that offers some console commands to write poses to the log.
 */
UCLASS()
class OCULUSHANDPOSERECOGNITION_API AHandRecognitionGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	/**
	 * Prints a hand pose string in the output log.
	 * @param Side - Either "left" or "right", case insensitive.
	 */
	UFUNCTION(Exec)
	void LogHandPose(FString Side);

private:
	/** Helps keep track of poses logged. */
	static int LoggedIndex;
};
