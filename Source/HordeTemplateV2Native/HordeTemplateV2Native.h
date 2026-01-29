// Copyright (c) 2018 - 2026 Marc Fraedrich
// Horde Template V2 (Native) - A cooperative zombie survival game framework
// Licensed under the MIT License

#pragma once

/**
 * @file HordeTemplateV2Native.h
 * @brief Core module header with project-wide definitions, constants, and utility functions
 * @author Marc Fraedrich
 *
 * This file contains all the central configuration values for the Horde Template framework:
 * - Game version and economy settings
 * - Zombie AI behavior parameters
 * - Dynamic spawner configuration
 * - Asset paths for DataTables, Widgets, and Sounds
 * - Physics material surface type definitions
 * - Utility template functions for dynamic object loading
 *
 * @note On Widget Blueprints or Blueprints don't forget the _C suffix to get the Compiled Asset Reference.
 */

#include "Net/UnrealNetwork.h"
#include "CoreMinimal.h"
// =============================================================================
// GENERAL SETTINGS
// =============================================================================

/** Current game version number */
#define GAME_VERSION 2.8

// =============================================================================
// ECONOMY SETTINGS
// =============================================================================
#define STARTING_MONEY 1500
#define ECONOMY_DATATABLE_PATH TEXT("DataTable'/Game/HordeTemplateBP/Data/TraderItems.TraderItems'")
#define CURRENCY_PREFIX "$"

// =============================================================================
// INVENTORY SETTINGS
// =============================================================================

/** Path to the inventory items DataTable */
#define INVENTORY_DATATABLE_PATH TEXT("DataTable'/Game/HordeTemplateBP/Data/HordeInventoryItems.HordeInventoryItems'")

// =============================================================================
// TRADER SETTINGS
// =============================================================================
#define TRADER_BUY_SOUND TEXT("SoundCue'/Game/HordeTemplateBP/Assets/Sounds/A_TraderBuy_Cue.A_TraderBuy_Cue'")

// =============================================================================
// ZOMBIE AI - CORE SETTINGS
// =============================================================================

/** Path to the zombie behavior tree asset */
#define ZED_BEHAVIORTREE_ASSET_PATH TEXT("BehaviorTree'/Game/HordeTemplateBP/Blueprint/Ai/BT/BT_Zed.BT_Zed'")

/** Minimum time before zombie loses sight of target (seconds) */
#define ZED_LOSE_SIGHT_TIME_MIN 6.f

/** Maximum time before zombie loses sight of target (seconds) */
#define ZED_LOSE_SIGHT_TIME_MAX 12.f

/** Distance at which zombie loses sight of target (units) */
#define ZED_LOSE_SIGHT_RADIUS 2500.f

/** Distance at which zombie can see targets (units) */
#define ZED_SIGHT_RADIUS 1000.f

// =============================================================================
// ZOMBIE AI - MOVEMENT SETTINGS
// =============================================================================
#define ZED_SPEED_IDLE 0.f
#define ZED_SPEED_PATROL 200.f
#define ZED_SPEED_ALERT 300.f
#define ZED_SPEED_CHASE 500.f
#define ZED_SPEED_ACCELERATION 200.f		// Units per second acceleration
#define ZED_SPEED_DECELERATION 400.f		// Units per second deceleration (faster stop)
#define ZED_SPEED_MULTIPLIER_MIN 0.85f		// Minimum speed variation
#define ZED_SPEED_MULTIPLIER_MAX 1.15f		// Maximum speed variation

// =============================================================================
// ZOMBIE AI - INVESTIGATION SETTINGS
// =============================================================================

/** Minimum investigation duration at last known position (seconds) */
#define ZED_INVESTIGATE_DURATION_MIN 2.f

/** Maximum investigation duration at last known position (seconds) */
#define ZED_INVESTIGATE_DURATION_MAX 3.f

/** Interval for line-of-sight validation during chase (seconds) */
#define ZED_LOS_CHECK_INTERVAL 0.5f

