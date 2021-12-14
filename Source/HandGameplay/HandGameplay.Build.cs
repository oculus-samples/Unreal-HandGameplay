// Copyright (c) Meta Platforms, Inc. and affiliates.

using UnrealBuildTool;

public class HandGameplay : ModuleRules
{
	public HandGameplay(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "OculusHandPoseRecognition" });

		PrivateDependencyModuleNames.AddRange(new string[] { "OculusUtils" });

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true

		if (Target.Platform == UnrealTargetPlatform.Android)
		{
			var manifestFile = System.IO.Path.Combine(ModuleDirectory, "UpdatePermissions.xml");
			AdditionalPropertiesForReceipt.Add("AndroidPlugin", manifestFile);
		}
	}
}