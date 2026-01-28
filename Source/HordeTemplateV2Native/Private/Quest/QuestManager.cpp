

#include "Quest/QuestManager.h"
#include "Quest/QuestTypes.h"
#include "AI/ZedPawn.h"
#include "Character/HordeBaseCharacter.h"
#include "Gameplay/HordeGameState.h"
#include "Gameplay/HordePlayerState.h"
#include "Gameplay/HordeWorldSettings.h"
#include "Kismet/GameplayStatics.h"
#include "LevelSequenceActor.h"
#include "LevelSequencePlayer.h"
#include "Sound/SoundCue.h"
#include "Net/UnrealNetwork.h"
#include "Engine/DataTable.h"
#include "HordeTemplateV2Native.h"


UQuestManager::UQuestManager()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
	SetIsReplicatedByDefault(true);
}


void UQuestManager::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UQuestManager, ActiveQuests);
	DOREPLIFETIME(UQuestManager, CompletedQuests);
	DOREPLIFETIME(UQuestManager, FailedQuests);
}


void UQuestManager::BeginPlay()
{
	Super::BeginPlay();

	if (GetOwner() && GetOwner()->HasAuthority())
	{
		LoadQuestsFromDataTable();

		// Delayed initialization to allow world to set up
		FTimerHandle InitTimer;
		GetWorld()->GetTimerManager().SetTimer(InitTimer, this, &UQuestManager::InitializeLevelQuests, 1.f, false);
	}
}


void UQuestManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}

	UpdateQuestTimers(DeltaTime);
	UpdateSurviveObjectives(DeltaTime);
}