/** Time without LOS before transitioning to investigation (seconds) */
#define ZED_LOS_LOST_THRESHOLD 2.f

// =============================================================================
// ZOMBIE AI - COMBAT SETTINGS
// =============================================================================
#define ZED_ATTACK_COOLDOWN_MIN 1.5f
#define ZED_ATTACK_COOLDOWN_MAX 2.5f
#define ZED_ATTACK_WINDUP_MIN 0.3f
#define ZED_ATTACK_WINDUP_MAX 0.5f
#define ZED_ATTACK_DAMAGE_MIN 3.f
#define ZED_ATTACK_DAMAGE_MAX 12.f

// =============================================================================
// ZOMBIE AI - HIT REACTION SETTINGS
// =============================================================================

/** Probability of stagger when taking damage (0.0-1.0) */
#define ZED_STAGGER_CHANCE 0.3f

/** Speed multiplier during stagger state */
#define ZED_STAGGER_SPEED_MULTIPLIER 0.5f

/** Duration of stagger effect (seconds) */
#define ZED_STAGGER_DURATION 0.5f

/** Speed multiplier during anger burst */
#define ZED_ANGER_SPEED_MULTIPLIER 1.2f

/** Duration of anger burst effect (seconds) */
#define ZED_ANGER_DURATION 1.f

// =============================================================================
// DYNAMIC ZOMBIE SPAWNER - DISTANCE SETTINGS
// =============================================================================
#define ZED_SPAWNER_MIN_DISTANCE 500.f		// Minimum distance from player to spawn
#define ZED_SPAWNER_MAX_DISTANCE 3000.f		// Maximum distance from player to spawn

// =============================================================================
// DYNAMIC ZOMBIE SPAWNER - SPAWN RATE SETTINGS
// =============================================================================

/** Base interval between spawn attempts (seconds) */
#define ZED_SPAWNER_INTERVAL 3.f

/** Random deviation applied to spawn interval (seconds) */
#define ZED_SPAWNER_INTERVAL_DEVIATION 1.f

/** Maximum concurrent zombies from a single spawner */
#define ZED_SPAWNER_MAX_CONCURRENT 5

/** Global zombie limit across all spawners */
#define ZED_SPAWNER_GLOBAL_LIMIT 30

// =============================================================================
// DYNAMIC ZOMBIE SPAWNER - EXHAUSTION SETTINGS
// =============================================================================
#define ZED_SPAWNER_MAX_KILLS 10			// Kills before exhaustion
#define ZED_SPAWNER_REACTIVATION_DELAY 60.f	// Seconds before reactivation

// Dynamic Zombie Spawner - Vision Check
#define ZED_SPAWNER_VISION_ANGLE 60.f		// Degrees from center of view
#define ZED_SPAWNER_VISION_BUFFER 200.f		// Buffer distance for vision check

// =============================================================================
// LOBBY SYSTEM SETTINGS
// =============================================================================

/** Path to the playable levels DataTable */
#define MAPS_DATATABLE_PATH TEXT("DataTable'/Game/HordeTemplateBP/Data/PlayableLevels.PlayableLevels'")

/** Path to the playable characters DataTable */
#define CHARACTER_DATATABLE_PATH TEXT("DataTable'/Game/HordeTemplateBP/Data/PlayableCharacters.PlayableCharacters'")

// =============================================================================
// TRACE CHANNEL DEFINITIONS
// =============================================================================

/** Trace channel for item drop ground detection */
#define DROP_TRACE_CHANNEL ECC_GameTraceChannel1

/** Trace channel for player name/health display visibility */
#define PLAYER_HEAD_DISPLAY_CHANNEL ECC_GameTraceChannel2

/** Trace channel for interaction system raycasts */
#define INTERACTION_TRACE_CHANNEL ECC_GameTraceChannel3

// =============================================================================
// DEFAULT MAP SETTINGS
// =============================================================================

