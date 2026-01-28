

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "QuestTypes.generated.h"

class AZedPawn;
class ALevelSequenceActor;

/**
 * Type of quest objective
 */
UENUM(BlueprintType)
enum class EQuestObjectiveType : uint8
{
	Location,			// Reach a specific location/area
	CollectItem,		// Collect specific quest items
	KillEnemies,		// Kill a number of enemies
	KillSpecificType,	// Kill specific enemy type
	Interact,			// Interact with an object
	Survive,			// Survive for a duration
	Escort,				// Keep an NPC/object alive
	TriggerSequence,	// Trigger a level sequence
	Custom				// Blueprint-defined objective
};

/**
 * Current state of a quest
 */
UENUM(BlueprintType)
enum class EQuestState : uint8
{
	Inactive,			// Quest not yet started
	Active,				// Quest is in progress
	Completed,			// Quest successfully completed
	Failed				// Quest failed
};

/**
 * Current state of an objective
 */
UENUM(BlueprintType)
enum class EObjectiveState : uint8
{
	Locked,				// Not yet available (previous objective not complete)
	Active,				// Currently tracking
	Completed,			// Successfully completed
	Failed				// Failed (optional objectives can fail without failing quest)
};

/**
 * Quest completion behavior
 */
UENUM(BlueprintType)
enum class EQuestCompletionMode : uint8
{
	AllObjectives,		// All objectives must be completed
	AnyObjective,		// Any single objective completes the quest
	SequentialObjectives // Objectives must be completed in order
};

/**
 * Defines a single quest objective
 */
USTRUCT(BlueprintType)
struct HORDETEMPLATEV2NATIVE_API FQuestObjective
{
	GENERATED_BODY()

	/** Unique identifier for this objective */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
		FName ObjectiveID;

	/** Display name shown to players */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
		FText DisplayName;

	/** Description of what to do */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
		FText Description;

	/** Type of objective */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
		EQuestObjectiveType ObjectiveType = EQuestObjectiveType::Location;

	/** Is this objective optional? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
		bool bIsOptional = false;

	/** Should this objective be hidden from the player until active? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
		bool bHiddenUntilActive = false;

	// ========== TYPE-SPECIFIC PARAMETERS ==========

	/** Target count (items to collect, enemies to kill, etc.) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective|Parameters")
		int32 TargetCount = 1;

	/** Target actor tag for location/interact objectives */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective|Parameters")
		FName TargetActorTag;

	/** Target item ID for collect objectives */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective|Parameters")
		FName TargetItemID;

	/** Target enemy class for kill specific type objectives */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective|Parameters")
		TSubclassOf<AZedPawn> TargetEnemyClass;

	/** Duration for survive objectives (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective|Parameters")
		float SurviveDuration = 60.f;

	/** Level sequence to play for sequence objectives */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective|Parameters")
		TSoftObjectPtr<ALevelSequenceActor> SequenceToTrigger;

	/** Radius for location objectives */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective|Parameters")
		float LocationRadius = 200.f;

	// ========== REWARDS & TRIGGERS ==========

	/** Points awarded on completion */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective|Rewards")
		int32 PointsReward = 0;

	/** Money awarded on completion */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective|Rewards")
		int32 MoneyReward = 0;

	/** Tag to trigger when objective completes (for world events) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective|Triggers")
		FName CompletionTriggerTag;

	// ========== RUNTIME STATE (Not saved in DataTable) ==========

	/** Current progress count */
	UPROPERTY(BlueprintReadOnly, Category = "Objective|Runtime")
		int32 CurrentCount = 0;

	/** Current state */
	UPROPERTY(BlueprintReadOnly, Category = "Objective|Runtime")
		EObjectiveState State = EObjectiveState::Locked;

	/** Time remaining for survive objectives */
	UPROPERTY(BlueprintReadOnly, Category = "Objective|Runtime")
		float TimeRemaining = 0.f;

	FQuestObjective()
	{
		ObjectiveID = NAME_None;
		DisplayName = FText::GetEmpty();
		Description = FText::GetEmpty();
	}
};

/**
 * Defines a complete quest
 */
USTRUCT(BlueprintType)
struct HORDETEMPLATEV2NATIVE_API FQuestData : public FTableRowBase
{
	GENERATED_BODY()

	/** Unique identifier for this quest */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
		FName QuestID;

	/** Display name shown to players */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
		FText QuestName;

	/** Full description of the quest */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
		FText QuestDescription;

	/** Short description for HUD display */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
		FText ShortDescription;

	/** Quest objectives */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
		TArray<FQuestObjective> Objectives;

	/** How objectives are evaluated for completion */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
		EQuestCompletionMode CompletionMode = EQuestCompletionMode::AllObjectives;