void UQuestManager::LoadQuestsFromDataTable()
{
	if (!QuestDataTable)
	{
		// Try to load from world settings
		AHordeWorldSettings* WorldSettings = Cast<AHordeWorldSettings>(GetWorld()->GetWorldSettings());
		if (WorldSettings && WorldSettings->QuestDataTable)
		{
			QuestDataTable = WorldSettings->QuestDataTable;
		}
	}

	if (!QuestDataTable)
	{
		UE_LOG(LogTemp, Warning, TEXT("QuestManager: No QuestDataTable assigned!"));
		return;
	}

	TArray<FQuestData*> QuestRows;
	QuestDataTable->GetAllRows<FQuestData>(TEXT("QuestManager"), QuestRows);

	for (FQuestData* Quest : QuestRows)
	{
		if (Quest && Quest->QuestID != NAME_None)
		{
			AllQuests.Add(Quest->QuestID, *Quest);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("QuestManager: Loaded %d quests from DataTable"), AllQuests.Num());
}


void UQuestManager::InitializeLevelQuests()
{
	// Start any quests marked for auto-start
	for (auto& Pair : AllQuests)
	{
		FQuestData& Quest = Pair.Value;
		if (Quest.bStartOnLevelBegin)
		{
			if (Quest.StartDelay > 0.f)
			{
				FTimerHandle DelayTimer;
				FTimerDelegate DelayDelegate;
				FName QuestID = Quest.QuestID;
				DelayDelegate.BindLambda([this, QuestID]() {
					StartQuest(QuestID);
				});
				GetWorld()->GetTimerManager().SetTimer(DelayTimer, DelayDelegate, Quest.StartDelay, false);
			}
			else
			{
				StartQuest(Quest.QuestID);
			}
		}
	}

	// Also start quests configured in world settings
	AHordeWorldSettings* WorldSettings = Cast<AHordeWorldSettings>(GetWorld()->GetWorldSettings());
	if (WorldSettings)
	{
		for (const FName& QuestID : WorldSettings->AutoStartQuests)
		{
			if (!IsQuestActive(QuestID) && !IsQuestCompleted(QuestID))
			{
				StartQuest(QuestID);
			}
		}
	}
}


void UQuestManager::UpdateQuestTimers(float DeltaTime)
{
	for (FQuestData& Quest : ActiveQuests)
	{
		if (Quest.TimeLimit > 0.f && Quest.TimeRemaining > 0.f)
		{
			Quest.TimeRemaining -= DeltaTime;
			if (Quest.TimeRemaining <= 0.f)
			{
				Quest.TimeRemaining = 0.f;
				if (Quest.bCanFail)
				{
					FailQuest(Quest.QuestID);
				}
			}
		}
	}
}


void UQuestManager::UpdateSurviveObjectives(float DeltaTime)
{
	for (FQuestData& Quest : ActiveQuests)
	{
		for (FQuestObjective& Objective : Quest.Objectives)
		{
			if (Objective.ObjectiveType == EQuestObjectiveType::Survive &&
				Objective.State == EObjectiveState::Active)
			{
				Objective.TimeRemaining -= DeltaTime;
				if (Objective.TimeRemaining <= 0.f)
				{
					Objective.TimeRemaining = 0.f;
					Objective.State = EObjectiveState::Completed;
					Multicast_ObjectiveCompleted(Quest.QuestID, Objective.ObjectiveID);
					OnObjectiveStateChanged.Broadcast(Quest.QuestID, Objective.ObjectiveID, EObjectiveState::Completed);

					AwardObjectiveRewards(Objective);
					FireCompletionTrigger(Objective.CompletionTriggerTag);

					if (Quest.CompletionMode == EQuestCompletionMode::SequentialObjectives)
					{
						AdvanceToNextObjective(Quest);
					}
					CheckQuestCompletion(Quest.QuestID);
				}
			}
		}
	}
}


bool UQuestManager::CanStartQuest(FName QuestID) const
{
	// Check if already active or completed
	if (IsQuestActive(QuestID) || IsQuestCompleted(QuestID))
	{
		return false;
	}

	// Check if quest exists
	const FQuestData* Quest = AllQuests.Find(QuestID);
	if (!Quest)
	{
		return false;
	}

	// Check prerequisite
	if (Quest->PrerequisiteQuestID != NAME_None)
	{
		if (!IsQuestCompleted(Quest->PrerequisiteQuestID))
		{
			return false;
		}
	}

	// Check max active quests
	if (!bAllowMultipleActiveQuests && ActiveQuests.Num() > 0)
	{
		return false;
	}
	else if (bAllowMultipleActiveQuests && ActiveQuests.Num() >= MaxActiveQuests)
	{
		return false;
	}

	return true;
}


bool UQuestManager::StartQuest(FName QuestID)
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return false;
	}

	if (!CanStartQuest(QuestID))
	{
		return false;
	}

	FQuestData* QuestTemplate = AllQuests.Find(QuestID);
	if (!QuestTemplate)
	{
		UE_LOG(LogTemp, Warning, TEXT("QuestManager: Quest '%s' not found!"), *QuestID.ToString());
		return false;
	}

	// Create a copy for active tracking
	FQuestData ActiveQuest = *QuestTemplate;
	ActiveQuest.State = EQuestState::Active;
	ActiveQuest.TimeRemaining = ActiveQuest.TimeLimit;

	// Initialize objectives
	for (FQuestObjective& Objective : ActiveQuest.Objectives)
	{
		Objective.CurrentCount = 0;
		Objective.State = EObjectiveState::Locked;
		if (Objective.ObjectiveType == EQuestObjectiveType::Survive)
		{
			Objective.TimeRemaining = Objective.SurviveDuration;
		}
	}

	ActiveQuests.Add(ActiveQuest);
	ActivateInitialObjectives(ActiveQuests.Last());

	// Play start sound
	PlayQuestSound(ActiveQuest.StartSound);

	// Notify
	Multicast_QuestStarted(QuestID);
	OnQuestStateChanged.Broadcast(QuestID, EQuestState::Active);

	UE_LOG(LogTemp, Log, TEXT("QuestManager: Started quest '%s'"), *QuestID.ToString());
	return true;
}


void UQuestManager::ActivateInitialObjectives(FQuestData& Quest)
{
	if (Quest.CompletionMode == EQuestCompletionMode::SequentialObjectives)
	{
		// Only activate first objective
		if (Quest.Objectives.Num() > 0)
		{
			Quest.Objectives[0].State = EObjectiveState::Active;
			OnObjectiveStateChanged.Broadcast(Quest.QuestID, Quest.Objectives[0].ObjectiveID, EObjectiveState::Active);
		}
	}
	else
	{
		// Activate all objectives
		for (FQuestObjective& Objective : Quest.Objectives)
		{
			Objective.State = EObjectiveState::Active;
			OnObjectiveStateChanged.Broadcast(Quest.QuestID, Objective.ObjectiveID, EObjectiveState::Active);
		}
	}
}


