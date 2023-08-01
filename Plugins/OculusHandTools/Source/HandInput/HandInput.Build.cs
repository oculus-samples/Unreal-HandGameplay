// Copyright (c) Meta Platforms, Inc. and affiliates.

using UnrealBuildTool;

public class HandInput : ModuleRules
{
	public HandInput(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		//CppStandard = CppStandardVersion.Latest;

		PublicDependencyModuleNames.AddRange(
			new string[] {
				"Core",
				"CoreUObject",
				"Engine",
				"InputCore",
				"OculusXRInput",
				"HeadMountedDisplay"
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[] {
				"OculusHandPoseRecognition",
				"OculusUtils"
			}
		);

		PublicIncludePaths.AddRange(new string[] {
			// TODO(els): Relative to UE4\Source... not sure why this fixes the build since we do depend on Core.
			// "Runtime/Core/Public/Containers",
			// "Runtime/CoreUObject/Public/UObject",
		});

		PrivateIncludePaths.AddRange(new string[] {
		});
	}
}
