// InventoryComponent.cpp

#include "InventoryComponent.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "Components/PrimitiveComponent.h"

#include "Character/HordeBaseCharacter.h"
#include "Gameplay/HordeWorldSettings.h"
#include "InventoryBaseItem.h"
#include "BaseFirearm.h" // your firearm class header

UInventoryComponent::UInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;

	SetIsReplicatedByDefault(true);
}

void UInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Optimization: These values only needed by owner for HUD display
	DOREPLIFETIME_CONDITION(UInventoryComponent, ActiveItemIndex, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(UInventoryComponent, AvailableAmmoForCurrentWeapon, COND_OwnerOnly);
}

const TArray<FItem>& UInventoryComponent::GetInventory()
{
	return Inventory;
}

void UInventoryComponent::UpdateCurrentItemAmmo(int32 Ammo)
{
	if (Inventory.IsValidIndex(ActiveItemIndex))
	{
		Inventory[ActiveItemIndex].UpdateAmmo(Ammo);
	}
}

void UInventoryComponent::RefreshCurrentAmmoForItem()
{
	if (Inventory.IsValidIndex(ActiveItemIndex))
	{
		int32 TempIndex = -1;
		AvailableAmmoForCurrentWeapon = CountAmmo(Inventory[ActiveItemIndex].AmmoType, TempIndex);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("InventoryComponent: ActiveItemIndex invalid in RefreshCurrentAmmoForItem."));
	}
}

// ====== SERVER RPCs ======

void UInventoryComponent::ServerAddItem_Implementation(const FString& ItemID, bool Custom, FItem CustomItem)
{
	FItem FoundItm;
	if (!Custom)
	{
		if (!GetItemByID(ItemID, FoundItm))
		{
			UE_LOG(LogTemp, Warning, TEXT("InventoryComponent: Unable to find %s in DataTable."), *ItemID);
			return;
		}
	}
	const FItem& NewItem = Custom ? CustomItem : FoundItm;

	int32 ExistingItemIndex = INDEX_NONE;
	EActiveType ExistingActiveType = EActiveType::EActiveMisc;
	const bool bExists = InventoryItemExists(ItemID, ExistingItemIndex, ExistingActiveType);

	AHordeBaseCharacter* PLY = Cast<AHordeBaseCharacter>(GetOwner());

	if (bExists)
	{
		if (NewItem.Type != EActiveType::EActiveAmmo)
		{
			// If adding a weapon that already exists: drop existing one
			if (PLY)
			{
				ABaseFirearm* Cur = PLY->GetCurrentFirearm();
				if (Cur && ItemID == Cur->WeaponID)
				{
					ServerDropItem(Cur);
				}
				else if (Inventory.IsValidIndex(ExistingItemIndex))
				{
					ServerDropItemAtIndex(ExistingItemIndex);
				}
			}

			AddItem_Internal(NewItem, /*bSetActiveIfWeapon=*/true);
			RefreshCurrentAmmoForItem();
		}
		else
		{
			// Ammo stack: increment ammo in that stack
			if (Inventory.IsValidIndex(ExistingItemIndex))
			{
				const int32 NewAmmo = Inventory[ExistingItemIndex].DefaultLoadedAmmo + NewItem.DefaultLoadedAmmo;
				Inventory[ExistingItemIndex].UpdateAmmo(NewAmmo);
			}
			RefreshCurrentAmmoForItem();
		}
	}
	else
	{
		AddItem_Internal(NewItem, /*bSetActiveIfWeapon=*/NewItem.Type != EActiveType::EActiveAmmo);
		RefreshCurrentAmmoForItem();
	}
}

bool UInventoryComponent::ServerAddItem_Validate(const FString& ItemID, bool Custom, FItem CustomItem)
{
	return true;
}

