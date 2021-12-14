// Copyright (c) Meta Platforms, Inc. and affiliates.

using UnrealBuildTool;
using System.Collections.Generic;

public class HandGameplayEditorTarget : TargetRules
{
	public HandGameplayEditorTarget( TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V2;
		ExtraModuleNames.AddRange( new string[] { "HandGameplay" } );
	}
}
