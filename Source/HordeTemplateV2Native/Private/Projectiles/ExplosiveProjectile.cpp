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
#include "Components/PointLightComponent.h"
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
 * Plays Explosion Particle and Sound. Also plays global camera shake and force feedback.
 *
 * @param Impact Epicenter
 * @return void
 */
void AExplosiveProjectile::PlayWorldFX_Implementation(FVector Epicenter)
{
	// Sound effect
	USoundCue* ExpSound = ObjectFromPath<USoundCue>(TEXT("SoundCue'/Game/HordeTemplateBP/Assets/Sounds/A_GranadeExplosion.A_GranadeExplosion'"));
	if (ExpSound)
	{
		UGameplayStatics::SpawnSoundAtLocation(GetWorld(), ExpSound, Epicenter);
	}

	// Particle effect
	UParticleSystem* ExpEmitter = ObjectFromPath<UParticleSystem>(TEXT("ParticleSystem'/Game/HordeTemplateBP/Assets/Effects/P_Explosion.P_Explosion'"));
	if (ExpEmitter)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExpEmitter, Epicenter);
	}

	// AAA-quality camera shake using Perlin noise pattern
	UGameplayStatics::PlayWorldCameraShake(GetWorld(), UCameraShake_Explosion::StaticClass(), Epicenter, 500.f, 3000.f, 1.f, false);

	// Force feedback (controller rumble) for nearby players
	const float MaxFeedbackDistance = 3000.f;
	APlayerController* LocalPC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (LocalPC && LocalPC->GetPawn())
	{
		float Distance = FVector::Dist(LocalPC->GetPawn()->GetActorLocation(), Epicenter);
		if (Distance < MaxFeedbackDistance)
		{
			// Scale intensity based on distance (closer = stronger)
			float Intensity = 1.0f - (Distance / MaxFeedbackDistance);
			Intensity = FMath::Clamp(Intensity, 0.1f, 1.0f);

			// Play force feedback
			LocalPC->PlayDynamicForceFeedback(
				Intensity,          // Intensity
				0.3f,               // Duration
				true,               // Affects left large
				true,               // Affects left small
				true,               // Affects right large
				true,               // Affects right small
				EDynamicForceFeedbackAction::Start
			);
		}
	}

	// Spawn a brief point light for visual flash
	if (UWorld* World = GetWorld())
	{
		// Use a simple approach - spawn light component attached to nothing, it will auto-destroy
		UPointLightComponent* FlashLight = NewObject<UPointLightComponent>(World);
		if (FlashLight)
		{
			FlashLight->SetWorldLocation(Epicenter);
			FlashLight->SetIntensity(50000.f);
			FlashLight->SetLightColor(FLinearColor(1.0f, 0.7f, 0.3f)); // Orange-ish
			FlashLight->SetAttenuationRadius(2000.f);
			FlashLight->RegisterComponent();

			// Fade out the light over time
			FTimerHandle LightFadeTimer;
			FTimerDelegate LightFadeDelegate;
			LightFadeDelegate.BindLambda([FlashLight]()
			{
				if (FlashLight && FlashLight->IsValidLowLevel())
				{
					FlashLight->DestroyComponent();
				}
			});
			World->GetTimerManager().SetTimer(LightFadeTimer, LightFadeDelegate, 0.15f, false);
		}
	}
}

bool AExplosiveProjectile::PlayWorldFX_Validate(FVector Epicenter)
{
	return true;
}
