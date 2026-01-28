

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "QuestTypes.h"
#include "QuestManager.generated.h"

class AZedPawn;
class AHordeBaseCharacter;
class ALevelSequenceActor;
class ULevelSequence;

/**
 * Quest Manager Component - Attach to GameState for global quest management
 * Handles all quest logic, progression, and synchronization across players
 */
UCLASS(ClassGroup = "Quest", meta = (BlueprintSpawnableComponent))
class HORDETEMPLATEV2NATIVE_API UQuestManager : public UActorComponent
{
	GENERATED_BODY()

public:
	UQuestManager();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// ==================== EVENTS ====================

	/** Fired when a quest state changes */
	UPROPERTY(BlueprintAssignable, Category = "Quest|Events")
		FOnQuestStateChanged OnQuestStateChanged;

	/** Fired when objective progress updates */
	UPROPERTY(BlueprintAssignable, Category = "Quest|Events")
		FOnObjectiveProgressUpdated OnObjectiveProgressUpdated;

	/** Fired when objective state changes */
	UPROPERTY(BlueprintAssignable, Category = "Quest|Events")
		FOnObjectiveStateChanged OnObjectiveStateChanged;

	// ==================== CONFIGURATION ====================

	/** DataTable containing quest definitions */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Configuration")
		UDataTable* QuestDataTable;

	/** If true, allows multiple quests to be active simultaneously */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Configuration")
		bool bAllowMultipleActiveQuests = false;

	/** Maximum number of active quests (if bAllowMultipleActiveQuests) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Configuration", meta = (EditCondition = "bAllowMultipleActiveQuests"))
		int32 MaxActiveQuests = 3;

	// ==================== QUEST MANAGEMENT ====================

	/** Start a quest by ID */
	UFUNCTION(BlueprintCallable, Category = "Quest")
		bool StartQuest(FName QuestID);

	/** Complete a quest (called internally when objectives done) */
	UFUNCTION(BlueprintCallable, Category = "Quest")
		void CompleteQuest(FName QuestID);

	/** Fail a quest */
	UFUNCTION(BlueprintCallable, Category = "Quest")
		void FailQuest(FName QuestID);

	/** Abandon/cancel a quest */
	UFUNCTION(BlueprintCallable, Category = "Quest")
		void AbandonQuest(FName QuestID);

	/** Trigger a quest by its trigger tag */
	UFUNCTION(BlueprintCallable, Category = "Quest")
		bool TriggerQuestByTag(FName TriggerTag);

	// ==================== OBJECTIVE PROGRESS ====================

	/** Report progress on a location objective */
	UFUNCTION(BlueprintCallable, Category = "Quest|Progress")
		void ReportLocationReached(FName LocationTag, AHordeBaseCharacter* Player);

	/** Report an item collected */
	UFUNCTION(BlueprintCallable, Category = "Quest|Progress")
		void ReportItemCollected(FName ItemID, int32 Count = 1);

	/** Report an enemy killed */
	UFUNCTION(BlueprintCallable, Category = "Quest|Progress")
		void ReportEnemyKilled(AZedPawn* Enemy, AHordeBaseCharacter* Killer);

	/** Report an interaction */
	UFUNCTION(BlueprintCallable, Category = "Quest|Progress")
		void ReportInteraction(FName InteractableTag, AHordeBaseCharacter* Player);

	/** Report a sequence triggered */
	UFUNCTION(BlueprintCallable, Category = "Quest|Progress")
		void ReportSequenceTriggered(FName SequenceTag);

	/** Report custom objective progress (for Blueprint objectives) */
	UFUNCTION(BlueprintCallable, Category = "Quest|Progress")
		void ReportCustomProgress(FName QuestID, FName ObjectiveID, int32 ProgressAmount = 1);

	/** Complete an objective directly */
	UFUNCTION(BlueprintCallable, Category = "Quest|Progress")
		void ForceCompleteObjective(FName QuestID, FName ObjectiveID);

	// ==================== QUERY FUNCTIONS ====================

	/** Get current active quest (primary quest if multiple allowed) */
	UFUNCTION(BlueprintPure, Category = "Quest|Query")
		bool GetActiveQuest(FQuestData& OutQuest) const;

	/** Get all active quests */
	UFUNCTION(BlueprintPure, Category = "Quest|Query")
		TArray<FQuestData> GetAllActiveQuests() const;

