// Copyright (c) 2018 - 2026 Marc Fraedrich
// Horde Template V2 (Native) - A cooperative zombie survival game framework
// Licensed under the MIT License

#include "BaseFirearm.h"

/**
 * @file BaseFirearm.cpp
 * @brief Implementation of server-authoritative weapon system with spread, timing, and fire modes
 * @author Marc Fraedrich
 */
#include "Gameplay/GameplayStructures.h"
#include "Inventory/InventoryHelpers.h"
#include "Character/HordeBaseCharacter.h"
#include "Gameplay/HordeBaseController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/KismetMathLibrary.h"


ABaseFirearm::ABaseFirearm()
{
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;
	SetReplicates(true);

	// Root Component
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root Component"));
	Weapon = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Weapon Mesh"));
	Weapon->SetupAttachment(RootComponent);

	// Particle Component (For Muzzle Flash)
	MuzzleFlash = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("Muzzle Flash"));
	MuzzleFlash->SetupAttachment(Weapon);
	MuzzleFlash->SetAutoActivate(false);

	// Audio Component (For Weapon Sound)
	WeaponSound = CreateDefaultSubobject<UAudioComponent>(TEXT("Weapon Sound"));
	WeaponSound->SetupAttachment(Weapon);
	WeaponSound->SetAutoActivate(false);
}


void ABaseFirearm::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABaseFirearm, LoadedAmmo);
	DOREPLIFETIME(ABaseFirearm, WeaponID);
	DOREPLIFETIME(ABaseFirearm, ProjectileFromMuzzle);
	DOREPLIFETIME(ABaseFirearm, FireMode);
	DOREPLIFETIME(ABaseFirearm, FiringState);
	DOREPLIFETIME(ABaseFirearm, bIsFiring);
}


void ABaseFirearm::BeginPlay()
{
	Super::BeginPlay();

	// Cache weapon data on server
	if (HasAuthority())
	{
		CacheWeaponData();
	}
}


void ABaseFirearm::CacheWeaponData()
{
	FItem WeaponData = UInventoryHelpers::FindItemByID(FName(*WeaponID));
	if (WeaponData.ItemID != "None")
	{
		CachedFireRate = FMath::Max(WeaponData.FireRate, 0.05f); // Minimum 50ms between shots
		MaxBurstCount = FMath::Max((int32)WeaponData.BurstFireAmount, 1);
	}
}


// ==================== VALIDATION ====================


bool ABaseFirearm::ValidateOwnerState() const
{
	AHordeBaseCharacter* OwnerChar = GetOwningCharacter();
	if (!OwnerChar)
	{
		return false;
	}

	// Check if owner is dead
	if (OwnerChar->GetIsDead())
	{
		return false;
	}

	// Check if owner is reloading
	if (OwnerChar->Reloading)
	{
		return false;
	}

	return true;
}


bool ABaseFirearm::CanFire() const
{
	// Must have ammo
	if (LoadedAmmo <= 0)
	{
		return false;
	}

	// Must pass owner validation
	if (!ValidateOwnerState())
	{
		return false;
	}

	// Check fire rate timing (server-authoritative)
	if (HasAuthority())
	{
		const double CurrentTime = GetWorld()->GetTimeSeconds();
		const double TimeSinceLastShot = CurrentTime - LastFireTime;

		if (TimeSinceLastShot < CachedFireRate)
		{
			return false;
		}
	}

	return true;
}


float ABaseFirearm::GetTimeUntilNextShot() const
{
	if (!HasAuthority())
	{
		return 0.0f;
	}

	const double CurrentTime = GetWorld()->GetTimeSeconds();
	const double TimeSinceLastShot = CurrentTime - LastFireTime;
	const float TimeRemaining = CachedFireRate - (float)TimeSinceLastShot;

	return FMath::Max(TimeRemaining, 0.0f);
}


AHordeBaseCharacter* ABaseFirearm::GetOwningCharacter() const
{
	return Cast<AHordeBaseCharacter>(GetOwner());
}


// ==================== SPREAD SYSTEM ====================


float ABaseFirearm::CalculateCurrentSpread() const
{
	float CurrentSpread = BaseSpreadAngle + AccumulatedSpread;

	AHordeBaseCharacter* OwnerChar = GetOwningCharacter();
	if (OwnerChar)
	{
		// Add spread if moving
		if (OwnerChar->GetVelocity().SizeSquared() > 100.0f)
		{
			CurrentSpread *= MovingSpreadMultiplier;
		}

		// Add spread if airborne
		UCharacterMovementComponent* MovementComp = OwnerChar->GetCharacterMovement();
		if (MovementComp && !MovementComp->IsMovingOnGround())
		{
			CurrentSpread *= AirborneSpreadMultiplier;
		}
	}

	return FMath::Clamp(CurrentSpread, 0.0f, MaxAccumulatedSpread + BaseSpreadAngle * AirborneSpreadMultiplier * MovingSpreadMultiplier);
}


