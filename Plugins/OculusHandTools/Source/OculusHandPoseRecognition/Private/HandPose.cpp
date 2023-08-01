// Copyright (c) Meta Platforms, Inc. and affiliates.

#include "HandPose.h"
#include "OculusXRInputFunctionLibrary.h"
#include <limits>

void FHandPose::UpdatePose(EOculusXRHandType Side, FRotator Wrist)
{
	Hand = Side;
	Rotations[Thumb_0] = UOculusXRInputFunctionLibrary::GetBoneRotation(Side, EOculusXRBone::Thumb_0).Rotator();
	Rotations[Thumb_1] = UOculusXRInputFunctionLibrary::GetBoneRotation(Side, EOculusXRBone::Thumb_1).Rotator();
	Rotations[Thumb_2] = UOculusXRInputFunctionLibrary::GetBoneRotation(Side, EOculusXRBone::Thumb_2).Rotator();
	Rotations[Thumb_3] = UOculusXRInputFunctionLibrary::GetBoneRotation(Side, EOculusXRBone::Thumb_3).Rotator();
	Rotations[Index_1] = UOculusXRInputFunctionLibrary::GetBoneRotation(Side, EOculusXRBone::Index_1).Rotator();
	Rotations[Index_2] = UOculusXRInputFunctionLibrary::GetBoneRotation(Side, EOculusXRBone::Index_2).Rotator();
	Rotations[Index_3] = UOculusXRInputFunctionLibrary::GetBoneRotation(Side, EOculusXRBone::Index_3).Rotator();
	Rotations[Middle_1] = UOculusXRInputFunctionLibrary::GetBoneRotation(Side, EOculusXRBone::Middle_1).Rotator();
	Rotations[Middle_2] = UOculusXRInputFunctionLibrary::GetBoneRotation(Side, EOculusXRBone::Middle_2).Rotator();
	Rotations[Middle_3] = UOculusXRInputFunctionLibrary::GetBoneRotation(Side, EOculusXRBone::Middle_3).Rotator();
	Rotations[Ring_1] = UOculusXRInputFunctionLibrary::GetBoneRotation(Side, EOculusXRBone::Ring_1).Rotator();
	Rotations[Ring_2] = UOculusXRInputFunctionLibrary::GetBoneRotation(Side, EOculusXRBone::Ring_2).Rotator();
	Rotations[Ring_3] = UOculusXRInputFunctionLibrary::GetBoneRotation(Side, EOculusXRBone::Ring_3).Rotator();
	Rotations[Pinky_0] = UOculusXRInputFunctionLibrary::GetBoneRotation(Side, EOculusXRBone::Pinky_0).Rotator();
	Rotations[Pinky_1] = UOculusXRInputFunctionLibrary::GetBoneRotation(Side, EOculusXRBone::Pinky_1).Rotator();
	Rotations[Pinky_2] = UOculusXRInputFunctionLibrary::GetBoneRotation(Side, EOculusXRBone::Pinky_2).Rotator();
	Rotations[Pinky_3] = UOculusXRInputFunctionLibrary::GetBoneRotation(Side, EOculusXRBone::Pinky_3).Rotator();
	Rotations[ERecognizedBone::Wrist] = Wrist;
}

