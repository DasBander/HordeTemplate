// Copyright (c) 2018 - 2026 Marc Fraedrich
// Horde Template V2 (Native) - A cooperative zombie survival game framework
// Licensed under the MIT License

#pragma once

/**
 * @file QuestTrigger.h
 * @brief Volume-based trigger actor for starting quests and completing objectives
 * @author Marc Fraedrich
 */

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "Components/BillboardComponent.h"
#include "QuestTypes.h"
#include "QuestTrigger.generated.h"

class UQuestManager;

/**
 * Trigger activation mode
 */
UENUM(BlueprintType)
enum class EQuestTriggerMode : uint8
{
	OnOverlap,			// Triggered when player enters
	OnInteract,			// Triggered when player interacts
	OnExternalCall,		// Only triggered via Blueprint/code
	OnAllPlayersInside	// All players must be inside
};

/**
 * Quest Trigger - An actor that can start quests, complete objectives, or fire events
 */
UCLASS(ClassGroup = "Quest", Blueprintable)
class HORDETEMPLATEV2NATIVE_API AQuestTrigger : public AActor
{
	GENERATED_BODY()

public:
	AQuestTrigger();

	// ==================== CONFIGURATION ====================

	/** How this trigger is activated */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Trigger")
		EQuestTriggerMode TriggerMode = EQuestTriggerMode::OnOverlap;

	/** Quest ID to start when triggered */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Trigger|Quest")
		FName QuestToStart;

	/** Quest trigger tag (alternative to specific quest ID) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Trigger|Quest")
		FName QuestTriggerTag;

	/** Objective to complete when triggered */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Trigger|Objective")
		FName QuestIDForObjective;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Trigger|Objective")
		FName ObjectiveToComplete;

	/** Location tag for location objectives */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Trigger|Location")
		FName LocationTag;

	/** Interaction tag for interact objectives */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Trigger|Interaction")
		FName InteractionTag;

	/** Sequence tag for triggering sequences */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Trigger|Sequence")
		FName SequenceTag;

	/** If true, trigger can only be activated once */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Trigger")
		bool bOneShot = true;

	/** If true, trigger is initially enabled */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Trigger")
		bool bStartEnabled = true;

	/** Required quest to be active before this trigger works */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Trigger|Prerequisites")
		FName RequiredActiveQuest;

	/** Required quest to be completed before this trigger works */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Trigger|Prerequisites")
		FName RequiredCompletedQuest;

	// ==================== VISUAL ====================

	/** Show debug visualization */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Trigger|Debug")
		bool bShowDebug = false;

	// ==================== EVENTS ====================

	/** Called when trigger is activated */
	UFUNCTION(BlueprintImplementableEvent, Category = "Quest Trigger|Events")
		void OnTriggerActivated(ACharacter* ActivatingPlayer);

	// ==================== FUNCTIONS ====================

	/** Manually activate the trigger */
	UFUNCTION(BlueprintCallable, Category = "Quest Trigger")
		void ActivateTrigger(ACharacter* ActivatingPlayer = nullptr);

	/** Enable/disable the trigger */
	UFUNCTION(BlueprintCallable, Category = "Quest Trigger")
		void SetTriggerEnabled(bool bEnabled);

	/** Check if trigger is enabled */
	UFUNCTION(BlueprintPure, Category = "Quest Trigger")
		bool IsTriggerEnabled() const { return bIsEnabled; }

	/** Interact with this trigger (for OnInteract mode) */
	UFUNCTION(BlueprintCallable, Category = "Quest Trigger")
		void Interact(ACharacter* InteractingPlayer);

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleDefaultsOnly, Category = "Components")
		UBoxComponent* TriggerVolume;

	UPROPERTY(VisibleDefaultsOnly, Category = "Components")
		UBillboardComponent* EditorIcon;

	UPROPERTY()
		bool bIsEnabled = true;

	UPROPERTY()
		bool bHasTriggered = false;

	UPROPERTY()
		TArray<ACharacter*> PlayersInTrigger;

	UFUNCTION()
		void OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
			UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
		void OnTriggerEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
			UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	bool CheckPrerequisites() const;
	void ExecuteTriggerActions(ACharacter* ActivatingPlayer);
	int32 GetTotalPlayerCount() const;
};
