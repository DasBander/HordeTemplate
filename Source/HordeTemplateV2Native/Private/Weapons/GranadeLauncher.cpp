// Copyright (c) 2018 - 2026 Marc Fraedrich
// Horde Template V2 (Native) - A cooperative zombie survival game framework
// Licensed under the MIT License

#include "GranadeLauncher.h"

/**
 * @file GranadeLauncher.cpp
 * @brief Implementation of grenade launcher with custom mesh and disabled muzzle flash
 * @author Marc Fraedrich
 */
#include "ConstructorHelpers.h"
#include "Engine/SkeletalMesh.h"
#include "Components/SkeletalMeshComponent.h"
#include "Materials/MaterialInstanceConstant.h"

/**
 * Constructor for AGranadeLauncher
 *
 * @param
 * @return
 */
AGranadeLauncher::AGranadeLauncher()
{
	//Nullptr the Muzzleflash as we don't want a muzzle flash on our weapon.
	MuzzleFlash->SetTemplate(nullptr);

	//Setting the Mesh
	const ConstructorHelpers::FObjectFinder<USkeletalMesh> LauncherMeshAsset(TEXT("SkeletalMesh'/Game/HordeTemplateBP/Assets/Meshes/Weapons/NadeLauncher/SK_GranadeLauncher.SK_GranadeLauncher'"));
	if (LauncherMeshAsset.Succeeded())
	{
		Weapon->SetSkeletalMesh(LauncherMeshAsset.Object);
		Weapon->SetRelativeLocation(FVector(0.f, 0.f, 0.f));
		Weapon->SetRelativeRotation(FRotator(0.f, 0.f, 0.f).Quaternion());
		Weapon->SetRelativeScale3D(FVector(1.f, 1.f, 1.f));
	}
	const ConstructorHelpers::FObjectFinder<UMaterialInstanceConstant> LauncherMaterialAsset(TEXT("MaterialInstanceConstant'/Game/HordeTemplateBP/Assets/Meshes/Weapons/NadeLauncher/SM_GranadeLaunch_Mat_Inst.SM_GranadeLaunch_Mat_Inst'"));
	if (LauncherMaterialAsset.Succeeded())
	{
		Weapon->SetMaterial(0, LauncherMaterialAsset.Object);
	}
}