void FHandPose::Encode()
{
	CustomEncodedPose.Empty(1024);

	CustomEncodedPose
		.Append(Hand == EOculusXRHandType::HandLeft ? "L" : "R")
		.Append(*FmtRot(TEXT(" T0"), Rotations[Thumb_0]))
		.Append(*FmtRot(TEXT(" T1"), Rotations[Thumb_1]))
		.Append(*FmtRot(TEXT(" T2"), Rotations[Thumb_2]))
		.Append(*FmtRot(TEXT(" T3"), Rotations[Thumb_3]))
		.Append(*FmtRot(TEXT("  I1"), Rotations[Index_1]))
		.Append(*FmtRot(TEXT(" I2"), Rotations[Index_2]))
		.Append(*FmtRot(TEXT(" I3"), Rotations[Index_3]))
		.Append(*FmtRot(TEXT("  M1"), Rotations[Middle_1]))
		.Append(*FmtRot(TEXT(" M2"), Rotations[Middle_2]))
		.Append(*FmtRot(TEXT(" M3"), Rotations[Middle_3]))
		.Append(*FmtRot(TEXT("  R1"), Rotations[Ring_1]))
		.Append(*FmtRot(TEXT(" R2"), Rotations[Ring_2]))
		.Append(*FmtRot(TEXT(" R3"), Rotations[Ring_3]))
		.Append(*FmtRot(TEXT("  P0"), Rotations[Pinky_0]))
		.Append(*FmtRot(TEXT(" P1"), Rotations[Pinky_1]))
		.Append(*FmtRot(TEXT(" P2"), Rotations[Pinky_2]))
		.Append(*FmtRot(TEXT(" P3"), Rotations[Pinky_3]))
		.Append(*FmtRot(TEXT("  W"), Rotations[Wrist]));
}

bool FHandPose::Decode()
{
	const TCHAR* Buffer = CustomEncodedPose.GetCharArray().GetData();
	if (!Buffer)
	{
		Hand = EOculusXRHandType::None;
		return false;
	}

	SkipWhitespace(&Buffer);

	// Hand
	if (Buffer && *Buffer == 'L')
	{
		Hand = EOculusXRHandType::HandLeft;
	}
	else if (Buffer && *Buffer == 'R')
	{
		Hand = EOculusXRHandType::HandRight;
	}
	else
	{
		Hand = EOculusXRHandType::None;
		return false;
	}
	++Buffer;

	// Rotators
	auto const Successful =
		ReadRot(&Buffer, TEXT("T0"), Rotations[Thumb_0], Weights[Thumb_0]) &&
		ReadRot(&Buffer, TEXT("T1"), Rotations[Thumb_1], Weights[Thumb_1]) &&
		ReadRot(&Buffer, TEXT("T2"), Rotations[Thumb_2], Weights[Thumb_2]) &&
		ReadRot(&Buffer, TEXT("T3"), Rotations[Thumb_3], Weights[Thumb_3]) &&
		ReadRot(&Buffer, TEXT("I1"), Rotations[Index_1], Weights[Index_1]) &&
		ReadRot(&Buffer, TEXT("I2"), Rotations[Index_2], Weights[Index_2]) &&
		ReadRot(&Buffer, TEXT("I3"), Rotations[Index_3], Weights[Index_3]) &&
		ReadRot(&Buffer, TEXT("M1"), Rotations[Middle_1], Weights[Middle_1]) &&
		ReadRot(&Buffer, TEXT("M2"), Rotations[Middle_2], Weights[Middle_2]) &&
		ReadRot(&Buffer, TEXT("M3"), Rotations[Middle_3], Weights[Middle_3]) &&
		ReadRot(&Buffer, TEXT("R1"), Rotations[Ring_1], Weights[Ring_1]) &&
		ReadRot(&Buffer, TEXT("R2"), Rotations[Ring_2], Weights[Ring_2]) &&
		ReadRot(&Buffer, TEXT("R3"), Rotations[Ring_3], Weights[Ring_3]) &&
		ReadRot(&Buffer, TEXT("P0"), Rotations[Pinky_0], Weights[Pinky_0]) &&
		ReadRot(&Buffer, TEXT("P1"), Rotations[Pinky_1], Weights[Pinky_1]) &&
		ReadRot(&Buffer, TEXT("P2"), Rotations[Pinky_2], Weights[Pinky_2]) &&
		ReadRot(&Buffer, TEXT("P3"), Rotations[Pinky_3], Weights[Pinky_3]) &&
		ReadRot(&Buffer, TEXT("W"), Rotations[Wrist], Weights[Wrist]);

	if (!Successful)
	{
		Hand = EOculusXRHandType::None;
	}

	return Successful;
}

