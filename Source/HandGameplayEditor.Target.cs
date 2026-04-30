// Copyright (c) Meta Platforms, Inc. and affiliates.

using UnrealBuildTool;
using System.Collections.Generic;

public class HandGameplayEditorTarget : TargetRules
{
	public HandGameplayEditorTarget( TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V6;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_7;
		ExtraModuleNames.AddRange( new string[] { "HandGameplay" } );
	}
}