void UInventoryComponent::ServerDropItem_Implementation(ABaseFirearm* Firearm)
{
	if (!Firearm) { return; }

	if (Inventory.IsValidIndex(ActiveItemIndex) &&
		Inventory[ActiveItemIndex].ItemID != "Item_Hands" &&
		Inventory[ActiveItemIndex].ItemID != "Hands")
	{
		FItem ItemToDrop = Inventory[ActiveItemIndex];

		// sync carried ammo into the dropped item
		ItemToDrop.UpdateAmmo(Firearm->LoadedAmmo);

		const FTransform TransformToSpawn = CalculateDropLocation();

		AInventoryBaseItem* Item = GetWorld()->SpawnActorDeferred<AInventoryBaseItem>(
			AInventoryBaseItem::StaticClass(),
			TransformToSpawn,
			GetOwner(),
			nullptr,
			ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);

		if (Item)
		{
			Item->ItemInfo = ItemToDrop;
			Item->Spawned = true;
			Item->ItemID = ItemToDrop.ItemID;

			Inventory.RemoveAt(ActiveItemIndex);
			// Clamp active index
			if (Inventory.Num() == 0)
			{
				ActiveItemIndex = 0;
			}
			else
			{
				ActiveItemIndex = FMath::Clamp(ActiveItemIndex, 0, Inventory.Num() - 1);
			}

			if (AHordeBaseCharacter* OwningPLY = Cast<AHordeBaseCharacter>(GetOwner()))
			{
				if (!OwningPLY->GetIsDead())
				{
					OnActiveItemChanged.Broadcast("Item_Hands", 0, 0);
				}
			}

			Item->FinishSpawning(TransformToSpawn);
		}
	}
}

bool UInventoryComponent::ServerDropItem_Validate(ABaseFirearm* Firearm)
{
	return true;
}

void UInventoryComponent::ServerDropItemAtIndex_Implementation(int32 IndexToDrop)
{
	if (Inventory.IsValidIndex(IndexToDrop) &&
		Inventory[IndexToDrop].ItemID != "Item_Hands" &&
		Inventory[IndexToDrop].ItemID != "Hands")
	{
		const FItem ItemToDrop = Inventory[IndexToDrop];
		const FTransform TransformToSpawn = CalculateDropLocation();

		AInventoryBaseItem* Item = GetWorld()->SpawnActorDeferred<AInventoryBaseItem>(
			AInventoryBaseItem::StaticClass(),
			TransformToSpawn,
			GetOwner(),
			nullptr,
			ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);

		if (Item)
		{
			Item->ItemInfo = ItemToDrop;
			Item->Spawned = true;
			Item->ItemID = ItemToDrop.ItemID;

			Inventory.RemoveAt(IndexToDrop);

			// Fix active index if it went out of bounds
			if (!Inventory.IsValidIndex(ActiveItemIndex) && Inventory.Num() > 0)
			{
				ActiveItemIndex = FMath::Clamp(ActiveItemIndex, 0, Inventory.Num() - 1);
				OnActiveItemChanged.Broadcast(Inventory[ActiveItemIndex].ItemID.ToString(), ActiveItemIndex, Inventory[ActiveItemIndex].DefaultLoadedAmmo);
			}

			Item->FinishSpawning(TransformToSpawn);
		}
	}
}

bool UInventoryComponent::ServerDropItemAtIndex_Validate(int32 IndexToDrop)
{
	return true;
}

void UInventoryComponent::SwitchWeapon_Implementation(EActiveType ItemType)
{
	AHordeBaseCharacter* PLY = Cast<AHordeBaseCharacter>(GetOwner());
	if (!PLY || PLY->Reloading || PLY->GetIsDead()) { return; }

	FItem NewItem;
	int32 NewIndex = -1;
	FindItemByCategory(ItemType, NewItem, NewIndex);

	if (NewIndex > -1 && NewIndex != ActiveItemIndex && Inventory.IsValidIndex(NewIndex))
	{
		ActiveItemIndex = NewIndex;
		OnActiveItemChanged.Broadcast(Inventory[NewIndex].ItemID.ToString(), NewIndex, Inventory[NewIndex].DefaultLoadedAmmo);
		RefreshCurrentAmmoForItem();
	}
}

bool UInventoryComponent::SwitchWeapon_Validate(EActiveType ItemType)
{
	return true;
}