/** Default items given to players at game start (DataTable row names) */
#define DEFAULT_STARTING_ITEMS {"Item_Hands"}

/** Default available player characters for selection (DataTable row names) */
#define DEFAULT_AVAILABLE_PLAYERCHARACTERS { "Char_Matt", "Char_Louis", "Char_Jacob", "Char_Joshua" }

// =============================================================================
// WIDGET & HUD ASSET PATHS
// =============================================================================

/** Trader/shop UI widget blueprint path */
#define WIDGET_TRADER_UI_PATH TEXT("WidgetBlueprint'/Game/HordeTemplateBP/Blueprint/Widgets/Trader/WBP_Trader_Main.WBP_Trader_Main_C'")

/** Main game HUD widget blueprint path */
#define WIDGET_HUD_MAIN_UI_PATH TEXT("WidgetBlueprint'/Game/HordeTemplateBP/Blueprint/Widgets/WBP_HUDCpp.WBP_HUDCpp_C'")

/** Lobby/character selection widget blueprint path */
#define WIDGET_LOBBY_UI_PATH TEXT("WidgetBlueprint'/Game/HordeTemplateBP/Blueprint/Widgets/Lobby/WBP_Lobby_Main.WBP_Lobby_Main_C'")

/** End game/results screen widget blueprint path */
#define WIDGET_ENDSCREEN_UI_PATH TEXT("WidgetBlueprint'/Game/HordeTemplateBP/Blueprint/Widgets/Game/WBP_EndScreen.WBP_EndScreen_C'")

/** In-game scoreboard widget blueprint path */
#define WIDGET_SCOREBOARD_PATH TEXT("WidgetBlueprint'/Game/HordeTemplateBP/Blueprint/Widgets/Game/WBP_Scoreboard.WBP_Scoreboard_C'")

/** Server travel/loading screen widget blueprint path */
#define WIDGET_SERVERTRAVEL_UI_PATH TEXT("WidgetBlueprint'/Game/HordeTemplateBP/Blueprint/Widgets/Game/WBP_ServerTravel.WBP_ServerTravel_C'")

/** Escape/pause menu widget blueprint path */
#define WIDGET_ESCAPEMENU_UI_PATH TEXT("WidgetBlueprint'/Game/HordeTemplateBP/Blueprint/Widgets/Game/WBP_LeaveGame.WBP_LeaveGame_C'")

/** Crosshair texture asset path */
#define CROSSHAIR_TEXTURE_PATH TEXT("Texture2D'/Game/HordeTemplateBP/Assets/Textures/Hud/center_dot.center_dot'")

// =============================================================================
// PHYSICS MATERIAL SURFACE TYPES
// =============================================================================

/** Surface type for concrete/hard surfaces (impacts, footsteps) */
#define SURFACE_CONCRETE SurfaceType1

/** Surface type for flesh/organic materials (blood, gore effects) */
#define SURFACE_FLESH SurfaceType2

// =============================================================================
// UTILITY TEMPLATE FUNCTIONS
// =============================================================================

/**
 * Dynamically loads an object from an asset path
 * @tparam ObjType The type of object to load (e.g., UTexture2D, USoundCue)
 * @param Path The asset path as FName (e.g., "Texture2D'/Game/...'")
 * @return Pointer to the loaded object, or nullptr if not found
 */
template <typename ObjType>
static FORCEINLINE ObjType* ObjectFromPath(const FName& Path)
{
	if (Path == NAME_None) return nullptr;
	return Cast<ObjType>(StaticLoadObject(ObjType::StaticClass(), nullptr, *Path.ToString()));
}

/**
 * Retrieves a row from a DataTable by path and row name
 * @tparam DatatableType The struct type of the DataTable rows
 * @param DatatablePath The asset path to the DataTable
 * @param DatatableKey The row name to look up
 * @return Copy of the row data, or default-constructed struct if not found
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