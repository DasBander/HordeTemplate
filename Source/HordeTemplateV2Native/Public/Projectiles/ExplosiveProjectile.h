// Copyright (c) 2018 - 2026 Marc Fraedrich
// Horde Template V2 (Native) - A cooperative zombie survival game framework
// Licensed under the MIT License

#pragma once

/**
 * @file ExplosiveProjectile.h
 * @brief Explosive projectile with radial damage falloff and explosion visual/audio effects
 * @author Marc Fraedrich
 */

#include "CoreMinimal.h"
#include "Projectiles/BaseProjectile.h"
#include "ExplosiveProjectile.generated.h"

/**
 * 
 */
UCLASS()
class HORDETEMPLATEV2NATIVE_API AExplosiveProjectile : public ABaseProjectile
{
	GENERATED_BODY()
	
public:

	AExplosiveProjectile();

	UFUNCTION()
		void OnProjectileImpact(const FHitResult& ImpactResult, const FVector& ImpactVelocity);

	UFUNCTION(NetMulticast, WithValidation, Reliable, Category = "FX")
		void PlayWorldFX(FVector Epicenter);
};
