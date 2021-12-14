// Copyright (c) Meta Platforms, Inc. and affiliates.

#include "TransformString.h"

void FTransformString::TransformToString(FTransform Transform, FString& String)
{
	auto const Location = Transform.GetLocation();
	auto const Rotation = Transform.GetRotation();
	auto const Scale = Transform.GetScale3D();
	String = FString::Printf(
		TEXT("LOC X%0.3f Y%0.3f Z%0.3f ROT W%0.3f X%0.3f Y%0.3f Z%0.3f"),
		Location.X, Location.Y, Location.Z,
		Rotation.W, Rotation.X, Rotation.Y, Rotation.Z);

	float const Tolerance = 0.01;
	if (FMath::Abs(Scale.X - 1.0f) > Tolerance ||
		FMath::Abs(Scale.Y - 1.0f) > Tolerance ||
		FMath::Abs(Scale.Z - 1.0f) > Tolerance)
	{
		String.Append(FString::Printf(
			TEXT(" SCA X%0.3f Y%0.3f Z%0.3f"),
			Scale.X, Scale.Y, Scale.Z));
	}
}

bool FTransformString::StringToTransform(FString String, FTransform& Transform)
{
	FVector Location;
	FQuat Rotation;
	FVector Scale(1.0f, 1.0f, 1.0f);

	TArray<FString> Elems;
	auto const NumElems = String.ParseIntoArray(Elems, TEXT(" "), true);

	if (NumElems != 9 && NumElems != 13) return false;

	if (Elems[0] != TEXT("LOC")) return false;
	if (!ReadFloat(Elems[1], TEXT("X"), Location.X)) return false;
	if (!ReadFloat(Elems[2], TEXT("Y"), Location.Y)) return false;
	if (!ReadFloat(Elems[3], TEXT("Z"), Location.Z)) return false;

	if (Elems[4] != TEXT("ROT")) return false;
	if (!ReadFloat(Elems[5], TEXT("W"), Rotation.W)) return false;
	if (!ReadFloat(Elems[6], TEXT("X"), Rotation.X)) return false;
	if (!ReadFloat(Elems[7], TEXT("Y"), Rotation.Y)) return false;
	if (!ReadFloat(Elems[8], TEXT("Z"), Rotation.Z)) return false;

	if (NumElems == 13)
	{
		if (Elems[9] != TEXT("SCA")) return false;
		if (!ReadFloat(Elems[10], TEXT("X"), Scale.X)) return false;
		if (!ReadFloat(Elems[11], TEXT("Y"), Scale.Y)) return false;
		if (!ReadFloat(Elems[12], TEXT("Z"), Scale.Z)) return false;
	}

	Transform.SetLocation(Location);
	Transform.SetRotation(Rotation);
	Transform.SetScale3D(Scale);
	return true;
}
