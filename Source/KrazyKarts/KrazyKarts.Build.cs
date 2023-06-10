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
        PublicDefinitions.Add("WG_TEXT(x)=if(GEngine){GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, TEXT(x));}");
        PublicDefinitions.Add("WG_INT(x)=if(GEngine){GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, FString::SanitizeInt(x));}");
        PublicDefinitions.Add("WG_FLOAT(x)=if(GEngine){GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, FString::SanitizeFloat(x));}");
        PublicDefinitions.Add("WG_VECTOR(x)=if(GEngine){GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, x.ToString());}");
    }
}
