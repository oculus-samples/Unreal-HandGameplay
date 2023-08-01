// Copyright (c) Meta Platforms, Inc. and affiliates.

#pragma once

#include "CoreMinimal.h"

/**
 * Class for transform encoding.
 */
class FTransformString
{
public:
	/**
	 * Transform to String.
	 * @param Transform - Transform to convert to string.
	 * @param String - Output string.
	 */
	static void TransformToString(FTransform Transform, FString& String);

	/**
	 * String to Transform.
	 * @param String - String encoded transform to convert to Transform.
	 * @param Transform - Decoded Transform.
	 * @return bool - Whether the encoded string is proper.
	 */
	static bool StringToTransform(FString String, FTransform& Transform);

private:
	/**
	 * Parsing float from string, with expected prefix.
	 * @param Source - Transform sub-string of the form <letter><float>.
	 * @param ExpectedPrefix - The expected one-letter prefix for this sub-field.
	 * @param Destination - Output float.
	 * @return bool - True if prefix matches.
	 */
	static bool ReadFloat(FString& Source, const TCHAR* ExpectedPrefix, double& Destination)
	{
		if (**Source != *ExpectedPrefix) return false;
		Destination = FCString::Atof(*Source + 1);
		return true;
	}
};
