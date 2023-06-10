// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class KrazyKarts : ModuleRules
{
	public KrazyKarts(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "HeadMountedDisplay", "EnhancedInput" });

        // Woogle's debug macro
        PublicDefinitions.Add("WG_LOG(x)=UE_LOG(LogTemp, Warning, TEXT(x));");
        PublicDefinitions.Add("WG_SDM(x)=if(GEngine){GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, TEXT(x));}");
    }
}
