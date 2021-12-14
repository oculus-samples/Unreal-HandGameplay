// Copyright (c) Meta Platforms, Inc. and affiliates.

#include "HandTrackingFilter.h"

#define LOCTEXT_NAMESPACE "FHandTrackingFilterModule"

#include "OculusDeveloperTelemetry.h"

OCULUS_TELEMETRY_LOAD_MODULE("Unreal-HandTrackingFilter");

bool FHandTrackingFilterModule::IsGameModule() const
{
	return true;
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FHandTrackingFilterModule, HandTrackingFilter);