	/** Get quest by ID */
	UFUNCTION(BlueprintPure, Category = "Quest|Query")
		bool GetQuestByID(FName QuestID, FQuestData& OutQuest) const;

	/** Check if a quest is active */
	UFUNCTION(BlueprintPure, Category = "Quest|Query")
		bool IsQuestActive(FName QuestID) const;

	/** Check if a quest is completed */
	UFUNCTION(BlueprintPure, Category = "Quest|Query")
		bool IsQuestCompleted(FName QuestID) const;

	/** Get quest state */
	UFUNCTION(BlueprintPure, Category = "Quest|Query")
		EQuestState GetQuestState(FName QuestID) const;

	/** Get current objective for a quest */
	UFUNCTION(BlueprintPure, Category = "Quest|Query")
		bool GetCurrentObjective(FName QuestID, FQuestObjective& OutObjective) const;

	/** Get all completed quest IDs */
	UFUNCTION(BlueprintPure, Category = "Quest|Query")
		TArray<FName> GetCompletedQuestIDs() const;

	// ==================== UTILITY ====================

	/** Get static instance (from GameState) */
	UFUNCTION(BlueprintPure, Category = "Quest", meta = (WorldContext = "WorldContextObject"))
		static UQuestManager* GetQuestManager(const UObject* WorldContextObject);

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// ==================== INTERNAL DATA ====================

	/** All quests loaded from DataTable */
	UPROPERTY()
		TMap<FName, FQuestData> AllQuests;

	/** Currently active quests */
	UPROPERTY(ReplicatedUsing = OnRep_ActiveQuests)
		TArray<FQuestData> ActiveQuests;

	/** Completed quest IDs */
	UPROPERTY(Replicated)
		TArray<FName> CompletedQuests;

	/** Failed quest IDs */
	UPROPERTY(Replicated)
		TArray<FName> FailedQuests;

	// ==================== REPLICATION ====================

	UFUNCTION()
		void OnRep_ActiveQuests();

	/** Multicast RPC to notify all clients of quest events */
	UFUNCTION(NetMulticast, Reliable)
		void Multicast_QuestStarted(FName QuestID);

	UFUNCTION(NetMulticast, Reliable)
		void Multicast_QuestCompleted(FName QuestID);

	UFUNCTION(NetMulticast, Reliable)
		void Multicast_QuestFailed(FName QuestID);

	UFUNCTION(NetMulticast, Reliable)
		void Multicast_ObjectiveUpdated(FName QuestID, FName ObjectiveID, int32 CurrentProgress, int32 TargetProgress);

	UFUNCTION(NetMulticast, Reliable)
		void Multicast_ObjectiveCompleted(FName QuestID, FName ObjectiveID);

	// ==================== INTERNAL FUNCTIONS ====================

	/** Load quests from DataTable */
	void LoadQuestsFromDataTable();

	/** Initialize level-start quests */
	void InitializeLevelQuests();

	/** Update active quest timers */
	void UpdateQuestTimers(float DeltaTime);

	/** Update survive objectives */
	void UpdateSurviveObjectives(float DeltaTime);

	/** Check if quest can be started */
	bool CanStartQuest(FName QuestID) const;

	/** Activate the first objective(s) of a quest */
	void ActivateInitialObjectives(FQuestData& Quest);

	/** Advance to next objective (for sequential quests) */
	void AdvanceToNextObjective(FQuestData& Quest);

	/** Check if quest should complete based on objectives */
	void CheckQuestCompletion(FName QuestID);

	/** Play quest sounds */
	void PlayQuestSound(const TSoftObjectPtr<USoundCue>& Sound);

	/** Award quest rewards */
	void AwardQuestRewards(const FQuestData& Quest);

	/** Award objective rewards */
	void AwardObjectiveRewards(const FQuestObjective& Objective);

	/** Fire completion trigger */
	void FireCompletionTrigger(const FName& TriggerTag);

	/** Find quest in active list */
	FQuestData* FindActiveQuest(FName QuestID);
	const FQuestData* FindActiveQuest(FName QuestID) const;

	/** Find objective in quest */
	FQuestObjective* FindObjective(FQuestData& Quest, FName ObjectiveID);

	/** Trigger level sequence */
	void TriggerLevelSequence(const TSoftObjectPtr<ALevelSequenceActor>& SequenceActor);
};
