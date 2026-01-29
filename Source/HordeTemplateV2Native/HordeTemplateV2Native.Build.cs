

using UnrealBuildTool;

public class HordeTemplateV2Native : ModuleRules
{
	public HordeTemplateV2Native(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] {
            "Core",
            "CoreUObject",
            "Engine",
            "InputCore",
            "PhysicsCore",
            "Landscape",
            "AIModule",
            "OnlineSubsystem",
            "OnlineSubsystemUtils",
            "UMG",
            "Slate",
            "SlateCore",
            "LevelSequence",
            "MovieScene",
            "RHI",
            "ApexDestruction",
            "AIModule",
            "GameplayTasks",
            "NavigationSystem",
            "MoviePlayer"
        });

		PrivateDependencyModuleNames.AddRange(new string[] {
            "CoreUObject",
            "Engine",
            "Slate",
            "SlateCore",
            "Core",
            "InputCore",
            "Json",
            "JsonUtilities",
            "OnlineSubsystem"
        });

        DynamicallyLoadedModuleNames.AddRange(
          new string[] {
                "OnlineSubsystemNull",
                "NetworkReplayStreaming",
                "NullNetworkReplayStreaming",
                "HttpNetworkReplayStreaming",
                "OnlineSubsystemSteam"
           }
         );

        PrivateIncludePathModuleNames.AddRange(
            new string[] {
                "NetworkReplayStreaming"
            }
        );

    }
}