void UQuestManager::AdvanceToNextObjective(FQuestData& Quest)
{
	for (int32 i = 0; i < Quest.Objectives.Num(); i++)
	{
		if (Quest.Objectives[i].State == EObjectiveState::Locked)
		{
			Quest.Objectives[i].State = EObjectiveState::Active;
			OnObjectiveStateChanged.Broadcast(Quest.QuestID, Quest.Objectives[i].ObjectiveID, EObjectiveState::Active);
			return;
		}
	}
}


void UQuestManager::CheckQuestCompletion(FName QuestID)
{
	FQuestData* Quest = FindActiveQuest(QuestID);
	if (!Quest)
	{
		return;
	}

	bool bShouldComplete = false;

	switch (Quest->CompletionMode)
	{
	case EQuestCompletionMode::AllObjectives:
		bShouldComplete = Quest->AreAllRequiredObjectivesComplete();
		break;

	case EQuestCompletionMode::AnyObjective:
		bShouldComplete = Quest->IsAnyObjectiveComplete();
		break;

	case EQuestCompletionMode::SequentialObjectives:
		bShouldComplete = Quest->AreAllRequiredObjectivesComplete();
		break;
	}

	if (bShouldComplete)
	{
		CompleteQuest(QuestID);
	}
}


void UQuestManager::CompleteQuest(FName QuestID)
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}

	FQuestData* Quest = FindActiveQuest(QuestID);
	if (!Quest)
	{
		return;
	}

	Quest->State = EQuestState::Completed;

	// Award rewards
	AwardQuestRewards(*Quest);

	// Play completion sound
	PlayQuestSound(Quest->CompleteSound);

	// Move to completed list
	CompletedQuests.Add(QuestID);

	// Store follow-up info before removing
	FName FollowUpID = Quest->FollowUpQuestID;
	bool bAutoStart = Quest->bAutoStartFollowUp;

	// Remove from active
	ActiveQuests.RemoveAll([QuestID](const FQuestData& Q) { return Q.QuestID == QuestID; });

	// Notify
	Multicast_QuestCompleted(QuestID);
	OnQuestStateChanged.Broadcast(QuestID, EQuestState::Completed);

	// Start follow-up quest
	if (FollowUpID != NAME_None && bAutoStart)
	{
		StartQuest(FollowUpID);
	}

	UE_LOG(LogTemp, Log, TEXT("QuestManager: Completed quest '%s'"), *QuestID.ToString());
}


void UQuestManager::FailQuest(FName QuestID)
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}

	FQuestData* Quest = FindActiveQuest(QuestID);
	if (!Quest || !Quest->bCanFail)
	{
		return;
	}

	Quest->State = EQuestState::Failed;

	// Play fail sound
	PlayQuestSound(Quest->FailSound);

	// Move to failed list
	FailedQuests.Add(QuestID);

	// Remove from active
	ActiveQuests.RemoveAll([QuestID](const FQuestData& Q) { return Q.QuestID == QuestID; });

	// Notify
	Multicast_QuestFailed(QuestID);
	OnQuestStateChanged.Broadcast(QuestID, EQuestState::Failed);

	UE_LOG(LogTemp, Log, TEXT("QuestManager: Failed quest '%s'"), *QuestID.ToString());
}


void UQuestManager::AbandonQuest(FName QuestID)
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}

	ActiveQuests.RemoveAll([QuestID](const FQuestData& Q) { return Q.QuestID == QuestID; });
	OnQuestStateChanged.Broadcast(QuestID, EQuestState::Inactive);
}


bool UQuestManager::TriggerQuestByTag(FName TriggerTag)
{
	for (auto& Pair : AllQuests)
	{
		if (Pair.Value.TriggerTag == TriggerTag)
		{
			return StartQuest(Pair.Value.QuestID);
		}
	}
	return false;
}


