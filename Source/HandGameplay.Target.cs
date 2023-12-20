// Copyright (c) Meta Platforms, Inc. and affiliates.

using UnrealBuildTool;
using System.Collections.Generic;

public class HandGameplayTarget : TargetRules
{
	public HandGameplayTarget( TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V4;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_3;
		ExtraModuleNames.AddRange( new string[] { "HandGameplay" } );
	}
}
