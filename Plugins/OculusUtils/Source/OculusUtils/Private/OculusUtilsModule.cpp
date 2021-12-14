// Copyright (c) Facebook, Inc. and its affiliates.

#include "OculusUtilsModule.h"

#if WITH_EDITOR
#include "ISettingsModule.h"
#endif

#include "OculusDeveloperTelemetry.h"

OCULUS_TELEMETRY_LOAD_MODULE("Unreal-OculusUtils");

#define LOCTEXT_NAMESPACE "FOculusUtilsModule"

DEFINE_LOG_CATEGORY(LogOculusUtils);

void FOculusUtilsModule::StartupModule()
{
#if WITH_EDITOR
	auto&& PropertyModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyModule.RegisterCustomClassLayout(
		"OculusDeveloperTelemetry",
		FOnGetDetailCustomizationInstance::CreateLambda([]
		{ 
			return MakeShared<FOculusTelemetrySettingsCustomization>(); 
		}));

	auto SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings");
	SettingsModule->RegisterSettings(
		"Editor", "Privacy", "Oculus Developer Telemetry",
		LOCTEXT("PrivacyAnalyticsSettingsName", "Oculus Developer Telemetry"),
		LOCTEXT("PrivacyAnalyticsSettingsDescription", "Configure the way your Editor usage information is handled."),
		GetMutableDefault<UOculusDeveloperTelemetry>());
#endif
}

void FOculusUtilsModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FOculusUtilsModule, OculusUtils)