void UQuestManager::ReportLocationReached(FName LocationTag, AHordeBaseCharacter* Player)
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}

	for (FQuestData& Quest : ActiveQuests)
	{
		for (FQuestObjective& Objective : Quest.Objectives)
		{
			if (Objective.State == EObjectiveState::Active &&
				Objective.ObjectiveType == EQuestObjectiveType::Location &&
				Objective.TargetActorTag == LocationTag)
			{
				Objective.CurrentCount++;
				if (Objective.CurrentCount >= Objective.TargetCount)
				{
					Objective.State = EObjectiveState::Completed;
					Multicast_ObjectiveCompleted(Quest.QuestID, Objective.ObjectiveID);
					OnObjectiveStateChanged.Broadcast(Quest.QuestID, Objective.ObjectiveID, EObjectiveState::Completed);

					AwardObjectiveRewards(Objective);
					FireCompletionTrigger(Objective.CompletionTriggerTag);

					if (Quest.CompletionMode == EQuestCompletionMode::SequentialObjectives)
					{
						AdvanceToNextObjective(Quest);
					}
					CheckQuestCompletion(Quest.QuestID);
				}
				else
				{
					Multicast_ObjectiveUpdated(Quest.QuestID, Objective.ObjectiveID, Objective.CurrentCount, Objective.TargetCount);
					OnObjectiveProgressUpdated.Broadcast(Quest.QuestID, Objective.ObjectiveID, Objective.CurrentCount, Objective.TargetCount);
				}
			}
		}
	}
}


void UQuestManager::ReportItemCollected(FName ItemID, int32 Count)
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}

	for (FQuestData& Quest : ActiveQuests)
	{
		for (FQuestObjective& Objective : Quest.Objectives)
		{
			if (Objective.State == EObjectiveState::Active &&
				Objective.ObjectiveType == EQuestObjectiveType::CollectItem &&
				Objective.TargetItemID == ItemID)
			{
				Objective.CurrentCount += Count;

				if (Objective.CurrentCount >= Objective.TargetCount)
				{
					Objective.CurrentCount = Objective.TargetCount;
					Objective.State = EObjectiveState::Completed;
					Multicast_ObjectiveCompleted(Quest.QuestID, Objective.ObjectiveID);
					OnObjectiveStateChanged.Broadcast(Quest.QuestID, Objective.ObjectiveID, EObjectiveState::Completed);

					AwardObjectiveRewards(Objective);
					FireCompletionTrigger(Objective.CompletionTriggerTag);

					if (Quest.CompletionMode == EQuestCompletionMode::SequentialObjectives)
					{
						AdvanceToNextObjective(Quest);
					}
					CheckQuestCompletion(Quest.QuestID);
				}
				else
				{
					Multicast_ObjectiveUpdated(Quest.QuestID, Objective.ObjectiveID, Objective.CurrentCount, Objective.TargetCount);
					OnObjectiveProgressUpdated.Broadcast(Quest.QuestID, Objective.ObjectiveID, Objective.CurrentCount, Objective.TargetCount);
				}
			}
		}
	}
}


void UQuestManager::ReportEnemyKilled(AZedPawn* Enemy, AHordeBaseCharacter* Killer)
{
	if (!GetOwner() || !GetOwner()->HasAuthority() || !Enemy)
	{
		return;
	}

	for (FQuestData& Quest : ActiveQuests)
	{
		for (FQuestObjective& Objective : Quest.Objectives)
		{
			if (Objective.State != EObjectiveState::Active)
			{
				continue;
			}

			bool bMatches = false;

			if (Objective.ObjectiveType == EQuestObjectiveType::KillEnemies)
			{
				bMatches = true;
			}
			else if (Objective.ObjectiveType == EQuestObjectiveType::KillSpecificType)
			{
				if (Objective.TargetEnemyClass && Enemy->IsA(Objective.TargetEnemyClass))
				{
					bMatches = true;
				}
			}

			if (bMatches)
			{
				Objective.CurrentCount++;

				if (Objective.CurrentCount >= Objective.TargetCount)
				{
					Objective.CurrentCount = Objective.TargetCount;
					Objective.State = EObjectiveState::Completed;
					Multicast_ObjectiveCompleted(Quest.QuestID, Objective.ObjectiveID);
					OnObjectiveStateChanged.Broadcast(Quest.QuestID, Objective.ObjectiveID, EObjectiveState::Completed);

					AwardObjectiveRewards(Objective);
					FireCompletionTrigger(Objective.CompletionTriggerTag);

					if (Quest.CompletionMode == EQuestCompletionMode::SequentialObjectives)
					{
						AdvanceToNextObjective(Quest);
					}
					CheckQuestCompletion(Quest.QuestID);
				}
				else
				{
					Multicast_ObjectiveUpdated(Quest.QuestID, Objective.ObjectiveID, Objective.CurrentCount, Objective.TargetCount);
					OnObjectiveProgressUpdated.Broadcast(Quest.QuestID, Objective.ObjectiveID, Objective.CurrentCount, Objective.TargetCount);
				}
			}
		}
	}
}