FRotator ABaseFirearm::ApplySpread(const FRotator& BaseRotation) const
{
	const float SpreadAngle = CalculateCurrentSpread();

	if (SpreadAngle <= 0.0f)
	{
		return BaseRotation;
	}

	// Generate random spread within cone
	const float HalfAngleRad = FMath::DegreesToRadians(SpreadAngle * 0.5f);
	const float RandomAngle = FMath::FRandRange(0.0f, 2.0f * PI);
	const float RandomRadius = FMath::FRandRange(0.0f, FMath::Sin(HalfAngleRad));

	const float SpreadYaw = FMath::RadiansToDegrees(FMath::Cos(RandomAngle) * RandomRadius);
	const float SpreadPitch = FMath::RadiansToDegrees(FMath::Sin(RandomAngle) * RandomRadius);

	return FRotator(
		BaseRotation.Pitch + SpreadPitch,
		BaseRotation.Yaw + SpreadYaw,
		BaseRotation.Roll
	);
}


void ABaseFirearm::ResetSpread()
{
	AccumulatedSpread = 0.0f;
}


// ==================== FIRING LOGIC ====================


void ABaseFirearm::FireFirearm()
{
	if (!HasAuthority())
	{
		return;
	}

	if (!CanFire())
	{
		return;
	}

	FItem CurrentWeaponItem = UInventoryHelpers::FindItemByID(FName(*WeaponID));

	if (!CurrentWeaponItem.ProjectileClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("FireFirearm: ProjectileClass is null for weapon %s"), *WeaponID);
		return;
	}

	// Update last fire time BEFORE spawning (prevents double-fire exploits)
	LastFireTime = GetWorld()->GetTimeSeconds();

	// Consume ammo
	LoadedAmmo = FMath::Max(LoadedAmmo - 1, 0);

	// Play FX on all clients
	PlayFirearmFX();

	// Get spawn transform
	FVector EyeViewPoint;
	FRotator EyeRotation;
	GetOwnerEyePoint(ProjectileFromMuzzle, EyeViewPoint, EyeRotation);

	// Apply spread
	FRotator SpreadRotation = ApplySpread(EyeRotation);

	FTransform SpawnTransform;
	SpawnTransform.SetLocation(EyeViewPoint);
	SpawnTransform.SetRotation(SpreadRotation.Quaternion());

	// Spawn projectile on server
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = GetOwner();
	SpawnParams.Instigator = Cast<APawn>(GetOwner());

	ABaseProjectile* Projectile = GetWorld()->SpawnActor<ABaseProjectile>(
		CurrentWeaponItem.ProjectileClass,
		SpawnTransform,
		SpawnParams
	);

	if (Projectile)
	{
		Projectile->SetOwner(GetOwner());
	}

	// Update accumulated spread
	AccumulatedSpread = FMath::Min(AccumulatedSpread + SpreadIncreasePerShot, MaxAccumulatedSpread);

	// Reset spread decay timer
	GetWorld()->GetTimerManager().ClearTimer(SpreadResetTimerHandle);
	GetWorld()->GetTimerManager().SetTimer(
		SpreadResetTimerHandle,
		this,
		&ABaseFirearm::ResetSpread,
		SpreadResetTime,
		false
	);

	// Update inventory ammo display
	AHordeBaseCharacter* OwnerChar = GetOwningCharacter();
	if (OwnerChar && OwnerChar->Inventory)
	{
		OwnerChar->Inventory->UpdateCurrentItemAmmo(LoadedAmmo);
	}
}


void ABaseFirearm::ExecuteFireSequence()
{
	if (!HasAuthority())
	{
		return;
	}

	// Check if we should stop firing (permanent conditions)
	AHordeBaseCharacter* OwnerChar = GetOwningCharacter();
	if (!OwnerChar || OwnerChar->GetIsDead() || OwnerChar->Reloading)
	{
		ServerStopFiring();
		return;
	}

	// Check ammo
	if (LoadedAmmo <= 0)
	{
		ServerStopFiring();
		return;
	}

	// Check fire rate timing - if not ready, just skip this tick (don't stop)
	const double CurrentTime = GetWorld()->GetTimeSeconds();
	const double TimeSinceLastShot = CurrentTime - LastFireTime;
	if (TimeSinceLastShot < CachedFireRate)
	{
		// Not enough time has passed, wait for next timer tick
		return;
	}

	// Fire the weapon
	FireFirearm();

	// Handle burst mode completion
	if (FiringState == EWeaponFiringState::BurstFiring)
	{
		CurrentBurstCount++;
		if (CurrentBurstCount >= MaxBurstCount)
		{
			OnBurstComplete();
		}
	}
}


