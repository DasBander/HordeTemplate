

#pragma once
#include "Net/UnrealNetwork.h"
#include "CoreMinimal.h"

/*
	Default Project Definitions
	Documentation found on https://bugs.finalspark.io/docs/horde-template-native
*/

/*
	Notes:
		- On Widget Blueprints or Blueprints don't forget the _C to get the Compiled Asset Reference. 
*/
//General
#define GAME_VERSION 2.8

//Economy
#define STARTING_MONEY 1500
#define ECONOMY_DATATABLE_PATH TEXT("DataTable'/Game/HordeTemplateBP/Data/TraderItems.TraderItems'")
#define CURRENCY_PREFIX "$"

//Inventory
#define INVENTORY_DATATABLE_PATH TEXT("DataTable'/Game/HordeTemplateBP/Data/HordeInventoryItems.HordeInventoryItems'")

//Trader
#define TRADER_BUY_SOUND TEXT("SoundCue'/Game/HordeTemplateBP/Assets/Sounds/A_TraderBuy_Cue.A_TraderBuy_Cue'")

//Zombie AI
#define ZED_BEHAVIORTREE_ASSET_PATH TEXT("BehaviorTree'/Game/HordeTemplateBP/Blueprint/Ai/BT/BT_Zed.BT_Zed'")
#define ZED_LOSE_SIGHT_TIME_MIN 6.f
#define ZED_LOSE_SIGHT_TIME_MAX 12.f
#define ZED_LOSE_SIGHT_RADIUS 2500.f
#define ZED_SIGHT_RADIUS 1000.f

// Zombie AI - Speed Settings
#define ZED_SPEED_IDLE 0.f
#define ZED_SPEED_PATROL 200.f
#define ZED_SPEED_ALERT 300.f
#define ZED_SPEED_CHASE 500.f
#define ZED_SPEED_ACCELERATION 200.f		// Units per second acceleration
#define ZED_SPEED_DECELERATION 400.f		// Units per second deceleration (faster stop)
#define ZED_SPEED_MULTIPLIER_MIN 0.85f		// Minimum speed variation
#define ZED_SPEED_MULTIPLIER_MAX 1.15f		// Maximum speed variation

// Zombie AI - Investigation
#define ZED_INVESTIGATE_DURATION_MIN 2.f
#define ZED_INVESTIGATE_DURATION_MAX 3.f
#define ZED_LOS_CHECK_INTERVAL 0.5f			// How often to validate line of sight
#define ZED_LOS_LOST_THRESHOLD 2.f			// Seconds without LOS before investigating

// Zombie AI - Combat
#define ZED_ATTACK_COOLDOWN_MIN 1.5f
#define ZED_ATTACK_COOLDOWN_MAX 2.5f
#define ZED_ATTACK_WINDUP_MIN 0.3f
#define ZED_ATTACK_WINDUP_MAX 0.5f
#define ZED_ATTACK_DAMAGE_MIN 3.f
#define ZED_ATTACK_DAMAGE_MAX 12.f

// Zombie AI - Hit Reactions
#define ZED_STAGGER_CHANCE 0.3f				// 30% chance to stagger
#define ZED_STAGGER_SPEED_MULTIPLIER 0.5f	// 50% speed during stagger
#define ZED_STAGGER_DURATION 0.5f
#define ZED_ANGER_SPEED_MULTIPLIER 1.2f		// 20% faster during anger
#define ZED_ANGER_DURATION 1.f

// Dynamic Zombie Spawner - Distance Settings
#define ZED_SPAWNER_MIN_DISTANCE 500.f		// Minimum distance from player to spawn
#define ZED_SPAWNER_MAX_DISTANCE 3000.f		// Maximum distance from player to spawn

// Dynamic Zombie Spawner - Spawn Rate
#define ZED_SPAWNER_INTERVAL 3.f			// Seconds between spawn attempts
#define ZED_SPAWNER_INTERVAL_DEVIATION 1.f	// Random deviation for spawn interval
#define ZED_SPAWNER_MAX_CONCURRENT 5		// Max zombies per spawner at once
#define ZED_SPAWNER_GLOBAL_LIMIT 30			// Global zombie limit for all spawners