void UQuestManager::ReportInteraction(FName InteractableTag, AHordeBaseCharacter* Player)
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}

	for (FQuestData& Quest : ActiveQuests)
	{
		for (FQuestObjective& Objective : Quest.Objectives)
		{
			if (Objective.State == EObjectiveState::Active &&
				Objective.ObjectiveType == EQuestObjectiveType::Interact &&
				Objective.TargetActorTag == InteractableTag)
			{
				Objective.CurrentCount++;

				if (Objective.CurrentCount >= Objective.TargetCount)
				{
					Objective.State = EObjectiveState::Completed;
					Multicast_ObjectiveCompleted(Quest.QuestID, Objective.ObjectiveID);
					OnObjectiveStateChanged.Broadcast(Quest.QuestID, Objective.ObjectiveID, EObjectiveState::Completed);

					AwardObjectiveRewards(Objective);
					FireCompletionTrigger(Objective.CompletionTriggerTag);

					if (Quest.CompletionMode == EQuestCompletionMode::SequentialObjectives)
					{
						AdvanceToNextObjective(Quest);
					}
					CheckQuestCompletion(Quest.QuestID);
				}
				else
				{
					Multicast_ObjectiveUpdated(Quest.QuestID, Objective.ObjectiveID, Objective.CurrentCount, Objective.TargetCount);
					OnObjectiveProgressUpdated.Broadcast(Quest.QuestID, Objective.ObjectiveID, Objective.CurrentCount, Objective.TargetCount);
				}
			}
		}
	}
}


void UQuestManager::ReportSequenceTriggered(FName SequenceTag)
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}

	for (FQuestData& Quest : ActiveQuests)
	{
		for (FQuestObjective& Objective : Quest.Objectives)
		{
			if (Objective.State == EObjectiveState::Active &&
				Objective.ObjectiveType == EQuestObjectiveType::TriggerSequence &&
				Objective.TargetActorTag == SequenceTag)
			{
				// Trigger the sequence
				TriggerLevelSequence(Objective.SequenceToTrigger);

				Objective.State = EObjectiveState::Completed;
				Multicast_ObjectiveCompleted(Quest.QuestID, Objective.ObjectiveID);
				OnObjectiveStateChanged.Broadcast(Quest.QuestID, Objective.ObjectiveID, EObjectiveState::Completed);

				AwardObjectiveRewards(Objective);
				FireCompletionTrigger(Objective.CompletionTriggerTag);

				if (Quest.CompletionMode == EQuestCompletionMode::SequentialObjectives)
				{
					AdvanceToNextObjective(Quest);
				}
				CheckQuestCompletion(Quest.QuestID);
			}
		}
	}
}


void UQuestManager::ReportCustomProgress(FName QuestID, FName ObjectiveID, int32 ProgressAmount)
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}

	FQuestData* Quest = FindActiveQuest(QuestID);
	if (!Quest)
	{
		return;
	}

	FQuestObjective* Objective = FindObjective(*Quest, ObjectiveID);
	if (!Objective || Objective->State != EObjectiveState::Active)
	{
		return;
	}

	Objective->CurrentCount += ProgressAmount;

	if (Objective->CurrentCount >= Objective->TargetCount)
	{
		Objective->CurrentCount = Objective->TargetCount;
		Objective->State = EObjectiveState::Completed;
		Multicast_ObjectiveCompleted(QuestID, ObjectiveID);
		OnObjectiveStateChanged.Broadcast(QuestID, ObjectiveID, EObjectiveState::Completed);

		AwardObjectiveRewards(*Objective);
		FireCompletionTrigger(Objective->CompletionTriggerTag);

		if (Quest->CompletionMode == EQuestCompletionMode::SequentialObjectives)
		{
			AdvanceToNextObjective(*Quest);
		}
		CheckQuestCompletion(QuestID);
	}
	else
	{
		Multicast_ObjectiveUpdated(QuestID, ObjectiveID, Objective->CurrentCount, Objective->TargetCount);
		OnObjectiveProgressUpdated.Broadcast(QuestID, ObjectiveID, Objective->CurrentCount, Objective->TargetCount);
	}
}


