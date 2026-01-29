// Copyright (c) 2018 - 2026 Marc Fraedrich
// Horde Template V2 (Native) - A cooperative zombie survival game framework
// Licensed under the MIT License

#pragma once

/**
 * @file QuestItem.h
 * @brief Collectible quest item actor with visual effects and interaction support
 * @author Marc Fraedrich
 */

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Inventory/InteractionInterface.h"
#include "QuestItem.generated.h"

class AHordeBaseCharacter;

/**
 * Quest Item - A collectible item for quest objectives
 */
UCLASS(ClassGroup = "Quest", Blueprintable)
class HORDETEMPLATEV2NATIVE_API AQuestItem : public AActor, public IInteractionInterface
{
	GENERATED_BODY()

public:
	AQuestItem();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// ==================== CONFIGURATION ====================

	/** Unique ID for this item type (matches quest objective TargetItemID) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Item")
		FName ItemID;

	/** Display name shown to players */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Item")
		FText DisplayName;

	/** Number of items this pickup represents */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Item")
		int32 ItemCount = 1;

	/** If true, item requires interaction to pick up. If false, auto-pickup on overlap */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Item")
		bool bRequireInteraction = false;

	/** If true, any player can pick up. If false, only specific player (for personal objectives) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Item")
		bool bAnyPlayerCanPickup = true;

	/** Quest that must be active to pick up this item */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Item|Prerequisites")
		FName RequiredActiveQuest;

	// ==================== VISUAL ====================

	/** Static mesh to display */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Item|Visual")
		UStaticMesh* ItemMesh;

	/** Material override */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Item|Visual")
		UMaterialInterface* ItemMaterial;

	/** Rotation speed for floating effect (degrees per second) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Item|Visual")
		float RotationSpeed = 90.f;

	/** Bobbing amplitude */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Item|Visual")
		float BobAmplitude = 10.f;

	/** Bobbing speed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Item|Visual")
		float BobSpeed = 2.f;

	/** Glow/highlight effect */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Item|Visual")
		bool bEnableGlow = true;

	// ==================== AUDIO ====================

	/** Sound played when item is picked up */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Item|Audio")
		USoundCue* PickupSound;

	// ==================== EVENTS ====================

	/** Called when item is picked up */
	UFUNCTION(BlueprintImplementableEvent, Category = "Quest Item|Events")
		void OnItemPickedUp(AHordeBaseCharacter* Player);

	// ==================== INTERACTION ====================

	/** Interaction info for the interaction system */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Item|Interaction")
		FInteractionInfo InteractionInfo;

	// ==================== FUNCTIONS ====================

	/** Try to pick up this item */
	UFUNCTION(BlueprintCallable, Category = "Quest Item")
		bool TryPickup(AHordeBaseCharacter* Player);

	/** Check if item can be picked up by this player */
	UFUNCTION(BlueprintPure, Category = "Quest Item")
		bool CanBePickedUp(AHordeBaseCharacter* Player) const;

	// ==================== INTERACTION INTERFACE ====================

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
		void Interact(AActor* InteractingOwner);
		virtual void Interact_Implementation(AActor* InteractingOwner) override;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
		FInteractionInfo GetInteractionInfo();
		virtual FInteractionInfo GetInteractionInfo_Implementation() override;

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleDefaultsOnly, Category = "Components")
		USphereComponent* PickupCollision;

	UPROPERTY(VisibleDefaultsOnly, Category = "Components")
		UStaticMeshComponent* MeshComponent;

	UPROPERTY(Replicated)
		bool bIsPickedUp = false;

	UPROPERTY()
		FVector InitialLocation;

	UPROPERTY()
		float BobTimer = 0.f;

	UFUNCTION()
		void OnPickupOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
			UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION(NetMulticast, Reliable)
		void Multicast_OnPickedUp();

	void ExecutePickup(AHordeBaseCharacter* Player);
};
