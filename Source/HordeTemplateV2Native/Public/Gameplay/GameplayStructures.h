// Copyright (c) 2018 - 2026 Marc Fraedrich
// Horde Template V2 (Native) - A cooperative zombie survival game framework
// Licensed under the MIT License

#pragma once

/**
 * @file GameplayStructures.h
 * @brief Core gameplay data structures including items, weapons, fire modes, and player animation data
 * @author Marc Fraedrich
 */

#include "Engine/DataTable.h"
#include "Weapons/BaseFirearm.h"
#include "Inventory/InteractionInterface.h"
#include "Projectiles/BaseProjectile.h"
#include "GameplayStructures.generated.h"

USTRUCT(BlueprintType)
struct FPlayerAnimationData {
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
		UAnimMontage* CharacterReloadAnimation = nullptr;

	FPlayerAnimationData() {}

};

UENUM(BlueprintType)
enum class EActiveType : uint8
{
	EActiveRifle    UMETA(DisplayName="Rifle"),
	EActivePistol   UMETA(DisplayName="Pistol"),
	EActiveMed      UMETA(DisplayName="Medical"),
	EActiveMisc     UMETA(DisplayName="Misc"),
	EActiveAmmo     UMETA(DisplayName="Ammo")
};

UENUM(BlueprintType)
enum class EFireMode : uint8
{
	EFireModeSingle UMETA(DisplayName = "Single Fire"),
	EFireModeBurst UMETA(DisplayName = "Burst Fire"),
	EFireModeFull UMETA(DisplayName = "Full Automatic")
};

USTRUCT(BlueprintType)
struct FItem : public FTableRowBase
{
	GENERATED_USTRUCT_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Item")
		FName ItemID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
		EActiveType Type;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
		FString ItemName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Firearm Settings")
		int32 DefaultLoadedAmmo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Firearm Settings")
		int32 MaximumLoadedAmmo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Firearm Settings")
		int32 MaxAmount;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Firearm Settings")
		TSubclassOf<ABaseFirearm> FirearmClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Firearm Settings")
		TSubclassOf<UObject> VisualRecoilClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Firearm Settings")
		FName AttachmentPoint;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Firearm Settings")
		TSubclassOf<ABaseProjectile> ProjectileClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Firearm Settings")
		FName AmmoType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Firearm Settings")
		int32 AnimType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Firearm Settings")
		float FireRate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Firearm Settings")
		float BurstFireAmount; 

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Firearm Settings")
		EFireMode DefaultFireMode;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Firearm Settings")
		TArray<EFireMode> FireModes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup")
		UStaticMesh* WorldModel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup")
		FInteractionInfo InteractionInfo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
		FPlayerAnimationData PlayerAnimationData;

	FItem()
	{
		ItemID = "None";
		Type = EActiveType::EActiveRifle;
		ItemName = "None";
		DefaultLoadedAmmo = 0;
		MaximumLoadedAmmo = 0;
		MaxAmount = 1;
		FirearmClass = nullptr;
		AttachmentPoint = FName(TEXT("None"));
		ProjectileClass = nullptr;
		AmmoType = FName(TEXT("None"));
		FireRate = 0.f;
		AnimType = 0;
		BurstFireAmount = 0.f;
		FireModes.Add(EFireMode::EFireModeSingle);
		WorldModel = nullptr;
		DefaultFireMode = EFireMode::EFireModeSingle;
		VisualRecoilClass = nullptr;
	}

	void UpdateAmmo(int32 Ammo) {
		DefaultLoadedAmmo = Ammo;
	}
};