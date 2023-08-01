// Copyright (c) Meta Platforms, Inc. and affiliates.

#pragma once

#include "CoreMinimal.h"
#include "OculusXRInputFunctionLibrary.h"
#include "HandPose.generated.h"

/** Bones that we care about. */
UENUM()
enum ERecognizedBone
{
	Thumb_0 = 0,
	Thumb_1,
	Thumb_2,
	Thumb_3,
	Index_1,
	Index_2,
	Index_3,
	Middle_1,
	Middle_2,
	Middle_3,
	Ring_1,
	Ring_2,
	Ring_3,
	Pinky_0,
	Pinky_1,
	Pinky_2,
	Pinky_3,
	Wrist,
	NUM
};

/** A struct that represents a hand pose. */
USTRUCT(BlueprintType)
struct OCULUSHANDPOSERECOGNITION_API FHandPose
{
	GENERATED_BODY()

public:
	/** Name for this pose. */
	UPROPERTY(Category = "Hand Pose", EditAnywhere, BlueprintReadWrite)
	FString PoseName;

	/** Hand pose encoded in a string. */
	UPROPERTY(Category = "Hand Pose", EditAnywhere, BlueprintReadWrite)
	FString CustomEncodedPose;

	/** Hand pose custom confidence floor, when greater than 0. */
	UPROPERTY(Category = "Hand Pose", EditAnywhere, BlueprintReadWrite)
	float CustomConfidenceFloor = 0.0f;

	/**
	 * Hand pose error at max confidence of 1.0.
	 * Confidence of 0.50 corresponds to twice this error.
	 * Confidence of 0.25 corresponds to four times this error.
	 * Confidence of 0.10 corresponds to ten times this error.
	 */
	UPROPERTY(Category = "Hand Pose", EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "100.0", UIMin = "100.0"))
	float ErrorAtMaxConfidence = 2000.0f;

	/** Returns the side that this */
	EOculusXRHandType GetHandType() const
	{
		return Hand;
	}

	/**
	 * Rotator access.
	 * @param Bone
	 * @return A const reference to the rotator of that bone.
	 */
	const FRotator& GetRotator(ERecognizedBone Bone) const
	{
		return Rotations[Bone];
	}

	/**
	* Weight access.
	* @param Bone
	* @return The weight of the given bone.
	*/
	float GetWeight(ERecognizedBone Bone) const
	{
		return Weights[Bone];
	}

	/**
	 * Updates the structure with the current bone rotations for the side specified.
	 * Wrist information is received from the HandPoseRecognizer.
	 *
	 * @param Side - EOculusXRHandType to track
	 * @param Wrist - FRotator from the controller.
	 */
	void UpdatePose(EOculusXRHandType Hand, FRotator Wrist);

	/** Encodes rotators to string form, without weights. */
	void Encode();

	/**
	 * Decodes the encoded pose string into rotators and weights.
	 * This is always called to configure references poses.
	 * @return True if there were no issues during decoding.
	 */
	bool Decode();

	/**
	 * Computes the confidence of the other pose being similar to this reference pose.
	 * @param Other - The hand pose to evaluate.
	 * @param RawError - The raw error (optional).
	 * @return The confidence level.
	 * @warning You should call this method on a reference pose.
	 */
	float ComputeConfidence(const FHandPose& Other, float* RawError = nullptr) const;

	/**
	 * Used for average bone rotations: weighted transition to another pose.
	 * @param Other - Hand pose.
	 * @param OtherRatio - How much of the other pose we take.
	 */
	void AddWeighted(const FHandPose& Other, float OtherRatio);

	/**
	 * Used to record the minimum bone angles.
	 * Copies all pitch/yaw/roll angles from Other that are smaller that our current ones.
 	 * @param Other - Hand pose.
	 */
	void Min(const FHandPose& Other);

	/**
	 * Used to record the maximum bone angles.
	 * Copies all pitch/yaw/roll angles from Other that are greater that our current ones.
	 * @param Other - Hand pose.
	 */
	void Max(const FHandPose& Other);

protected:
	/** Hand side that will be set during parsing. */
	EOculusXRHandType Hand = EOculusXRHandType::None;

	/** Hand bone rotators. */
	FRotator Rotations[NUM];

	/** Hand bone weights. */
	float Weights[NUM] = {};

private:
	static int NormalizedOutputAngle(float Angle);
	static FString FmtRot(FString Prefix, FRotator R);
	static void SkipWhitespace(const TCHAR** Buffer);
	static bool ReadRotComp(const TCHAR** Buffer, double* RotComp);
	static bool ReadWeight(const TCHAR** Buffer, float* Weight);
	static bool ReadRot(const TCHAR** Buffer, const TCHAR* Prefix, FRotator& R, float& Weight);
	static float ComputeAngleError(float Ref, float Angle);
	static float RotError(ERecognizedBone Bone, const FRotator* RefRot, const float* RefWeight, const FRotator* OtherRot);
};
