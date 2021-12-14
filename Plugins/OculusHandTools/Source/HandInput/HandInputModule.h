// Copyright (c) Meta Platforms, Inc. and affiliates.

#pragma once

#include "Modules/ModuleInterface.h"

class HANDINPUT_API FHandInputModule : public IModuleInterface
{
public:
	virtual bool IsGameModule() const override;
};
