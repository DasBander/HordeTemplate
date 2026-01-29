// Copyright (c) 2018 - 2026 Marc Fraedrich
// Horde Template V2 (Native) - A cooperative zombie survival game framework
// Licensed under the MIT License

#pragma once

/**
 * @file HordeWorldSettings.h
 * @brief Per-level world settings for player starting items, match mode, round configuration, and quest system
 * @author Marc Fraedrich
 */

#include "CoreMinimal.h"
#include "GameFramework/WorldSettings.h"
#include "Gameplay/HordeGameMode.h"
#include "HordeTemplateV2Native.h"
#include "Engine/DataTable.h"
#include "HordeWorldSettings.generated.h"

/**
 * World settings for Horde game levels
 * Configure player starting items, match mode, round settings, and quest configuration per level
 */
UCLASS()
class HORDETEMPLATEV2NATIVE_API AHordeWorldSettings : public AWorldSettings
{
	GENERATED_BODY()


public:
	// ==================== PLAYER SETTINGS ====================

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Horde Game|Player Starting")
		TArray<FName> StartingItems = DEFAULT_STARTING_ITEMS;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Horde Game|Lobby")
		TArray<FName> PlayerCharacters = DEFAULT_AVAILABLE_PLAYERCHARACTERS;

	// ==================== GAMEPLAY SETTINGS ====================

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Horde Game|Gameplay")
		EMatchMode MatchMode;

	// ==================== ROUND BASED SETTINGS ====================

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Horde Game|Round Based")
		int32 MaxRounds = 13;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Horde Game|Round Based")
		int32 PauseTime = 30;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Horde Game|Round Based")
		int32 RoundTime = 300;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Horde Game|Round Based")
		int32 ZedMultiplier = 2;

	// ==================== QUEST SYSTEM SETTINGS ====================

	/** Data table containing all quest definitions for this level (uses FQuestData row struct) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Horde Game|Quest System")
		UDataTable* QuestDataTable;

	/** Quests that automatically start when the level begins */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Horde Game|Quest System")
		TArray<FName> AutoStartQuests;

	/** If true, quest progress is shown on the HUD */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Horde Game|Quest System")
		bool bShowQuestHUD = true;

	/** If true, notifications are shown when quest state changes */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Horde Game|Quest System")
		bool bShowQuestNotifications = true;

	/** Delay before auto-starting quests after BeginPlay (allows level to initialize) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Horde Game|Quest System", meta = (ClampMin = "0.0"))
		float QuestAutoStartDelay = 1.0f;

	/** If true, failed quests can be restarted via QuestManager */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Horde Game|Quest System")
		bool bAllowQuestRestart = false;

	/** Maximum number of active quests at once (0 = unlimited) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Horde Game|Quest System", meta = (ClampMin = "0"))
		int32 MaxActiveQuests = 0;

};
