// Copyright (c) Facebook, Inc. and its affiliates.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Components/ActorComponent.h"
#include "OculusUtilsLibrary.generated.h"

UENUM(BlueprintType)
enum class ETickUntilInputPin : uint8
{
	Start,
	Break
};

/**
 * Build configuration from the project's game configuration.
 */
UCLASS(Config=Game)
class OCULUSUTILS_API UBuildInfo : public UObject
{
	GENERATED_BODY()

public:
	/** Source control changelist */
	UPROPERTY(Config)
	FString PackageChangelist;

	/** Build date and time (Pacific time) "YYYYMMDD HHMMSS" */
	UPROPERTY(Config)
	FString PackageDateAndTime;
};

/**
 * Oculus Utils Blueprint Library.
 */
UCLASS()
class OCULUSUTILS_API UOculusUtilsLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** Returns the Oculus store version. */
	UFUNCTION(BlueprintCallable, Category = "Oculus Utils")
	static bool GetOculusBuildInfo(FString& SourceControlChangelist, FString& BuildDateTimeString);

	/** Returns the array of components sorted by name. */
	UFUNCTION(BlueprintCallable, Category = "Oculus Utils", meta = (ComponentClass = "ActorComponent", DeterminesOutputType = "Components"))
	static TArray<UActorComponent*> SortComponentsByName(const TArray<UActorComponent*>& Components);

	/** 
	* Executes the "Completed" pin every tick until Break is hit. Calling Start again while it is still ticking will be ignored.
	* 
	* @param WorldContextObject	World context.
	* @param InputPin	Which pin is being called.
	* @param LatentInfo 	The latent action.
	*/
	UFUNCTION(BlueprintCallable, Category="Utilities|FlowControl", meta=(Latent, WorldContext="WorldContextObject", LatentInfo="LatentInfo", Keywords="sleep", ExpandEnumAsExecs="InputPin"))
	static void TickUntil(const UObject* WorldContextObject, ETickUntilInputPin InputPin, struct FLatentActionInfo LatentInfo);
};
