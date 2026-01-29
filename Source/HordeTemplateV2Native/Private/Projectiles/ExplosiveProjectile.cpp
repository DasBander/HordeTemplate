// Copyright (c) 2018 - 2026 Marc Fraedrich
// Horde Template V2 (Native) - A cooperative zombie survival game framework
// Licensed under the MIT License

#include "ExplosiveProjectile.h"

/**
 * @file ExplosiveProjectile.cpp
 * @brief Implementation of explosive projectile with radial damage and world FX on impact
 * @author Marc Fraedrich
 */
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Sound/SoundCue.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "HordeTemplateV2Native.h"
#include "Materials/MaterialInstanceConstant.h"
#include "FX/Camera/CameraShake_Explosion.h"
#include "UObject/ConstructorHelpers.h"

/**
 * Constructor for AExplosiveProjectile
 *
 * @param
 * @return
 */
AExplosiveProjectile::AExplosiveProjectile()
{
	TracerMesh->SetRelativeLocation(FVector(-16.f, 0.f, 0.f));
	TracerMesh->SetRelativeRotation(FRotator(0.f, 90.f, 0.f).Quaternion());
	TracerMesh->SetRelativeScale3D(FVector(1.f, 1.f, 1.f));

	const ConstructorHelpers::FObjectFinder<UStaticMesh> ExplosiveMesh(TEXT("StaticMesh'/Game/HordeTemplateBP/Assets/Meshes/Misc/SM_ExplosiveRound.SM_ExplosiveRound'"));
	if (ExplosiveMesh.Succeeded())
	{
		TracerMesh->SetStaticMesh(ExplosiveMesh.Object);
	}

	const ConstructorHelpers::FObjectFinder<UMaterialInstanceConstant> ExplosiveRoundMatAsset(TEXT("MaterialInstanceConstant'/Game/HordeTemplateBP/Assets/Materials/M_ExplosiveRound_Inst.M_ExplosiveRound_Inst'"));
	if (ExplosiveRoundMatAsset.Succeeded())
	{
		TracerMesh->SetMaterial(0, ExplosiveRoundMatAsset.Object);
	}

	ProjectileMovement->InitialSpeed = 5000.f;
	ProjectileMovement->MaxSpeed = 15000.f;
	ProjectileMovement->ProjectileGravityScale = 2.f;

	ProjectileMovement->OnProjectileBounce.AddDynamic(this, &AExplosiveProjectile::OnProjectileImpact);
}

/**
 * Apply Radial Damage with Falloff on Impact Location and Play Explosion FX.
 *
 * @param Impact Hit Result and the Impact Velocity.
 * @return void
 */
void AExplosiveProjectile::OnProjectileImpact(const FHitResult& ImpactResult, const FVector& ImpactVelocity)
{
	if (HasAuthority())
	{
		PlayWorldFX(ImpactResult.ImpactPoint);
		// Fixed: Cache and null-check GetOwner() before using it
		AActor* OwnerActor = GetOwner();
		TArray<AActor*> IgnoredDamageActors;
		if (OwnerActor)
		{
			IgnoredDamageActors.Add(OwnerActor);
		}
		UGameplayStatics::ApplyRadialDamageWithFalloff(GetWorld(), 150.f, 100.f, ImpactResult.ImpactPoint, 800.f, 1600.f, 1.f, nullptr, IgnoredDamageActors, OwnerActor, OwnerActor ? OwnerActor->GetInstigatorController() : nullptr, ECC_Visibility);
	}
}

/** ( Multicast )
 * Plays Explosion Particle and Sound. Also plays global camera shake.
 *
 * @param Impact Epicenter
 * @return void
 */
void AExplosiveProjectile::PlayWorldFX_Implementation(FVector Epicenter)
{
	USoundCue* ExpSound = ObjectFromPath<USoundCue>(TEXT("SoundCue'/Game/HordeTemplateBP/Assets/Sounds/A_GranadeExplosion.A_GranadeExplosion'"));
	if (ExpSound)
	{
		UGameplayStatics::SpawnSoundAtLocation(GetWorld(), ExpSound, Epicenter);
	}
	UParticleSystem* ExpEmitter = ObjectFromPath<UParticleSystem>(TEXT("ParticleSystem'/Game/HordeTemplateBP/Assets/Effects/P_Explosion.P_Explosion'"));
	if (ExpEmitter)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExpEmitter, Epicenter);
	}


	UGameplayStatics::PlayWorldCameraShake(GetWorld(), UCameraShake_Explosion::StaticClass(), Epicenter, 0.f, 8000.f, 1.f, true);
}

bool AExplosiveProjectile::PlayWorldFX_Validate(FVector Epicenter)
{
	return true;
}
