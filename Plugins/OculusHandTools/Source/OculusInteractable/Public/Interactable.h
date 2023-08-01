// Copyright (c) Meta Platforms, Inc. and affiliates.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "InteractablePose.h"
#include "Interactable.generated.h"

class AInteractableSelector;

UENUM(BlueprintType)
enum class EHandSide : uint8
{
	HandLeft,
	HandRight
};

/** Base actor class of interactable objects. */
UCLASS()
class OCULUSINTERACTABLE_API AInteractable : public AActor
{
	GENERATED_BODY()

public:
	/** Sets default values for this actor's properties. */
	AInteractable();

protected:
	/** Called when the game starts or when spawned. */
	virtual void BeginPlay() override;

	/** Called when this actor is being removed from the level. */
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	/** Called every frame. */
	virtual void Tick(float DeltaTime) override;

	/** Minimum time required for the selector to stay on this object before it can be selected in the far-field. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactable Grab Pose")
	float FarFieldSelectionDelayMs;

	/**
	 * Event fired when selection starts.
	 * Can only be called from C++.
	 * @param Selector - The Interactable Selector that just selected us.
	 */
	UFUNCTION(BlueprintNativeEvent)
	void BeginSelection(AInteractableSelector* Selector);

	/**
	 * Event fired when selection ends.
	 * Can only be called from C++.
	 * @param Selector - The Interactable Selector that stopped selecting us.
	 */
	UFUNCTION(BlueprintNativeEvent)
	void EndSelection(AInteractableSelector* Selector);

	/**
	 * Call to check if you are currently selected.
	 * @return boolean
	 */
	UFUNCTION(BlueprintCallable)
	bool IsSelected() const;

	/**
	 * Optional generic user event.
	 * The meaning, implementation and invocation is left to the user.
	 */
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	void Interaction1();

	/**
	 * Optional generic user event.
	 * The meaning, implementation and invocation is left to the user.
	 */
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	void Interaction2();

	/**
	 * Optional generic user event.
	 * The meaning, implementation and invocation is left to the user.
	 */
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	void Interaction3();

	/**
	 * Call to get all selectors currently selecting us.
	 * @return An array of Interactable Selectors.
	 */
	const TArray<AInteractableSelector*>& GetSelectors() const;

	/**
	 * Method called to turn on/off physics on interactable.
	 * By default it checks if the root component is a primitive and applies the change there.
	 * You can override this method in blueprint for special cases.
	 * @param SimulatePhysics - boolean.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void SetInteractablePhysicsSimulation(bool SimulatePhysics);

	/**
	 * Method to check if object is movable.
	 * By default it checks if the root component is movable.
	 * You can override this method in blueprint for special cases.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	bool IsMovable();

	/** Hand poses when grabbed. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Interactable Grab Pose")
	TArray<FInteractablePose> GrabPosesLeftHand;

	/** Hand poses when grabbed. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Interactable Grab Pose")
	TArray<FInteractablePose> GrabPosesRightHand;

	/**
	 * Selects the best grab pose for the interactable, if any.
	 * @param Side - EOculusXRHandType to select left or right hand.
	 * @param GrabPoseFound - returns a boolean indicating if a grab pose was found.
	 * @param GrabPoseName - if GrabPoseFound, returns the hand pose while grabbing.
	 * @param GrabTransform - if GrabPoseFound, returns the Interactable's transform relative to the hand.
	 * @param GrabHandPose - returns the grab pose string
	 */
	UFUNCTION(BlueprintCallable, Category = "Interactable Grab Pose")
	void SelectGrabPose(EHandSide Side, bool& GrabPoseFound, FString& GrabPoseName, FTransform& GrabTransform, FString& GrabHandPose);

protected:
	/** List of selectors currently selecting us. */
	TArray<AInteractableSelector*> Selectors;
};
