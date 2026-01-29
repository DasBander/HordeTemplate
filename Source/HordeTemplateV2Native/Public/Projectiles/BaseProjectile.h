// Copyright (c) 2018 - 2026 Marc Fraedrich
// Horde Template V2 (Native) - A cooperative zombie survival game framework
// Licensed under the MIT License

#pragma once

/**
 * @file BaseProjectile.h
 * @brief Replicated projectile with collision detection, damage application, and impact effects
 * @author Marc Fraedrich
 */

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "FX/Impact/BaseImpact.h"
#include "BaseProjectile.generated.h"

UCLASS()
class HORDETEMPLATEV2NATIVE_API ABaseProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABaseProjectile();

protected:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile")
		class UProjectileMovementComponent* ProjectileMovement;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile")
		class USphereComponent* CollisionSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile")
		class UStaticMeshComponent* TracerMesh;
	
	UPROPERTY()
		int32 ImpactCounter = 0;

	UPROPERTY(EditDefaultsOnly, Category="Projectile")
	float Damage = 25.f;

	UFUNCTION()
		void OnProjectileStop(const FHitResult& ImpactResult);

	UFUNCTION()
		void OnProjectileBounce(const FHitResult& ImpactResult, const FVector& ImpactVelocity);

	UFUNCTION(NetMulticast, WithValidation, Unreliable, Category = "FX")
		void SpawnImpactFX(FVector ImpactLocation, FQuat ImpactRotation, TSubclassOf<ABaseImpact> ImpactClass);
	
};
