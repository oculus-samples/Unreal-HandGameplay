// Copyright (c) Meta Platforms, Inc. and affiliates.

#include "TransformBufferComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"
#include "OculusThrowAssistModule.h"

static TAutoConsoleVariable<int> CVarDebugDrawTransformBuffer(
	TEXT("mnux.DebugDrawTransformBuffer"),
	0,
	TEXT("Draws all transforms in the transform buffer"),
	ECVF_Cheat);

UTransformBufferComponent::UTransformBufferComponent(FObjectInitializer const& ObjectInitializer) :
	Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UTransformBufferComponent::TickComponent(float DeltaTime, enum ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (UpdateMode == ETransformBufferUpdateMode::EveryFrame)
	{
		BufferCurrentData();
	}

#if !UE_BUILD_SHIPPING
	if (CVarDebugDrawTransformBuffer.GetValueOnAnyThread() > 0)
	{
		DebugDrawBuffer();
	}
#endif
}

void UTransformBufferComponent::BufferCurrentData()
{
	auto Timestamp = GetWorld()->GetTimeSeconds();
	auto const Transform = GetComponentTransform();
	auto const PrevIndex = BufferPosition == 0 ? (int)Buffer.Capacity() - 1 : BufferPosition - 1;
	auto const PrevTransform = Buffer[PrevIndex].Value.Transform;
	auto Velocity = FVector::ZeroVector;
	if (!PrevTransform.Equals(FTransform::Identity))
	{
		auto const Delta = Transform.GetLocation() - PrevTransform.GetLocation();
		auto const Time = Timestamp - Buffer[PrevIndex].Key;
		if (Time > 0)
		{
			Velocity = Delta / Time;
			Buffer[BufferPosition] = TTuple<float, FTransformBufferData>(
				Timestamp,
				FTransformBufferData(GetComponentTransform(), Velocity)
			);
			++BufferPosition;
		}
	}
	else
	{
		Buffer[BufferPosition] = TTuple<float, FTransformBufferData>(
			Timestamp,
			FTransformBufferData(GetComponentTransform(), Velocity)
		);
		++BufferPosition;
	}
}

bool UTransformBufferComponent::GetBufferData(float SecondsAgo, FTransformBufferData& OutBufferData)
{
	if (SecondsAgo <= 0)
	{
		int Index = BufferPosition == 0 ? Buffer.Capacity() - 1 : BufferPosition - 1;
		OutBufferData = Buffer[Index].Value;
		return true;
	}

	if (SecondsAgo > MaxBufferTimeSeconds)
	{
		UE_LOG(LogOculusThrowAssist, Warning,
			TEXT("UTransformBufferComponent::GetTransform: SecondsAgo can not be greater than MaxBufferTimeSeconds."
			));
		SecondsAgo = MaxBufferTimeSeconds;
	}

	// find the pair of buffer values to interpolate between
	auto LookupTime = GetWorld()->GetTimeSeconds() - SecondsAgo;
	auto Index = BufferPosition - 1;

	// check for the case that the most recent buffered value is older than the lookup time
	if (Buffer[Index].Key <= LookupTime)
	{
		UE_LOG(LogOculusThrowAssist, Warning,
			TEXT("UTransformBufferComponent::GetTransform: No data recent enough for an accurate result."));
		OutBufferData = Buffer[Index].Value;
		return false;
	}

	int const BufferCapacity = Buffer.Capacity();
	for (auto i = 0; i < BufferCapacity; ++i)
	{
		--Index;
		if (Index < 0)
		{
			Index = BufferCapacity + Index;
		}

		auto BufferTime = Buffer[Index].Key;
		if (BufferTime <= LookupTime)
		{
			auto NextElementTime = Buffer[Index + 1].Key;
			auto t = (LookupTime - BufferTime) / (NextElementTime - BufferTime);
			auto PrevData = Buffer[Index].Value;
			auto NextData = Buffer[Index + 1].Value;
			auto Transform = UKismetMathLibrary::TLerp(PrevData.Transform, NextData.Transform, t);
			auto Velocity = FMath::Lerp(PrevData.Velocity, NextData.Velocity, t);

			OutBufferData = FTransformBufferData(Transform, Velocity);

			auto const MaxPeriodForReliableData = 0.1f;
			return NextElementTime - BufferTime < MaxPeriodForReliableData;
		}
	}

	UE_LOG(LogOculusThrowAssist, Warning,
		TEXT("UTransformBufferComponent::GetTransform: No data old enough for an accurate result."));
	OutBufferData = Buffer[BufferPosition].Value; // return the oldest (could be default value)
	return false;
}

void UTransformBufferComponent::DebugDrawBuffer() const
{
	auto const TimeNow = GetWorld()->GetTimeSeconds();
	auto const OldestTime = TimeNow - MaxBufferTimeSeconds;
	auto const MaxScale = 5.0f;

	for (uint32 i = 0; i < Buffer.Capacity(); ++i)
	{
		auto BufferedTransform = Buffer[i].Value.Transform;
		auto const BufferedTimestamp = Buffer[i].Key;
		auto const NormalizedTime = FMath::GetMappedRangeValueClamped(
			FVector2D(OldestTime, TimeNow), FVector2D(0, 1), BufferedTimestamp);
		auto const Scale = NormalizedTime * MaxScale;
		DrawDebugCoordinateSystem(GetWorld(), BufferedTransform.GetLocation(), BufferedTransform.Rotator(), Scale);

		auto BufferedVelocity = Buffer[i].Value.Velocity;
		DrawDebugDirectionalArrow(
			GetWorld(),
			BufferedTransform.GetLocation(),
			BufferedTransform.GetLocation() + BufferedVelocity * 0.2f,
			1.f,
			FColor::Orange,
			false,
			-1,
			0,
			.3f);
	}
}