	/** Quest that follows this one (chain quest) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Chain")
		FName FollowUpQuestID;

	/** If true, follow-up quest starts automatically */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Chain")
		bool bAutoStartFollowUp = true;

	/** Required quest that must be completed first */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Prerequisites")
		FName PrerequisiteQuestID;

	// ========== TRIGGERS ==========

	/** If true, quest starts automatically when level begins */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Triggers")
		bool bStartOnLevelBegin = false;

	/** Delay before auto-starting (if bStartOnLevelBegin) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Triggers", meta = (EditCondition = "bStartOnLevelBegin"))
		float StartDelay = 0.f;

	/** Tag that can trigger this quest externally */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Triggers")
		FName TriggerTag;

	// ========== REWARDS ==========

	/** Points awarded on quest completion */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Rewards")
		int32 CompletionPoints = 500;

	/** Money awarded on quest completion */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Rewards")
		int32 CompletionMoney = 0;

	/** Items awarded on completion (item IDs) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Rewards")
		TArray<FName> RewardItems;

	// ========== FAILURE CONDITIONS ==========

	/** If true, quest can fail */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Failure")
		bool bCanFail = false;

	/** If true, quest fails if all players die */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Failure", meta = (EditCondition = "bCanFail"))
		bool bFailOnAllPlayersDead = true;

	/** Time limit in seconds (0 = no limit) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Failure", meta = (EditCondition = "bCanFail"))
		float TimeLimit = 0.f;

	// ========== VISUAL/AUDIO ==========

	/** Icon to display in HUD */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Visual")
		TSoftObjectPtr<UTexture2D> QuestIcon;

	/** Sound to play when quest starts */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Audio")
		TSoftObjectPtr<USoundCue> StartSound;

	/** Sound to play when quest completes */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Audio")
		TSoftObjectPtr<USoundCue> CompleteSound;

	/** Sound to play when quest fails */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Audio")
		TSoftObjectPtr<USoundCue> FailSound;

	// ========== RUNTIME STATE ==========

	/** Current quest state */
	UPROPERTY(BlueprintReadOnly, Category = "Quest|Runtime")
		EQuestState State = EQuestState::Inactive;

	/** Time remaining (if time limit set) */
	UPROPERTY(BlueprintReadOnly, Category = "Quest|Runtime")
		float TimeRemaining = 0.f;

	FQuestData()
	{
		QuestID = NAME_None;
		QuestName = FText::GetEmpty();
		QuestDescription = FText::GetEmpty();
		ShortDescription = FText::GetEmpty();
	}

	/** Get the current active objective (for sequential mode) */
	FQuestObjective* GetCurrentObjective()
	{
		for (FQuestObjective& Obj : Objectives)
		{
			if (Obj.State == EObjectiveState::Active)
			{
				return &Obj;
			}
		}
		return nullptr;
	}

	/** Check if all required objectives are complete */
	bool AreAllRequiredObjectivesComplete() const
	{
		for (const FQuestObjective& Obj : Objectives)
		{
			if (!Obj.bIsOptional && Obj.State != EObjectiveState::Completed)
			{
				return false;
			}
		}
		return true;
	}

	/** Check if any objective is complete (for AnyObjective mode) */
	bool IsAnyObjectiveComplete() const
	{
		for (const FQuestObjective& Obj : Objectives)
		{
			if (!Obj.bIsOptional && Obj.State == EObjectiveState::Completed)
			{
				return true;
			}
		}
		return false;
	}
};

/**
 * Quest progress snapshot for replication
 */
USTRUCT(BlueprintType)
struct HORDETEMPLATEV2NATIVE_API FQuestProgress
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
		FName QuestID;

	UPROPERTY(BlueprintReadOnly)
		EQuestState State = EQuestState::Inactive;

	UPROPERTY(BlueprintReadOnly)
		float TimeRemaining = 0.f;

	UPROPERTY(BlueprintReadOnly)
		TArray<int32> ObjectiveProgress;

	UPROPERTY(BlueprintReadOnly)
		TArray<EObjectiveState> ObjectiveStates;

	FQuestProgress() {}

	FQuestProgress(const FQuestData& Quest)
	{
		QuestID = Quest.QuestID;
		State = Quest.State;
		TimeRemaining = Quest.TimeRemaining;

		for (const FQuestObjective& Obj : Quest.Objectives)
		{
			ObjectiveProgress.Add(Obj.CurrentCount);
			ObjectiveStates.Add(Obj.State);
		}
	}
};

/**
 * Event fired when quest state changes
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnQuestStateChanged, FName, QuestID, EQuestState, NewState);

/**
 * Event fired when objective progress updates
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnObjectiveProgressUpdated, FName, QuestID, FName, ObjectiveID, int32, CurrentProgress, int32, TargetProgress);

/**
 * Event fired when objective state changes
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnObjectiveStateChanged, FName, QuestID, FName, ObjectiveID, EObjectiveState, NewState);