void ABaseFirearm::OnBurstComplete()
{
	if (!HasAuthority())
	{
		return;
	}

	GetWorld()->GetTimerManager().ClearTimer(FireTimerHandle);
	CurrentBurstCount = 0;
	FiringState = EWeaponFiringState::Idle;
	bIsFiring = false;

	Multicast_OnFiringStopped();
}


// ==================== SERVER RPCS ====================


void ABaseFirearm::ServerFireFirearm_Implementation()
{
	// For single fire mode, just fire once
	EFireMode CurrentFireMode = static_cast<EFireMode>(FireMode);

	if (CurrentFireMode == EFireMode::EFireModeSingle)
	{
		if (CanFire())
		{
			FiringState = EWeaponFiringState::Firing;
			FireFirearm();
			FiringState = EWeaponFiringState::Idle;
		}
	}
	else
	{
		// For burst/auto, use ServerStartFiring
		ServerStartFiring();
	}
}


bool ABaseFirearm::ServerFireFirearm_Validate()
{
	// Never kick players for fire requests - invalid states are handled in the implementation
	// Kicking for validation failure causes game restarts when client/server state desyncs
	return true;
}


void ABaseFirearm::ServerStartFiring_Implementation()
{
	if (!HasAuthority())
	{
		return;
	}

	// Already firing?
	if (bIsFiring)
	{
		return;
	}

	// Can we fire?
	if (!CanFire())
	{
		return;
	}

	EFireMode CurrentFireMode = static_cast<EFireMode>(FireMode);

	switch (CurrentFireMode)
	{
	case EFireMode::EFireModeSingle:
		// Single fire - just fire once
		FiringState = EWeaponFiringState::Firing;
		FireFirearm();
		FiringState = EWeaponFiringState::Idle;
		break;

	case EFireMode::EFireModeBurst:
		// Burst fire - fire MaxBurstCount shots
		FiringState = EWeaponFiringState::BurstFiring;
		bIsFiring = true;
		CurrentBurstCount = 0;

		// Fire first shot immediately
		FireFirearm();
		CurrentBurstCount = 1;

		// Set up timer for remaining burst shots
		if (CurrentBurstCount < MaxBurstCount)
		{
			GetWorld()->GetTimerManager().SetTimer(
				FireTimerHandle,
				this,
				&ABaseFirearm::ExecuteFireSequence,
				CachedFireRate,
				true
			);
		}
		else
		{
			OnBurstComplete();
		}

		Multicast_OnFiringStarted(FiringState);
		break;

	case EFireMode::EFireModeFull:
		// Full auto - fire continuously until stopped
		FiringState = EWeaponFiringState::AutoFiring;
		bIsFiring = true;

		// Fire first shot immediately
		FireFirearm();

		// Set up timer for continuous fire
		GetWorld()->GetTimerManager().SetTimer(
			FireTimerHandle,
			this,
			&ABaseFirearm::ExecuteFireSequence,
			CachedFireRate,
			true
		);

		Multicast_OnFiringStarted(FiringState);
		break;
	}
}


bool ABaseFirearm::ServerStartFiring_Validate()
{
	return ServerFireFirearm_Validate();
}


void ABaseFirearm::ServerStopFiring_Implementation()
{
	if (!HasAuthority())
	{
		return;
	}

	// Gracefully handle invalid states (validation doesn't kick, but we still check)
	AHordeBaseCharacter* OwnerChar = GetOwningCharacter();
	if (!OwnerChar || OwnerChar->GetCurrentFirearm() != this)
	{
		// Weapon is no longer the active weapon or owner changed - just clean up state
		GetWorld()->GetTimerManager().ClearTimer(FireTimerHandle);
		FiringState = EWeaponFiringState::Idle;
		bIsFiring = false;
		return;
	}

	// Don't stop burst mid-sequence (let it complete)
	if (FiringState == EWeaponFiringState::BurstFiring)
	{
		return;
	}

	// Clear fire timer only for non-burst modes
	GetWorld()->GetTimerManager().ClearTimer(FireTimerHandle);

	// Stop auto fire
	if (FiringState == EWeaponFiringState::AutoFiring)
	{
		FiringState = EWeaponFiringState::Idle;
		bIsFiring = false;
		Multicast_OnFiringStopped();
	}
}