float FHandPose::ComputeConfidence(const FHandPose& Other, float* RawError /* = nullptr */) const
{
	auto const Err =
		RotError(Thumb_0, Rotations, Weights, Other.Rotations) +
		RotError(Thumb_1, Rotations, Weights, Other.Rotations) +
		RotError(Thumb_2, Rotations, Weights, Other.Rotations) +
		RotError(Thumb_3, Rotations, Weights, Other.Rotations) +
		RotError(Index_1, Rotations, Weights, Other.Rotations) +
		RotError(Index_2, Rotations, Weights, Other.Rotations) +
		RotError(Index_3, Rotations, Weights, Other.Rotations) +
		RotError(Index_1, Rotations, Weights, Other.Rotations) +
		RotError(Middle_1, Rotations, Weights, Other.Rotations) +
		RotError(Middle_2, Rotations, Weights, Other.Rotations) +
		RotError(Middle_3, Rotations, Weights, Other.Rotations) +
		RotError(Ring_1, Rotations, Weights, Other.Rotations) +
		RotError(Ring_2, Rotations, Weights, Other.Rotations) +
		RotError(Ring_3, Rotations, Weights, Other.Rotations) +
		RotError(Pinky_0, Rotations, Weights, Other.Rotations) +
		RotError(Pinky_1, Rotations, Weights, Other.Rotations) +
		RotError(Pinky_2, Rotations, Weights, Other.Rotations) +
		RotError(Pinky_3, Rotations, Weights, Other.Rotations) +
		RotError(Wrist, Rotations, Weights, Other.Rotations);

	auto const MinErr = FMath::Max(ErrorAtMaxConfidence, 100.0f);
	auto const Confidence = MinErr / FMath::Max(Err, MinErr);

	if (RawError)
	{
		*RawError = Err;
	}

	return Confidence;
}

void FHandPose::AddWeighted(const FHandPose& Other, float OtherRatio)
{
	OtherRatio = FMath::Clamp(OtherRatio, 0.0f, 1.0f);

	for (auto Bone = 0; Bone < NUM; ++Bone)
	{
		Rotations[Bone] *= 1.0f - OtherRatio;
		Rotations[Bone] += Other.Rotations[Bone] * OtherRatio;
	}
}

void FHandPose::Min(const FHandPose& Other)
{
	for (auto Bone = 0; Bone < NUM; ++Bone)
	{
		Rotations[Bone].Pitch = FMath::Min(Rotations[Bone].Pitch, Other.Rotations[Bone].Pitch);
		Rotations[Bone].Yaw = FMath::Min(Rotations[Bone].Yaw, Other.Rotations[Bone].Yaw);
		Rotations[Bone].Roll = FMath::Min(Rotations[Bone].Roll, Other.Rotations[Bone].Roll);
	}
}

void FHandPose::Max(const FHandPose& Other)
{
	for (auto Bone = 0; Bone < NUM; ++Bone)
	{
		Rotations[Bone].Pitch = FMath::Max(Rotations[Bone].Pitch, Other.Rotations[Bone].Pitch);
		Rotations[Bone].Yaw = FMath::Max(Rotations[Bone].Yaw, Other.Rotations[Bone].Yaw);
		Rotations[Bone].Roll = FMath::Max(Rotations[Bone].Roll, Other.Rotations[Bone].Roll);
	}
}

int FHandPose::NormalizedOutputAngle(float Angle)
{
	// We never return 0, as it is used in reference poses to ignore angles.
	auto const IntAngle = static_cast<int>(roundf(Angle));
	return IntAngle ? IntAngle : 1;
}

