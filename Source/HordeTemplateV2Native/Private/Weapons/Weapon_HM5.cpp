// Copyright (c) 2018 - 2026 Marc Fraedrich
// Horde Template V2 (Native) - A cooperative zombie survival game framework
// Licensed under the MIT License

#include "Weapon_HM5.h"

/**
 * @file Weapon_HM5.cpp
 * @brief Implementation of HM5 assault rifle with mesh, muzzle flash, and sound setup
 * @author Marc Fraedrich
 */
#include "ConstructorHelpers.h"

/**
 * Constructor for AWeapon_HM5
 *
 * @param
 * @return
 */
AWeapon_HM5::AWeapon_HM5()
{
	const ConstructorHelpers::FObjectFinder<USkeletalMesh> WeaponMeshAsset(TEXT("SkeletalMesh'/Game/HordeTemplateBP/Assets/Meshes/Weapons/HTM4/SK_HTM4.SK_HTM4'"));
	if (WeaponMeshAsset.Succeeded())
	{
		Weapon->SetSkeletalMesh(WeaponMeshAsset.Object);
		Weapon->SetRelativeRotation(FRotator({ 0.f, -180.f, 0.f }).Quaternion());
		Weapon->SetRelativeLocation(FVector(-1.999965f, 5.000014f, -3.0f));
	}
	const ConstructorHelpers::FObjectFinder<UParticleSystem> MuzzleFlashAsset(TEXT("ParticleSystem'/Game/HordeTemplateBP/Assets/Effects/ParticleSystems/Weapons/AssaultRifle/Muzzle/P_AssaultRifle_MF.P_AssaultRifle_MF'"));
	if (MuzzleFlashAsset.Succeeded())
	{
		MuzzleFlash->SetTemplate(MuzzleFlashAsset.Object);
		MuzzleFlash->SetRelativeLocation(FVector(-0.372549f, 48.901855f, 5.627451));
		MuzzleFlash->SetRelativeRotation(FRotator(0.f, 90.f, 0.f));
	}
	const ConstructorHelpers::FObjectFinder<USoundCue> WeaponFireSound(TEXT("SoundCue'/Game/HordeTemplateBP/Assets/Sounds/A_rifle_shot_single.A_rifle_shot_single'"));
	if (WeaponFireSound.Succeeded())
	{
		WeaponSound->SetSound(WeaponFireSound.Object);
	}
}
