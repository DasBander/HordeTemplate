// Copyright (c) 2018 - 2026 Marc Fraedrich
// Horde Template V2 (Native) - A cooperative zombie survival game framework
// Licensed under the MIT License

#include "InventoryBaseItem.h"
#include "InventoryComponent.h"
#include "InventoryHelpers.h"

/**
 * @file InventoryBaseItem.cpp
 * @brief Implementation of interactable world items with replication support
 * @author Marc Fraedrich
 */

/**
 * Constructor for InventoryBaseItem
 *
 * @param
 * @return
 */
AInventoryBaseItem::AInventoryBaseItem()
{
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;
	SetReplicates(true);


	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root Component"));

	WorldMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WorldMesh"));
	WorldMesh->SetupAttachment(RootComponent);

}

/** ( Overridden )
 * Define Replicated Props
 *
 * @param
 * @output Out Lifetime Props
 * @return void
 */
void AInventoryBaseItem::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Fixed: Use DOREPLIFETIME_CONDITION with notify for proper client-side mesh setup
	DOREPLIFETIME(AInventoryBaseItem, ItemInfo);
	DOREPLIFETIME(AInventoryBaseItem, ItemID);
}

/**
 * OnRep for ItemInfo - Called on clients when ItemInfo is replicated.
 * Sets up the mesh after replication is complete.
 *
 * @param
 * @return void
 */
void AInventoryBaseItem::OnRep_ItemInfo()
{
	// Setup mesh on client after replication
	if (ItemInfo.WorldModel)
	{
		WorldMesh->SetStaticMesh(ItemInfo.WorldModel);
		if (Spawned)
		{
			WorldMesh->SetSimulatePhysics(true);
		}
	}
}

/** ( Virtual; Overridden )
 * Populate Item with Item Info.
 *
 * @param
 * @return
 */
void AInventoryBaseItem::BeginPlay()
{
	Super::BeginPlay();
	
	PopInfo();
}

/** (Interface)
 * On Interaction with Actor add this Item to his Inventory and set lifespan to 0.1f.
 *
 * @param Interacting Character / Actor.
 * @return void
 */
void AInventoryBaseItem::Interact_Implementation(AActor* InteractingOwner)
{
	UInventoryComponent* PlayerInventory = InteractingOwner->FindComponentByClass<UInventoryComponent>();
	if (PlayerInventory)
	{
		PlayerInventory->ServerAddItem(ItemID.ToString(), Spawned, ItemInfo);
		SetLifeSpan(0.1f);
	}
}

/** ( Interface )
 * Return Items Interaction Info.
 *
 * @param
 * @return Interaction Info.
 */
FInteractionInfo AInventoryBaseItem::GetInteractionInfo_Implementation()
{
	return ItemInfo.InteractionInfo;
}

/**
 * Sets Static Mesh and Simulates Physics on Item.
 *
 * @param
 * @return void
 */
void AInventoryBaseItem::PopInfo()
{
	if (ItemID.ToString() != "None" && !Spawned)
	{
		ItemInfo = UInventoryHelpers::FindItemByID(ItemID);
		if (ItemInfo.WorldModel)
		{
			WorldMesh->SetStaticMesh(ItemInfo.WorldModel);
		}
	}
	else {
		if (ItemInfo.WorldModel)
		{
			WorldMesh->SetStaticMesh(ItemInfo.WorldModel);
		}
		WorldMesh->SetSimulatePhysics(true);

	}
}

/** ( Virtual; Overridden )
 * Updates Item Mesh World Model.
 *
 * @param Item Location
 * @return void
 */
void AInventoryBaseItem::OnConstruction(const FTransform& Transform)
{
	PopInfo();
}

