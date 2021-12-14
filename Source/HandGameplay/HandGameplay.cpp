// Copyright (c) Meta Platforms, Inc. and affiliates.

#include "HandGameplay.h"
#include "Modules/ModuleManager.h"

#include "OculusDeveloperTelemetry.h"

OCULUS_TELEMETRY_LOAD_MODULE("Unreal-HandGameplayShowcase");

IMPLEMENT_PRIMARY_GAME_MODULE(
	FDefaultGameModuleImpl,
	HandGameplay,
	"HandGameplay"
);

DEFINE_LOG_CATEGORY(LogHandGameplayShowcase);