void UQuestManager::ForceCompleteObjective(FName QuestID, FName ObjectiveID)
{
	FQuestData* Quest = FindActiveQuest(QuestID);
	if (!Quest)
	{
		return;
	}

	FQuestObjective* Objective = FindObjective(*Quest, ObjectiveID);
	if (!Objective)
	{
		return;
	}

	Objective->CurrentCount = Objective->TargetCount;
	Objective->State = EObjectiveState::Completed;
	Multicast_ObjectiveCompleted(QuestID, ObjectiveID);
	OnObjectiveStateChanged.Broadcast(QuestID, ObjectiveID, EObjectiveState::Completed);

	AwardObjectiveRewards(*Objective);
	FireCompletionTrigger(Objective->CompletionTriggerTag);

	if (Quest->CompletionMode == EQuestCompletionMode::SequentialObjectives)
	{
		AdvanceToNextObjective(*Quest);
	}
	CheckQuestCompletion(QuestID);
}


bool UQuestManager::GetActiveQuest(FQuestData& OutQuest) const
{
	if (ActiveQuests.Num() > 0)
	{
		OutQuest = ActiveQuests[0];
		return true;
	}
	return false;
}


TArray<FQuestData> UQuestManager::GetAllActiveQuests() const
{
	return ActiveQuests;
}


bool UQuestManager::GetQuestByID(FName QuestID, FQuestData& OutQuest) const
{
	// Check active quests first
	for (const FQuestData& Quest : ActiveQuests)
	{
		if (Quest.QuestID == QuestID)
		{
			OutQuest = Quest;
			return true;
		}
	}

	// Check all quests
	const FQuestData* Quest = AllQuests.Find(QuestID);
	if (Quest)
	{
		OutQuest = *Quest;
		return true;
	}

	return false;
}


bool UQuestManager::IsQuestActive(FName QuestID) const
{
	for (const FQuestData& Quest : ActiveQuests)
	{
		if (Quest.QuestID == QuestID)
		{
			return true;
		}
	}
	return false;
}


bool UQuestManager::IsQuestCompleted(FName QuestID) const
{
	return CompletedQuests.Contains(QuestID);
}


EQuestState UQuestManager::GetQuestState(FName QuestID) const
{
	// Check active
	for (const FQuestData& Quest : ActiveQuests)
	{
		if (Quest.QuestID == QuestID)
		{
			return Quest.State;
		}
	}

	// Check completed
	if (CompletedQuests.Contains(QuestID))
	{
		return EQuestState::Completed;
	}

	// Check failed
	if (FailedQuests.Contains(QuestID))
	{
		return EQuestState::Failed;
	}

	return EQuestState::Inactive;
}


bool UQuestManager::GetCurrentObjective(FName QuestID, FQuestObjective& OutObjective) const
{
	const FQuestData* Quest = FindActiveQuest(QuestID);
	if (!Quest)
	{
		return false;
	}

	for (const FQuestObjective& Obj : Quest->Objectives)
	{
		if (Obj.State == EObjectiveState::Active)
		{
			OutObjective = Obj;
			return true;
		}
	}

	return false;
}


TArray<FName> UQuestManager::GetCompletedQuestIDs() const
{
	return CompletedQuests;
}


UQuestManager* UQuestManager::GetQuestManager(const UObject* WorldContextObject)
{
	if (!WorldContextObject)
	{
		return nullptr;
	}

	UWorld* World = WorldContextObject->GetWorld();
	if (!World)
	{
		return nullptr;
	}

	AGameStateBase* GameState = World->GetGameState();
	if (!GameState)
	{
		return nullptr;
	}

	return GameState->FindComponentByClass<UQuestManager>();
}


void UQuestManager::OnRep_ActiveQuests()
{
	// Client-side update notification
	for (const FQuestData& Quest : ActiveQuests)
	{
		OnQuestStateChanged.Broadcast(Quest.QuestID, Quest.State);
	}
}


void UQuestManager::Multicast_QuestStarted_Implementation(FName QuestID)
{
	OnQuestStateChanged.Broadcast(QuestID, EQuestState::Active);
}


void UQuestManager::Multicast_QuestCompleted_Implementation(FName QuestID)
{
	OnQuestStateChanged.Broadcast(QuestID, EQuestState::Completed);
}


void UQuestManager::Multicast_QuestFailed_Implementation(FName QuestID)
{
	OnQuestStateChanged.Broadcast(QuestID, EQuestState::Failed);
}