bool ABaseFirearm::ServerStopFiring_Validate()
{
	// Stop firing is always a safe operation - never kick for trying to stop
	// Invalid states are handled gracefully in the implementation
	return true;
}


void ABaseFirearm::ServerToggleFireMode_Implementation()
{
	EFireMode CurrentFireMode = static_cast<EFireMode>(FireMode);
	FItem CurrentWeaponItem = UInventoryHelpers::FindItemByID(FName(*WeaponID));

	if (CurrentWeaponItem.FireModes.Num() == 0)
	{
		return;
	}

	int32 CurrentSelectedIndex = CurrentWeaponItem.FireModes.Find(CurrentFireMode);

	if (CurrentSelectedIndex == INDEX_NONE)
	{
		FireMode = static_cast<uint8>(CurrentWeaponItem.FireModes[0]);
	}
	else if (CurrentSelectedIndex == CurrentWeaponItem.FireModes.Num() - 1)
	{
		FireMode = static_cast<uint8>(CurrentWeaponItem.FireModes[0]);
	}
	else
	{
		FireMode = static_cast<uint8>(CurrentWeaponItem.FireModes[CurrentSelectedIndex + 1]);
	}
}


bool ABaseFirearm::ServerToggleFireMode_Validate()
{
	return true;
}


// ==================== MULTICAST FUNCTIONS ====================


void ABaseFirearm::PlayFirearmFX_Implementation()
{
	if (MuzzleFlash)
	{
		MuzzleFlash->Activate(true);
	}

	if (WeaponSound)
	{
		WeaponSound->Play();
	}

	// Apply recoil only to the owning player
	AHordeBaseCharacter* OwnerChar = GetOwningCharacter();
	if (OwnerChar && OwnerChar->IsLocallyControlled())
	{
		APlayerController* PC = Cast<APlayerController>(OwnerChar->GetController());
		if (PC)
		{
			// Apply direct camera recoil for immediate kick feel
			float PitchRecoil = -RecoilPitch; // Negative pitch = look up
			float YawRecoil = FMath::RandRange(-RecoilYaw, RecoilYaw); // Random left/right

			PC->AddPitchInput(PitchRecoil);
			PC->AddYawInput(YawRecoil);

			// Play camera shake for additional feedback
			if (FireCameraShake)
			{
				PC->ClientStartCameraShake(FireCameraShake, 1.0f);
			}

			// Force feedback (controller rumble) for weapon fire
			PC->PlayDynamicForceFeedback(
				0.3f,   // Intensity
				0.1f,   // Duration
				true,   // Left large
				false,  // Left small
				true,   // Right large
				false,  // Right small
				EDynamicForceFeedbackAction::Start
			);
		}
	}
}


bool ABaseFirearm::PlayFirearmFX_Validate()
{
	return true;
}


void ABaseFirearm::Multicast_OnFiringStarted_Implementation(EWeaponFiringState NewState)
{
	// Can be used by animation blueprints to start looping fire animations
	FiringState = NewState;
	bIsFiring = true;
}


void ABaseFirearm::Multicast_OnFiringStopped_Implementation()
{
	// Can be used by animation blueprints to stop looping fire animations
	FiringState = EWeaponFiringState::Idle;
	bIsFiring = false;
}


// ==================== UTILITY ====================


void ABaseFirearm::GetOwnerEyePoint(bool LocationFromWeapon, FVector& ViewLocation, FRotator& ViewRotation)
{
	AHordeBaseCharacter* OwnerChar = GetOwningCharacter();
	if (OwnerChar)
	{
		FVector CopyLocationViewPoint;
		FRotator CopyRotationViewPoint;
		OwnerChar->GetActorEyesViewPoint(CopyLocationViewPoint, CopyRotationViewPoint);

		if (LocationFromWeapon && Weapon)
		{
			ViewLocation = Weapon->GetSocketLocation(FName("muzzle"));
			ViewRotation = Weapon->GetSocketRotation(FName("muzzle"));
		}
		else
		{
			ViewLocation = OwnerChar->GetCamera()->GetComponentLocation();
			ViewRotation = CopyRotationViewPoint;
		}
	}
	else
	{
		ViewLocation = FVector::ZeroVector;
		ViewRotation = FRotator::ZeroRotator;
	}
}
