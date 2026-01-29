// Copyright (c) 2018 - 2026 Marc Fraedrich
// Horde Template V2 (Native) - A cooperative zombie survival game framework
// Licensed under the MIT License

#include "Quest/QuestTrigger.h"
#include "Quest/QuestManager.h"
#include "Character/HordeBaseCharacter.h"
#include "Gameplay/HordeGameState.h"
#include "Components/BoxComponent.h"
#include "Components/BillboardComponent.h"
#include "DrawDebugHelpers.h"
#include "ConstructorHelpers.h"
#include "Runtime/Engine/Public/EngineUtils.h"

/**
 * @file QuestTrigger.cpp
 * @brief Implementation of volume-based quest triggers with prerequisite support
 * @author Marc Fraedrich
 */

AQuestTrigger::AQuestTrigger()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

	// Create trigger volume
	TriggerVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerVolume"));
	TriggerVolume->SetBoxExtent(FVector(100.f, 100.f, 100.f));
	TriggerVolume->SetCollisionProfileName(TEXT("Trigger"));
	TriggerVolume->SetGenerateOverlapEvents(true);
	RootComponent = TriggerVolume;

	TriggerVolume->OnComponentBeginOverlap.AddDynamic(this, &AQuestTrigger::OnTriggerBeginOverlap);
	TriggerVolume->OnComponentEndOverlap.AddDynamic(this, &AQuestTrigger::OnTriggerEndOverlap);

	// Create editor icon
	EditorIcon = CreateDefaultSubobject<UBillboardComponent>(TEXT("EditorIcon"));
	EditorIcon->SetupAttachment(RootComponent);
	EditorIcon->SetRelativeLocation(FVector(0.f, 0.f, 50.f));
	EditorIcon->bHiddenInGame = true;

	const ConstructorHelpers::FObjectFinder<UTexture2D> IconAsset(TEXT("Texture2D'/Engine/EditorResources/S_Trigger.S_Trigger'"));
	if (IconAsset.Succeeded())
	{
		EditorIcon->SetSprite(IconAsset.Object);
	}
}


void AQuestTrigger::BeginPlay()
{
	Super::BeginPlay();

	bIsEnabled = bStartEnabled;

	// Enable tick for AllPlayersInside mode
	if (TriggerMode == EQuestTriggerMode::OnAllPlayersInside)
	{
		PrimaryActorTick.bStartWithTickEnabled = true;
		SetActorTickEnabled(true);
	}
}


void AQuestTrigger::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Debug visualization
	if (bShowDebug)
	{
		FColor DebugColor = bIsEnabled ? FColor::Green : FColor::Red;
		if (bHasTriggered && bOneShot)
		{
			DebugColor = FColor::Yellow;
		}
		DrawDebugBox(GetWorld(), GetActorLocation(), TriggerVolume->GetScaledBoxExtent(), DebugColor, false, -1.f, 0, 2.f);
	}

	// Check for all players inside
	if (TriggerMode == EQuestTriggerMode::OnAllPlayersInside && bIsEnabled && !bHasTriggered)
	{
		int32 TotalPlayers = GetTotalPlayerCount();
		if (TotalPlayers > 0 && PlayersInTrigger.Num() >= TotalPlayers)
		{
			if (CheckPrerequisites())
			{
				ActivateTrigger(PlayersInTrigger.Num() > 0 ? PlayersInTrigger[0] : nullptr);
			}
		}
	}
}


void AQuestTrigger::OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AHordeBaseCharacter* Player = Cast<AHordeBaseCharacter>(OtherActor);
	if (!Player || Player->GetIsDead())
	{
		return;
	}

	PlayersInTrigger.AddUnique(Player);

	// Handle OnOverlap mode
	if (TriggerMode == EQuestTriggerMode::OnOverlap && bIsEnabled && !bHasTriggered)
	{
		if (CheckPrerequisites())
		{
			ActivateTrigger(Player);
		}
	}
}


void AQuestTrigger::OnTriggerEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	AHordeBaseCharacter* Player = Cast<AHordeBaseCharacter>(OtherActor);
	if (Player)
	{
		PlayersInTrigger.Remove(Player);
	}
}


bool AQuestTrigger::CheckPrerequisites() const
{
	UQuestManager* QuestManager = UQuestManager::GetQuestManager(this);
	if (!QuestManager)
	{
		return true; // No quest manager, allow trigger
	}

	// Check required active quest
	if (RequiredActiveQuest != NAME_None)
	{
		if (!QuestManager->IsQuestActive(RequiredActiveQuest))
		{
			return false;
		}
	}

	// Check required completed quest
	if (RequiredCompletedQuest != NAME_None)
	{
		if (!QuestManager->IsQuestCompleted(RequiredCompletedQuest))
		{
			return false;
		}
	}

	return true;
}


void AQuestTrigger::ActivateTrigger(ACharacter* ActivatingPlayer)
{
	if (!bIsEnabled)
	{
		return;
	}

	if (bOneShot && bHasTriggered)
	{
		return;
	}

	// Only execute on server
	if (!HasAuthority())
	{
		return;
	}

	ExecuteTriggerActions(ActivatingPlayer);

	bHasTriggered = true;
	OnTriggerActivated(ActivatingPlayer);

	if (bOneShot)
	{
		bIsEnabled = false;
	}
}


void AQuestTrigger::ExecuteTriggerActions(ACharacter* ActivatingPlayer)
{
	UQuestManager* QuestManager = UQuestManager::GetQuestManager(this);
	if (!QuestManager)
	{
		UE_LOG(LogTemp, Warning, TEXT("QuestTrigger: No QuestManager found!"));
		return;
	}

	AHordeBaseCharacter* HordePlayer = Cast<AHordeBaseCharacter>(ActivatingPlayer);

	// Start quest by ID
	if (QuestToStart != NAME_None)
	{
		QuestManager->StartQuest(QuestToStart);
	}

	// Trigger quest by tag
	if (QuestTriggerTag != NAME_None)
	{
		QuestManager->TriggerQuestByTag(QuestTriggerTag);
	}

	// Complete objective
	if (QuestIDForObjective != NAME_None && ObjectiveToComplete != NAME_None)
	{
		QuestManager->ForceCompleteObjective(QuestIDForObjective, ObjectiveToComplete);
	}

	// Report location reached
	if (LocationTag != NAME_None)
	{
		QuestManager->ReportLocationReached(LocationTag, HordePlayer);
	}

	// Report interaction
	if (InteractionTag != NAME_None)
	{
		QuestManager->ReportInteraction(InteractionTag, HordePlayer);
	}

	// Report sequence trigger
	if (SequenceTag != NAME_None)
	{
		QuestManager->ReportSequenceTriggered(SequenceTag);
	}
}


void AQuestTrigger::SetTriggerEnabled(bool bEnabled)
{
	bIsEnabled = bEnabled;
}


void AQuestTrigger::Interact(ACharacter* InteractingPlayer)
{
	if (TriggerMode != EQuestTriggerMode::OnInteract)
	{
		return;
	}

	if (!bIsEnabled || (bOneShot && bHasTriggered))
	{
		return;
	}

	if (CheckPrerequisites())
	{
		ActivateTrigger(InteractingPlayer);
	}
}


int32 AQuestTrigger::GetTotalPlayerCount() const
{
	int32 Count = 0;
	for (TActorIterator<AHordeBaseCharacter> It(GetWorld()); It; ++It)
	{
		if (*It && !(*It)->GetIsDead())
		{
			Count++;
		}
	}
	return Count;
}