void UQuestManager::Multicast_ObjectiveUpdated_Implementation(FName QuestID, FName ObjectiveID, int32 CurrentProgress, int32 TargetProgress)
{
	OnObjectiveProgressUpdated.Broadcast(QuestID, ObjectiveID, CurrentProgress, TargetProgress);
}


void UQuestManager::Multicast_ObjectiveCompleted_Implementation(FName QuestID, FName ObjectiveID)
{
	OnObjectiveStateChanged.Broadcast(QuestID, ObjectiveID, EObjectiveState::Completed);
}


void UQuestManager::PlayQuestSound(const TSoftObjectPtr<USoundCue>& Sound)
{
	if (Sound.IsNull())
	{
		return;
	}

	USoundCue* LoadedSound = Sound.LoadSynchronous();
	if (LoadedSound)
	{
		UGameplayStatics::PlaySound2D(GetWorld(), LoadedSound);
	}
}


void UQuestManager::AwardQuestRewards(const FQuestData& Quest)
{
	AHordeGameState* GS = Cast<AHordeGameState>(GetWorld()->GetGameState());
	if (!GS)
	{
		return;
	}

	// Award to all players
	for (APlayerState* PS : GS->PlayerArray)
	{
		AHordePlayerState* HPS = Cast<AHordePlayerState>(PS);
		if (HPS)
		{
			if (Quest.CompletionPoints > 0)
			{
				HPS->AddPoints(Quest.CompletionPoints, EPointType::EPointCasual);
			}
			if (Quest.CompletionMoney > 0)
			{
				HPS->AddMoney(Quest.CompletionMoney);
			}
		}
	}
}


void UQuestManager::AwardObjectiveRewards(const FQuestObjective& Objective)
{
	if (Objective.PointsReward <= 0 && Objective.MoneyReward <= 0)
	{
		return;
	}

	AHordeGameState* GS = Cast<AHordeGameState>(GetWorld()->GetGameState());
	if (!GS)
	{
		return;
	}

	// Award to all players
	for (APlayerState* PS : GS->PlayerArray)
	{
		AHordePlayerState* HPS = Cast<AHordePlayerState>(PS);
		if (HPS)
		{
			if (Objective.PointsReward > 0)
			{
				HPS->AddPoints(Objective.PointsReward, EPointType::EPointCasual);
			}
			if (Objective.MoneyReward > 0)
			{
				HPS->AddMoney(Objective.MoneyReward);
			}
		}
	}
}


void UQuestManager::FireCompletionTrigger(const FName& TriggerTag)
{
	if (TriggerTag == NAME_None)
	{
		return;
	}

	// Find actors with matching tag and trigger them
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), TriggerTag, FoundActors);

	for (AActor* Actor : FoundActors)
	{
		// Try to call a trigger function if the actor has one
		// This allows Blueprint actors to respond to quest triggers
		if (Actor)
		{
			UFunction* TriggerFunc = Actor->FindFunction(FName("OnQuestTrigger"));
			if (TriggerFunc)
			{
				Actor->ProcessEvent(TriggerFunc, nullptr);
			}
		}
	}
}


FQuestData* UQuestManager::FindActiveQuest(FName QuestID)
{
	for (FQuestData& Quest : ActiveQuests)
	{
		if (Quest.QuestID == QuestID)
		{
			return &Quest;
		}
	}
	return nullptr;
}


const FQuestData* UQuestManager::FindActiveQuest(FName QuestID) const
{
	for (const FQuestData& Quest : ActiveQuests)
	{
		if (Quest.QuestID == QuestID)
		{
			return &Quest;
		}
	}
	return nullptr;
}


FQuestObjective* UQuestManager::FindObjective(FQuestData& Quest, FName ObjectiveID)
{
	for (FQuestObjective& Obj : Quest.Objectives)
	{
		if (Obj.ObjectiveID == ObjectiveID)
		{
			return &Obj;
		}
	}
	return nullptr;
}


void UQuestManager::TriggerLevelSequence(const TSoftObjectPtr<ALevelSequenceActor>& SequenceActor)
{
	if (SequenceActor.IsNull())
	{
		return;
	}

	ALevelSequenceActor* LoadedActor = SequenceActor.LoadSynchronous();
	if (LoadedActor)
	{
		ULevelSequencePlayer* Player = LoadedActor->GetSequencePlayer();
		if (Player)
		{
			Player->Play();
		}
	}
}
