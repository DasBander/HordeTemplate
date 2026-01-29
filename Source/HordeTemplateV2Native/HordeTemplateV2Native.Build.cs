

using UnrealBuildTool;
using System.IO;

public class HordeTemplateV2Native : ModuleRules
{
	public HordeTemplateV2Native(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		// Add legacy include paths for backward compatibility
		string ModulePath = ModuleDirectory;
		PublicIncludePaths.AddRange(new string[] {
			ModulePath,
			Path.Combine(ModulePath, "Public"),
			Path.Combine(ModulePath, "Public/AI"),
			Path.Combine(ModulePath, "Public/AI/Services"),
			Path.Combine(ModulePath, "Public/AI/Tasks"),
			Path.Combine(ModulePath, "Public/Animation"),
			Path.Combine(ModulePath, "Public/Character"),
			Path.Combine(ModulePath, "Public/FX"),
			Path.Combine(ModulePath, "Public/FX/Camera"),
			Path.Combine(ModulePath, "Public/FX/Impact"),
			Path.Combine(ModulePath, "Public/Gameplay"),
			Path.Combine(ModulePath, "Public/HUD"),
			Path.Combine(ModulePath, "Public/HUD/Widgets"),
			Path.Combine(ModulePath, "Public/HUD/Widgets/Chat"),
			Path.Combine(ModulePath, "Public/Inventory"),
			Path.Combine(ModulePath, "Public/Misc"),
			Path.Combine(ModulePath, "Public/Projectiles"),
			Path.Combine(ModulePath, "Public/Quest"),
			Path.Combine(ModulePath, "Public/Weapons")
		});

		PrivateIncludePaths.AddRange(new string[] {
			Path.Combine(ModulePath, "Private"),
			Path.Combine(ModulePath, "Private/AI"),
			Path.Combine(ModulePath, "Private/AI/Services"),
			Path.Combine(ModulePath, "Private/AI/Tasks"),
			Path.Combine(ModulePath, "Private/Animation"),
			Path.Combine(ModulePath, "Private/Character"),
			Path.Combine(ModulePath, "Private/FX"),
			Path.Combine(ModulePath, "Private/FX/Camera"),
			Path.Combine(ModulePath, "Private/FX/Impact"),
			Path.Combine(ModulePath, "Private/Gameplay"),
			Path.Combine(ModulePath, "Private/HUD"),
			Path.Combine(ModulePath, "Private/HUD/Widgets"),
			Path.Combine(ModulePath, "Private/HUD/Widgets/Chat"),
			Path.Combine(ModulePath, "Private/Inventory"),
			Path.Combine(ModulePath, "Private/Misc"),
			Path.Combine(ModulePath, "Private/Projectiles"),
			Path.Combine(ModulePath, "Private/Quest"),
			Path.Combine(ModulePath, "Private/Weapons")
		});

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
