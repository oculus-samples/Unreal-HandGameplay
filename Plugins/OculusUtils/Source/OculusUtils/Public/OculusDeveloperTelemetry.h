// Copyright (c) Facebook, Inc. and its affiliates.

#pragma once

#include "CoreMinimal.h"

#if WITH_EDITOR
#include "IDetailCustomization.h"
#endif

#include "OculusDeveloperTelemetry.generated.h"

UCLASS(Config=EditorPerProjectUserSettings, meta=(DisplayName="Oculus Developer Telemetry"))
class OCULUSUTILS_API UOculusDeveloperTelemetry : public UObject
{
	GENERATED_BODY()

	DECLARE_MULTICAST_DELEGATE(FOnFlushEvent);
	FOnFlushEvent OnFlush;

public:
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	
	UPROPERTY(EditAnywhere, Config, meta=(Tooltip="Enabling telemetry will transmit data to Oculus about your usage of its samples and tools."))
	bool bIsEnabled = false;
	
	UPROPERTY(Config)
	bool bHasPrompted = false;
	
	void SendEvent(const char* eventName, const char* param, const char* source);
	void Flush();
};

#if WITH_EDITOR
class FOculusTelemetrySettingsCustomization : public IDetailCustomization
{
public:
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;
};
#endif

#if WITH_EDITOR
	#define OCULUS_TELEMETRY_LOAD_MODULE(ModuleName) \
		static FDelayedAutoRegisterHelper ANONYMOUS_VARIABLE(GMetricsHelper) (EDelayedRegisterRunPhase::EndOfEngineInit, [] \
		{ \
			GetMutableDefault<UOculusDeveloperTelemetry>()->SendEvent("module_loaded", ModuleName, "integration"); \
		});
#else
	#define OCULUS_TELEMETRY_LOAD_MODULE(...)
#endif
