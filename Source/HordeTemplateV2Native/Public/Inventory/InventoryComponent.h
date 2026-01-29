// Copyright (c) 2018 - 2026 Marc Fraedrich
// Horde Template V2 (Native) - A cooperative zombie survival game framework
// Licensed under the MIT License

#pragma once

/**
 * @file InventoryComponent.h
 * @brief Replicated inventory system with weapon slots and ammo management
 * @author Marc Fraedrich
 */

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include "Gameplay/GameplayStructures.h"
#include "InventoryComponent.generated.h"

// Forward decls
class ABaseFirearm;
class AInventoryBaseItem;


// === Events ===
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnActiveItemChanged, FString, ItemID, int32, Index, int32, LoadedAmmo);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class HORDETEMPLATEV2NATIVE_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UInventoryComponent();

	// --- Replication ---
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// Inventory accessor
	const TArray<FItem>& GetInventory();

	// Update ammo of the *active* item entry (not the world firearm)
	UFUNCTION(BlueprintCallable, Category="Inventory")
	void UpdateCurrentItemAmmo(int32 Ammo);

	// Recompute the simple replicated ammo count for the active item (avoid replicating whole item)
	UFUNCTION(BlueprintCallable, Category="Inventory")
	void RefreshCurrentAmmoForItem();

	// === Server RPCs ===
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerAddItem(const FString& ItemID, bool Custom, FItem CustomItem);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerDropItem(ABaseFirearm* Firearm);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerDropItemAtIndex(int32 IndexToDrop);

	UFUNCTION(Server, Reliable, WithValidation)
	void SwitchWeapon(EActiveType ItemType);

	UFUNCTION(Server, Reliable, WithValidation)
	void ScrollItems(bool ScrollUp);

	// === Queries / Helpers ===
	bool InventoryItemExists(FString ItemID, int32& Index, EActiveType& ItemType);
	FTransform CalculateDropLocation();
	int32 CountAmmo(FName AmmoType, int32& Index);                 // returns total, outputs last stack index (or -1)
	bool RemoveAmmoByType(FName AmmoType, int32 AmountToRemove);   // drains across stacks
	void FindItemByCategory(EActiveType IType, FItem& OutItem, int32& OutIndex);

	// Datatable lookup
	bool GetItemByID(FString ItemID, FItem& Item);

	// Event: inform UI/character when active item changes
	UPROPERTY(BlueprintAssignable, Category="Inventory")
	FOnActiveItemChanged OnActiveItemChanged;

protected:
	virtual void BeginPlay() override;

	// === RepNotifies ===
	UFUNCTION()
	void OnRep_ActiveItemIndex();

	UFUNCTION()
	void OnRep_AvailableAmmo();

protected:


	// Local inventory (server authoritative; replicated via your own approach)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory")
	TArray<FItem> Inventory;

public:
	// Replicated — which inventory slot is active
	UPROPERTY(ReplicatedUsing=OnRep_ActiveItemIndex, VisibleAnywhere, Category="Inventory")
	int32 ActiveItemIndex = 0;

	// Replicated — lightweight ammo info for HUD
	UPROPERTY(ReplicatedUsing=OnRep_AvailableAmmo, VisibleAnywhere, Category="Inventory")
	int32 AvailableAmmoForCurrentWeapon = 0;
	
	// Source of item definitions
	UPROPERTY(EditDefaultsOnly, Category="Inventory")
	UDataTable* DataTable = nullptr;

private:
	// Internal server-side add (no RPC used by server itself)
	void AddItem_Internal(const FItem& ItemToAdd, bool bSetActiveIfWeapon);
};
