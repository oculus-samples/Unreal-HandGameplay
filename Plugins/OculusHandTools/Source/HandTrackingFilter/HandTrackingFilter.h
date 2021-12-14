// Copyright (c) Meta Platforms, Inc. and affiliates.

#pragma once

#include "Modules/ModuleInterface.h"

class HANDTRACKINGFILTER_API FHandTrackingFilterModule : public IModuleInterface
{
public:
	virtual bool IsGameModule() const override;
};
