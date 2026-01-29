// Copyright (c) 2018 - 2026 Marc Fraedrich
// Horde Template V2 (Native) - A cooperative zombie survival game framework
// Licensed under the MIT License

#pragma once

/**
 * @file InventoryBaseItem.h
 * @brief Base actor class for world-placed interactable inventory items
 * @author Marc Fraedrich
 */

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "InteractionInterface.h"
#include "Gameplay/GameplayStructures.h"
#include "InventoryBaseItem.generated.h"

UCLASS()
class HORDETEMPLATEV2NATIVE_API AInventoryBaseItem : public AActor, public IInteractionInterface
{
	GENERATED_BODY()
	
public:	
	AInventoryBaseItem();

	// Fixed: Added ReplicatedUsing to properly setup mesh on clients after replication
	UPROPERTY(EditAnywhere, ReplicatedUsing=OnRep_ItemInfo, BlueprintReadWrite, Category = "Inventory Item")
		FItem ItemInfo;

	UPROPERTY(EditAnywhere, Replicated, BlueprintReadWrite, Category = "Inventory Item")
		FName ItemID;

	UFUNCTION()
		void OnRep_ItemInfo();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory Item")
		bool Spawned = false;

	


protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Firearm")
		UStaticMeshComponent* WorldMesh;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	void Interact(AActor* InteractingOwner);
	virtual void Interact_Implementation(AActor* InteractingOwner) override;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
		FInteractionInfo GetInteractionInfo();
	virtual FInteractionInfo GetInteractionInfo_Implementation() override;

	void PopInfo();

	virtual void OnConstruction(const FTransform& Transform) override;

};
