// Copyright (c) Meta Platforms, Inc. and affiliates.

#include "OculusInteractableModule.h"

#define LOCTEXT_NAMESPACE "FOculusInteractableModule"

#include "OculusDeveloperTelemetry.h"

OCULUS_TELEMETRY_LOAD_MODULE("Unreal-OculusInteractable");

DEFINE_LOG_CATEGORY(LogInteractable);

void FOculusInteractableModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
}

void FOculusInteractableModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FOculusInteractableModule, OculusInteractable)
