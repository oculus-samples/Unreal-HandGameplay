// Copyright (c) Meta Platforms, Inc. and affiliates.

#pragma once

#include "Math/Quat.h"

FORCEINLINE FQuat HANDTRACKINGFILTER_API Scale(FQuat Rotation, float S)
{
	return FQuat::Slerp(FQuat::Identity, Rotation, S);
}
