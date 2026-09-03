// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class FMCodex : ModuleRules
{
	public FMCodex(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore" });

		PrivateDependencyModuleNames.AddRange(new string[] { "Json", "Slate", "SlateCore", "UMG" });
		if (Target.bBuildEditor)
		{
			PrivateDependencyModuleNames.Add("UnrealEd");
			// Non-PIE, offscreen widget rendering in development visual audits.
			PrivateDependencyModuleNames.Add("RenderCore");
		}
		
		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
