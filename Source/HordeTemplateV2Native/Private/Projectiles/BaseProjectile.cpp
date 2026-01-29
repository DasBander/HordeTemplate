// Copyright (c) 2018 - 2026 Marc Fraedrich
// Horde Template V2 (Native) - A cooperative zombie survival game framework
// Licensed under the MIT License

#include "BaseProjectile.h"

/**
 * @file BaseProjectile.cpp
 * @brief Implementation of replicated projectile with bounce, impact FX, and damage system
 * @author Marc Fraedrich
 */
#include "ConstructorHelpers.h"
#include "Materials/MaterialInstanceConstant.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Engine/StaticMesh.h"
#include "FX/Impact/BloodImpact.h"
#include "HordeTemplateV2Native.h"
#include "Kismet/GameplayStatics.h"

/**
 * Constructor for ABaseProjectile
 *
 * @param
 * @return
 */
ABaseProjectile::ABaseProjectile()
{
	const ConstructorHelpers::FObjectFinder<UStaticMesh> TracerMeshAsset(TEXT("StaticMesh'/Game/HordeTemplateBP/Assets/Effects/Meshes/Weapon/SM_Bullet.SM_Bullet'"));
	const ConstructorHelpers::FObjectFinder<UMaterialInstanceConstant> TracerMaterialAsset(TEXT("MaterialInstanceConstant'/Game/HordeTemplateBP/Assets/Materials/M_Projectile_Trace_Inst.M_Projectile_Trace_Inst'"));
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;
	SetReplicates(true);
	InitialLifeSpan = 6.f;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("Projectile Movement Component"));
	if (GetWorld())
	{
		ProjectileMovement->RegisterComponent();
	}
	ProjectileMovement->InitialSpeed = 20000.f;
	ProjectileMovement->MaxSpeed = 20000.f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = true;
	ProjectileMovement->Bounciness = 0.05f;
	ProjectileMovement->BounceVelocityStopSimulatingThreshold = 5000.f;
	ProjectileMovement->OnProjectileStop.AddDynamic(this, &ABaseProjectile::OnProjectileStop);
	ProjectileMovement->OnProjectileBounce.AddDynamic(this, &ABaseProjectile::OnProjectileBounce);

	CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("Collision Sphere"));
	CollisionSphere->SetupAttachment(RootComponent);
	CollisionSphere->SetCollisionProfileName(FName(TEXT("Projectile")));
	CollisionSphere->SetSphereRadius(2.f);
	CollisionSphere->CanCharacterStepUpOn = ECB_No;
	CollisionSphere->bReturnMaterialOnMove = true;
	CollisionSphere->SetNotifyRigidBodyCollision(true);
	ProjectileMovement->UpdatedComponent = CollisionSphere;

	TracerMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Tracer Mesh"));
	TracerMesh->SetupAttachment(CollisionSphere);
	TracerMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	TracerMesh->SetRelativeLocation(FVector(-8.f, 0.f, 0.f));
	TracerMesh->SetRelativeScale3D(FVector(0.21875f, 0.03125f, 0.03125f));
	TracerMesh->SetIsReplicated(true);
	if (TracerMeshAsset.Succeeded() && TracerMaterialAsset.Succeeded())
	{
		TracerMesh->SetStaticMesh(TracerMeshAsset.Object);
		TracerMesh->SetMaterial(0, TracerMaterialAsset.Object);
	}

}

/**
 * Destroy Projectile if Projectile Stops.
 *
 * @param Impact Hit Result
 * @return void
 */
void ABaseProjectile::OnProjectileStop(const FHitResult& ImpactResult)
{
	Destroy(true, false);
}

/**
 * Spawn Impact FX and Apply Point Damage if Character got Hit.
 *
 * @param Impact Hit Result and Impact Velocity.
 * @return void
 */
void ABaseProjectile::OnProjectileBounce(const FHitResult& ImpactResult, const FVector& ImpactVelocity)
{
	UPhysicalMaterial* Phymat = Cast<UPhysicalMaterial>(ImpactResult.PhysMaterial);
	if (Phymat && Phymat->DetermineSurfaceType(Phymat) == SURFACE_CONCRETE)
	{
		SpawnImpactFX(ImpactResult.ImpactPoint, ImpactResult.ImpactNormal.Rotation().Quaternion(), ABaseImpact::StaticClass());
	}
	else if (Phymat && Phymat->DetermineSurfaceType(Phymat) == SURFACE_FLESH)
	{
		SpawnImpactFX(ImpactResult.ImpactPoint, ImpactResult.ImpactNormal.Rotation().Quaternion(), ABloodImpact::StaticClass());
	}
	else {
		SpawnImpactFX(ImpactResult.ImpactPoint, ImpactResult.ImpactNormal.Rotation().Quaternion(), ABaseImpact::StaticClass());
	}
	if (ImpactCounter > 0)
	{
		Destroy(true);
	}
	else
	{
		ImpactCounter++;
		if (ImpactResult.GetActor() && ImpactResult.GetActor() != GetOwner())
		{
			TracerMesh->SetVisibility(false);
			// Fixed: Pass the owner's InstigatorController for proper kill attribution
			AController* InstigatorController = GetOwner() ? GetOwner()->GetInstigatorController() : nullptr;
			UGameplayStatics::ApplyPointDamage(ImpactResult.GetActor(), Damage, CollisionSphere->GetComponentLocation(), ImpactResult, InstigatorController, GetOwner(), nullptr);
		}
	}

}

/** ( Multicast )
 * Spawn Impact FX Actor on Impact Location
 *
 * @param The Impact Location, Impact Rotation and Class of Impact.
 * @return
 */
void ABaseProjectile::SpawnImpactFX_Implementation(FVector ImpactLocation, FQuat ImpactRotation, TSubclassOf<ABaseImpact> ImpactClass)
{
	FActorSpawnParameters SpawnParam;
	FTransform ImpactTrans(ImpactLocation);
	ImpactTrans.SetRotation(ImpactRotation);
	SpawnParam.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	GetWorld()->SpawnActor<ABaseImpact>(ImpactClass, ImpactTrans, SpawnParam);
}

bool ABaseProjectile::SpawnImpactFX_Validate(FVector ImpactLocation, FQuat ImpactRotation, TSubclassOf<ABaseImpact> ImpactClass)
{
	return true;
}

