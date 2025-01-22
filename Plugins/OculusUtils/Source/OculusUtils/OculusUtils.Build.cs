// Copyright (c) Facebook, Inc. and its affiliates.

using System.IO;
using UnrealBuildTool;

public class OculusUtils : ModuleRules
{
	public OculusUtils(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicIncludePaths.AddRange(
			new string[]
			{
				// ... add public include paths required here ...
			}
		);


		PrivateIncludePaths.Add(Path.Combine(GetModuleDirectory("OculusXRHMD"), "Private"));


		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				// ... add other public dependencies that you statically link with here ...
			}
		);


		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				"OculusXRHMD",
				"DeveloperSettings",
				"OVRPluginXR"
			}
		);

		if (Target.bBuildEditor)
		{
			PrivateDependencyModuleNames.Add("UnrealEd");
			PrivateDependencyModuleNames.Add("DetailCustomizations");
			PrivateDependencyModuleNames.Add("ToolWidgets");
		}

		bLegacyParentIncludePaths = true;
	}
}