// Dynamic Zombie Spawner - Exhaustion
#define ZED_SPAWNER_MAX_KILLS 10			// Kills before exhaustion
#define ZED_SPAWNER_REACTIVATION_DELAY 60.f	// Seconds before reactivation

// Dynamic Zombie Spawner - Vision Check
#define ZED_SPAWNER_VISION_ANGLE 60.f		// Degrees from center of view
#define ZED_SPAWNER_VISION_BUFFER 200.f		// Buffer distance for vision check

//Lobby
#define MAPS_DATATABLE_PATH TEXT("DataTable'/Game/HordeTemplateBP/Data/PlayableLevels.PlayableLevels'")
#define CHARACTER_DATATABLE_PATH TEXT("DataTable'/Game/HordeTemplateBP/Data/PlayableCharacters.PlayableCharacters'")

//Tracing
#define DROP_TRACE_CHANNEL ECC_GameTraceChannel1
#define PLAYER_HEAD_DISPLAY_CHANNEL ECC_GameTraceChannel2
#define INTERACTION_TRACE_CHANNEL ECC_GameTraceChannel3

//Default Map Settings
#define DEFAULT_STARTING_ITEMS {"Item_Hands"}
#define DEFAULT_AVAILABLE_PLAYERCHARACTERS { "Char_Matt", "Char_Louis", "Char_Jacob", "Char_Joshua" }

//Widgets & HUD 
#define WIDGET_TRADER_UI_PATH TEXT("WidgetBlueprint'/Game/HordeTemplateBP/Blueprint/Widgets/Trader/WBP_Trader_Main.WBP_Trader_Main_C'")
#define WIDGET_HUD_MAIN_UI_PATH TEXT("WidgetBlueprint'/Game/HordeTemplateBP/Blueprint/Widgets/WBP_HUDCpp.WBP_HUDCpp_C'")
#define WIDGET_LOBBY_UI_PATH TEXT("WidgetBlueprint'/Game/HordeTemplateBP/Blueprint/Widgets/Lobby/WBP_Lobby_Main.WBP_Lobby_Main_C'")
#define WIDGET_ENDSCREEN_UI_PATH TEXT("WidgetBlueprint'/Game/HordeTemplateBP/Blueprint/Widgets/Game/WBP_EndScreen.WBP_EndScreen_C'")
#define WIDGET_SCOREBOARD_PATH TEXT("WidgetBlueprint'/Game/HordeTemplateBP/Blueprint/Widgets/Game/WBP_Scoreboard.WBP_Scoreboard_C'")
#define WIDGET_SERVERTRAVEL_UI_PATH TEXT("WidgetBlueprint'/Game/HordeTemplateBP/Blueprint/Widgets/Game/WBP_ServerTravel.WBP_ServerTravel_C'")
#define WIDGET_ESCAPEMENU_UI_PATH TEXT("WidgetBlueprint'/Game/HordeTemplateBP/Blueprint/Widgets/Game/WBP_LeaveGame.WBP_LeaveGame_C'")
#define CROSSHAIR_TEXTURE_PATH TEXT("Texture2D'/Game/HordeTemplateBP/Assets/Textures/Hud/center_dot.center_dot'")

//Physics Materials
#define SURFACE_CONCRETE SurfaceType1
#define SURFACE_FLESH SurfaceType2

/*
	Loads object dynamically from a given path.
*/
template <typename ObjType>
static FORCEINLINE ObjType* ObjectFromPath(const FName& Path)
{
	if (Path == NAME_None) return nullptr;
	return Cast<ObjType>(StaticLoadObject(ObjType::StaticClass(), nullptr, *Path.ToString()));
}
/*
	Get Datatable Row by Path and Row Name
*/
template <typename DatatableType>
static FORCEINLINE DatatableType FromDatatable(const FString& DatatablePath, const FName& DatatableKey)
{
	if (DatatablePath == "" || DatatableKey == NAME_None) return DatatableType();
	UDataTable* Data = Cast<UDataTable>(StaticLoadObject(UDataTable::StaticClass(), nullptr, *DatatablePath));
	if (Data) {
		DatatableType * DTD = Data->FindRow<DatatableType>(DatatableKey, "FromDatatable Failed to Find", true);
		if (DTD)
		{
			return *DTD;
		}
		else {
			return DatatableType();
		}
	}
	else {
		return DatatableType();
	}
}