FString FHandPose::FmtRot(FString Prefix, FRotator R)
{
	// Never output 0 degree angles, as they are used to disable comparisons.
	auto const Pitch = NormalizedOutputAngle(R.Pitch);
	auto const Yaw = NormalizedOutputAngle(R.Yaw);
	auto const Roll = NormalizedOutputAngle(R.Roll);

	return FString::Printf(TEXT("%s%+0d%+0d%+0d"), *Prefix, Pitch, Yaw, Roll);
}

void FHandPose::SkipWhitespace(const TCHAR** Buffer)
{
	while (**Buffer == ' ' || **Buffer == '\t')
		++*Buffer;
}

bool FHandPose::ReadRotComp(const TCHAR** Buffer, double* RotComp)
{
	SkipWhitespace(Buffer);

	auto Negative = false;

	if (**Buffer == '+')
	{
		++*Buffer;
	}
	else if (**Buffer == '-')
	{
		Negative = true;
		++*Buffer;
	}
	else
	{
		return false;
	}

	*RotComp = 0.0;
	while (**Buffer >= '0' && **Buffer <= '9')
	{
		*RotComp *= 10;
		*RotComp += **Buffer - '0';
		++*Buffer;
	}

	if (Negative)
	{
		*RotComp *= -1.0;
	}

	return true;
}

bool FHandPose::ReadWeight(const TCHAR** Buffer, float* Weight)
{
	SkipWhitespace(Buffer);

	// Check for weight marker
	if (**Buffer != '*')
	{
		*Weight = 1.0f; // Defaults to 1
		return true;
	}
	++*Buffer;

	SkipWhitespace(Buffer);

	// Read weight factor
	auto Denominator = 0.0f;
	auto Value = 0.0f;
	while (**Buffer >= '0' && **Buffer <= '9' || **Buffer == '.')
	{
		if (**Buffer == '.')
		{
			Denominator = 1.0f;
		}
		else
		{
			Value *= 10.0f;
			Value += **Buffer - '0';
			Denominator *= 10.0f; // Stays 0.0 as long as we have not seen the decimal point
		}

		++*Buffer;
	}

	if (Denominator > 0.0f)
	{
		Value /= Denominator;
	}

	*Weight = Value;
	return true;
}

bool FHandPose::ReadRot(const TCHAR** Buffer, const TCHAR* Prefix, FRotator& R, float& Weight)
{
	SkipWhitespace(Buffer);

	// Check if prefix matches
	auto PrefixPtr = Prefix;
	auto BufferPtr = *Buffer;

	while (*PrefixPtr && *BufferPtr && *PrefixPtr == *BufferPtr)
	{
		PrefixPtr++;
		BufferPtr++;
	}

	if (*PrefixPtr)
	{
		// Prefix mismatch, this may be a missing bone.
		R.Pitch = R.Yaw = R.Roll = 0.0f;
		Weight = 0.0f;
		return true;
	}

	// Looks good, let's get the rotations and optional weight
	*Buffer = BufferPtr;
	return
		ReadWeight(Buffer, &Weight) &&
		ReadRotComp(Buffer, &R.Pitch) &&
		ReadRotComp(Buffer, &R.Yaw) &&
		ReadRotComp(Buffer, &R.Roll);
}

float FHandPose::ComputeAngleError(float Ref, float Angle)
{
	// A reference angle of 0.0 is ignored.
	if (Ref == 0.0f) return 0.0f;

	// We find the minimum angle and square it
	auto const DeltaAngleDegrees = FMath::FindDeltaAngleDegrees(Ref, Angle);
	return DeltaAngleDegrees * DeltaAngleDegrees;
}

float FHandPose::RotError(ERecognizedBone Bone, const FRotator* RefRot, const float* RefWeight, const FRotator* OtherRot)
{
	return
		RefWeight[Bone] *
		(ComputeAngleError(RefRot[Bone].Pitch, OtherRot[Bone].Pitch) +
			ComputeAngleError(RefRot[Bone].Yaw, OtherRot[Bone].Yaw) +
			ComputeAngleError(RefRot[Bone].Roll, OtherRot[Bone].Roll));
}
