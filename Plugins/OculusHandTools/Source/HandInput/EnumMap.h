// Copyright (c) Meta Platforms, Inc. and affiliates.

#pragma once

#include "Containers/StaticArray.h"

template <typename EEnum, typename InElementType, int ExtraElements = 0>
class TEnumMap : public TStaticArray<InElementType, ExtraElements + static_cast<int>(EEnum::Invalid)>
{
	using Super = TStaticArray<InElementType, ExtraElements + static_cast<int>(EEnum::Invalid)>;

public:
	InElementType& operator[](EEnum Index)
	{
		return (*static_cast<Super*>(this))[static_cast<uint32>(Index)];
	}

	InElementType const& operator[](EEnum Index) const
	{
		return (*static_cast<Super const*>(this))[static_cast<uint32>(Index)];
	}
};
