// Copyright (c) Meta Platforms, Inc. and affiliates.

#include "HandInputModule.h"
#include "HandInput.h"

#define LOCTEXT_NAMESPACE "FHandInputModule"

#include "OculusDeveloperTelemetry.h"

OCULUS_TELEMETRY_LOAD_MODULE("Unreal-HandInput");

bool FHandInputModule::IsGameModule() const
{
	return true;
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FHandInputModule, HandInput);

DEFINE_LOG_CATEGORY(LogHandInput);
