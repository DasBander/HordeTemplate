

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
#define GAME_VERSION 2.7

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