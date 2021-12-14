// Copyright (c) Meta Platforms, Inc. and affiliates.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "Containers/CircularBuffer.h"
#include "TransformBufferComponent.generated.h"

UENUM()
enum class ETransformBufferUpdateMode : uint8
{
	Manual,
	EveryFrame
};

USTRUCT(BlueprintType)
struct FTransformBufferData
{
	GENERATED_BODY()

	FTransformBufferData() :
		Transform(FTransform::Identity), Velocity(FVector::ZeroVector)
	{
	}

	FTransformBufferData(FTransform InTransform, FVector InVelocity) :
		Transform(InTransform), Velocity(InVelocity)
	{
	}

	FTransform Transform;
	FVector Velocity;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class OCULUSTHROWASSIST_API UTransformBufferComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	explicit UTransformBufferComponent(FObjectInitializer const& ObjectInitializer);

	/// How the buffer will be updated with new data.
	UPROPERTY(EditAnywhere, Category = "Transform Buffer")
	ETransformBufferUpdateMode UpdateMode = ETransformBufferUpdateMode::Manual;

	/// How long to buffer data.
	UPROPERTY(EditAnywhere, Category = "Transform Buffer")
	float MaxBufferTimeSeconds = 1.0f;

	/// Force an update to the buffer with new data.
	UFUNCTION(BlueprintCallable)
	void BufferCurrentData();

	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/// Get transform data from the buffer. Returns true if the data can be considered reliable, otherwise false.
	UFUNCTION(BlueprintCallable)
	bool GetBufferData(float SecondsAgo, FTransformBufferData& OutBufferData);

private:
	// buffer size assumes max FPS at 90
	TCircularBuffer<TTuple<float, FTransformBufferData>> Buffer{(uint32)FMath::CeilToInt(MaxBufferTimeSeconds * 90)};
	int BufferPosition = 0;

	void DebugDrawBuffer() const;
};