void UInventoryComponent::ScrollItems_Implementation(bool ScrollUp)
{
	// FindNext/Last need a valid ActiveItemIndex; guard inside those functions
	if (ScrollUp)
	{
		// Next category
		AHordeBaseCharacter* PLY = Cast<AHordeBaseCharacter>(GetOwner());
		if (!PLY || PLY->Reloading || PLY->GetIsDead()) { return; }

		// Compute next type from current
		// Reuse GetAvailableCategories + current index
		const TArray<EActiveType> Cats = [&]()
		{
			TArray<EActiveType> Tmp;
			for (const FItem& Itm : Inventory)
			{
				if (Itm.Type != EActiveType::EActiveAmmo) { Tmp.AddUnique(Itm.Type); }
			}
			return Tmp;
		}();

		if (Cats.Num() == 0 || !Inventory.IsValidIndex(ActiveItemIndex)) { return; }

		const int32 Cur = Cats.Find(Inventory[ActiveItemIndex].Type);
		const EActiveType NextType = (Cur == INDEX_NONE) ? Cats[0] : ((Cur == Cats.Num() - 1) ? Cats[0] : Cats[Cur + 1]);
		SwitchWeapon(NextType);
	}
	else
	{
		// Previous category
		AHordeBaseCharacter* PLY = Cast<AHordeBaseCharacter>(GetOwner());
		if (!PLY || PLY->Reloading || PLY->GetIsDead()) { return; }

		const TArray<EActiveType> Cats = [&]()
		{
			TArray<EActiveType> Tmp;
			for (const FItem& Itm : Inventory)
			{
				if (Itm.Type != EActiveType::EActiveAmmo) { Tmp.AddUnique(Itm.Type); }
			}
			return Tmp;
		}();

		if (Cats.Num() == 0 || !Inventory.IsValidIndex(ActiveItemIndex)) { return; }

		const int32 Cur = Cats.Find(Inventory[ActiveItemIndex].Type);
		const EActiveType PrevType = (Cur == INDEX_NONE) ? Cats.Last() : ((Cur == 0) ? Cats.Last() : Cats[Cur - 1]);
		SwitchWeapon(PrevType);
	}
}

bool UInventoryComponent::ScrollItems_Validate(bool ScrollUp)
{
	return true;
}

// ====== Queries / Helpers ======

bool UInventoryComponent::InventoryItemExists(FString ItemID, int32& Index, EActiveType& ItemType)
{
	int32 TempIndex = -1;
	EActiveType TempActive = EActiveType::EActiveRifle;
	bool TempSuccess = false;

	for (const FItem& Item : Inventory)
	{
		++TempIndex;
		if (Item.ItemID == FName(*ItemID))
		{
			TempActive = Item.Type;
			TempSuccess = true;
			break;
		}
	}
	Index = TempIndex;
	ItemType = TempActive;
	return TempSuccess;
}

FTransform UInventoryComponent::CalculateDropLocation()
{
	FTransform TempTransform;

	AHordeBaseCharacter* PLY = Cast<AHordeBaseCharacter>(GetOwner());
	if (!PLY) { return TempTransform; }

	FVector EyeLocation;
	FRotator EyeRotation;
	PLY->GetActorEyesViewPoint(EyeLocation, EyeRotation);

	FCollisionQueryParams TraceParams(FName(TEXT("ItemTrace")), true, GetOwner());
	TraceParams.bTraceComplex = true;
	TraceParams.bReturnPhysicalMaterial = false;
	TraceParams.AddIgnoredActor(GetOwner());

	// Ignore current firearm if valid
	if (AHordeBaseCharacter* HC = Cast<AHordeBaseCharacter>(GetOwner()))
	{
		if (ABaseFirearm* Cur = HC->GetCurrentFirearm())
		{
			TraceParams.AddIgnoredActor(Cur);
		}
	}

	FHitResult HitResult(ForceInit);
	GetWorld()->LineTraceSingleByChannel(
		HitResult,
		EyeLocation,
		EyeLocation + (EyeRotation.Vector() * 200.f),
		ECC_Visibility,
		TraceParams
	);

	if (HitResult.bBlockingHit)
	{
		TempTransform.SetLocation(HitResult.ImpactPoint);
	}
	else
	{
		TempTransform.SetLocation(HitResult.TraceEnd);
	}

	TempTransform.SetRotation(FRotator(0.f, FMath::FRandRange(0.f, 360.f), 0.f).Quaternion());
	return TempTransform;
}

int32 UInventoryComponent::CountAmmo(FName AmmoType, int32& Index)
{
	int32 Total = 0;
	int32 LastAmmoIndex = -1;

	for (int32 i = 0; i < Inventory.Num(); ++i)
	{
		const FItem& Itm = Inventory[i];
		if (Itm.ItemID == AmmoType)
		{
			Total += Itm.DefaultLoadedAmmo;
			LastAmmoIndex = i;
		}
	}

	Index = LastAmmoIndex; // -1 if none
	return Total;
}

