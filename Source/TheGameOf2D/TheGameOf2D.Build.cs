// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class TheGameOf2D : ModuleRules
{
	public TheGameOf2D(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"AIModule",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"Niagara",
			"LevelSequence",
			"MovieScene",
			"ImageWrapper",
			"UMG",
			"Slate",
			"SlateCore"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"TheGameOf2D",
			"TheGameOf2D/Variant_SideScrolling",
			"TheGameOf2D/Variant_SideScrolling/Animation",
			"TheGameOf2D/Variant_SideScrolling/AI",
			"TheGameOf2D/Variant_SideScrolling/Gameplay",
			"TheGameOf2D/Variant_SideScrolling/Interfaces",
			"TheGameOf2D/Variant_SideScrolling/UI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