bool UInventoryComponent::RemoveAmmoByType(FName AmmoType, int32 AmountToRemove)
{
	// First, ensure we have enough across stacks
	int32 Total = 0;
	for (const FItem& Itm : Inventory)
	{
		if (Itm.ItemID == AmmoType) { Total += Itm.DefaultLoadedAmmo; }
	}
	if (Total < AmountToRemove) { return false; }

	// Drain from the end (policy choice)
	for (int32 i = Inventory.Num() - 1; i >= 0 && AmountToRemove > 0; --i)
	{
		FItem& Itm = Inventory[i];
		if (Itm.ItemID != AmmoType) { continue; }

		const int32 Take = FMath::Min(Itm.DefaultLoadedAmmo, AmountToRemove);
		Itm.UpdateAmmo(Itm.DefaultLoadedAmmo - Take);
		AmountToRemove -= Take;

		if (Itm.DefaultLoadedAmmo <= 0)
		{
			Inventory.RemoveAt(i);

			// Fix active index if needed
			if (Inventory.Num() == 0)
			{
				ActiveItemIndex = 0;
			}
			else if (!Inventory.IsValidIndex(ActiveItemIndex))
			{
				ActiveItemIndex = FMath::Clamp(ActiveItemIndex, 0, Inventory.Num() - 1);
			}
		}
	}

	RefreshCurrentAmmoForItem();
	return true;
}

void UInventoryComponent::FindItemByCategory(EActiveType IType, FItem& OutItem, int32& OutIndex)
{
	int32 LIndex = -1;
	int32 FoundIndex = -1;
	FItem  FoundItem;

	for (const FItem& Itm : Inventory)
	{
		++LIndex;
		if (Itm.Type == IType && LIndex != ActiveItemIndex && Itm.ItemID != "Item_Hands")
		{
			FoundItem  = Itm;
			FoundIndex = LIndex;
			break;
		}
	}

	OutItem  = FoundItem;
	OutIndex = FoundIndex; // -1 if not found
}

// ====== Lifecycle ======

void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	// Only the server seeds starting items
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		if (const UWorld* World = GetWorld())
		{
			if (AHordeWorldSettings* WorldSettings = Cast<AHordeWorldSettings>(World->GetWorldSettings(true)))
			{
				for (const FName& StartingItem : WorldSettings->StartingItems)
				{
					FItem Item;
					if (GetItemByID(StartingItem.ToString(), Item))
					{
						UE_LOG(LogTemp, Log, TEXT("InventoryComponent: Starting item %s found in DataTable."), *StartingItem.ToString());
						AddItem_Internal(Item, /*bSetActiveIfWeapon=*/ Item.Type != EActiveType::EActiveAmmo);
					}
					else
					{
						UE_LOG(LogTemp, Warning, TEXT("InventoryComponent: Starting item %s not found in DataTable."), *StartingItem.ToString());
					}
				}
				RefreshCurrentAmmoForItem();
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("InventoryComponent: WorldSettings is null"));
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("InventoryComponent: World is null"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("InventoryComponent: Only the server can seed starting items or owner is invalid.."));
	}
}

// ====== RepNotifies ======

void UInventoryComponent::OnRep_ActiveItemIndex()
{
	if (Inventory.IsValidIndex(ActiveItemIndex))
	{
		OnActiveItemChanged.Broadcast(
			Inventory[ActiveItemIndex].ItemID.ToString(),
			ActiveItemIndex,
			Inventory[ActiveItemIndex].DefaultLoadedAmmo
		);
	}
}

void UInventoryComponent::OnRep_AvailableAmmo()
{
	// Hook for HUD/widgets to refresh displayed ammo
	// (You can broadcast a specific ammo-changed event if you have one)
}

// ====== Datatable ======

bool UInventoryComponent::GetItemByID(FString ItemID, FItem& Item)
{
	if (!DataTable)
	{
		UE_LOG(LogTemp, Warning, TEXT("InventoryComponent: DataTable is null"));
		return false;
	}

	FItem* ItemFromRow = DataTable->FindRow<FItem>(FName(*ItemID), TEXT("Inventory Component - GetItemByID"), false);
	if (!ItemFromRow) { return false; }

	if (ItemFromRow->ItemID != "None")
	{
		Item = *ItemFromRow;
		return true;
	}

	return false;
}

// ====== Internal ======

void UInventoryComponent::AddItem_Internal(const FItem& ItemToAdd, bool bSetActiveIfWeapon)
{
	const int32 NewIndex = Inventory.Add(ItemToAdd);

	if (bSetActiveIfWeapon)
	{
		ActiveItemIndex = NewIndex;
		OnActiveItemChanged.Broadcast(Inventory[NewIndex].ItemID.ToString(), NewIndex, Inventory[NewIndex].DefaultLoadedAmmo);
	}
}
