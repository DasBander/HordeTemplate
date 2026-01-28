

#pragma once
#include "HordeTemplateV2Native.h"
#include "CoreMinimal.h"
#include "Components/AudioComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "GameFramework/Actor.h"
#include "Sound/SoundCue.h"
#include "BaseFirearm.generated.h"

class AHordeBaseCharacter;

/**
 * Firing state for server-authoritative weapon control
 */
UENUM(BlueprintType)
enum class EWeaponFiringState : uint8
{
	Idle,			// Not firing
	Firing,			// Currently firing (single shot)
	BurstFiring,	// In burst fire sequence
	AutoFiring		// Full auto firing
};

UCLASS()
class HORDETEMPLATEV2NATIVE_API ABaseFirearm : public AActor
{
	GENERATED_BODY()

public:
	ABaseFirearm();

	// ==================== CORE FUNCTIONS ====================

	UFUNCTION(BlueprintCallable, Category = "Firearm")
		virtual void FireFirearm();

	/** Check if weapon can fire (server-authoritative) */
	UFUNCTION(BlueprintPure, Category = "Firearm")
		bool CanFire() const;

	/** Get time until next shot is allowed */
	UFUNCTION(BlueprintPure, Category = "Firearm")
		float GetTimeUntilNextShot() const;

	// ==================== REPLICATED STATE ====================

	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Firearm")
		int32 LoadedAmmo;

	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Firearm")
		FString WeaponID;

	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Firearm")
		uint8 FireMode;

	UPROPERTY(BlueprintReadWrite, Replicated, Category = "Firearm")
		bool ProjectileFromMuzzle = false;

	/** Current firing state (replicated for animation sync) */
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Firearm|State")
		EWeaponFiringState FiringState = EWeaponFiringState::Idle;

	/** Is weapon currently in a firing sequence */
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Firearm|State")
		bool bIsFiring = false;

	// ==================== SPREAD CONFIGURATION ====================

	/** Base spread angle in degrees (0 = perfect accuracy) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Firearm|Spread")
		float BaseSpreadAngle = 0.5f;

	/** Additional spread when moving */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Firearm|Spread")
		float MovingSpreadMultiplier = 1.5f;

	/** Additional spread when in air */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Firearm|Spread")
		float AirborneSpreadMultiplier = 2.0f;

	/** Spread increase per consecutive shot (resets after not firing) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Firearm|Spread")
		float SpreadIncreasePerShot = 0.1f;

	/** Maximum spread from consecutive shots */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Firearm|Spread")
		float MaxAccumulatedSpread = 3.0f;

	/** Time to reset spread after not firing (seconds) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Firearm|Spread")
		float SpreadResetTime = 0.3f;

	// ==================== SERVER RPCS ====================

	UFUNCTION(Server, WithValidation, Reliable, BlueprintCallable, Category = "Firearm")
		void ServerToggleFireMode();

	/** Request to fire weapon - server validates timing */
	UFUNCTION(Server, WithValidation, Reliable, BlueprintCallable, Category = "Firearm")
		void ServerFireFirearm();

	/** Request to start automatic/burst fire */
	UFUNCTION(Server, WithValidation, Reliable, BlueprintCallable, Category = "Firearm")
		void ServerStartFiring();

	/** Request to stop automatic/burst fire */
	UFUNCTION(Server, WithValidation, Reliable, BlueprintCallable, Category = "Firearm")
		void ServerStopFiring();

protected:

	// ==================== TIMING (SERVER-AUTHORITATIVE) ====================

	/** Server timestamp of last shot fired */
	UPROPERTY()
		double LastFireTime = 0.0;

	/** Cached fire rate from weapon data */
	UPROPERTY()
		float CachedFireRate = 0.1f;

	/** Current accumulated spread from consecutive shots */
	UPROPERTY()
		float AccumulatedSpread = 0.0f;

	/** Number of shots in current burst */
	UPROPERTY()
		int32 CurrentBurstCount = 0;

	/** Maximum burst count from weapon data */
	UPROPERTY()
		int32 MaxBurstCount = 3;

	/** Timer for auto/burst fire */
	UPROPERTY()
		FTimerHandle FireTimerHandle;

	/** Timer for spread reset */
	UPROPERTY()
		FTimerHandle SpreadResetTimerHandle;

	// ==================== INTERNAL FUNCTIONS ====================

	/** Execute a single shot (called by server timer for auto/burst) */
	UFUNCTION()
		void ExecuteFireSequence();

	/** Handle burst fire completion */
	UFUNCTION()
		void OnBurstComplete();

	/** Reset accumulated spread */
	UFUNCTION()
		void ResetSpread();

	/** Calculate current spread angle based on state */
	UFUNCTION(BlueprintPure, Category = "Firearm|Spread")
		float CalculateCurrentSpread() const;

	/** Apply spread to a rotation */
	UFUNCTION(BlueprintPure, Category = "Firearm|Spread")
		FRotator ApplySpread(const FRotator& BaseRotation) const;

	/** Validate the owning character state */
	bool ValidateOwnerState() const;

	/** Cache weapon data on spawn */
	void CacheWeaponData();

	// ==================== FX ====================

	/** Play firing effects on all clients (now Reliable for competitive) */
	UFUNCTION(NetMulticast, WithValidation, Reliable, BlueprintCallable, Category = "Firearm|FX")
		void PlayFirearmFX();

	/** Notify clients that firing has started */
	UFUNCTION(NetMulticast, Reliable, Category = "Firearm|FX")
		void Multicast_OnFiringStarted(EWeaponFiringState NewState);

	/** Notify clients that firing has stopped */
	UFUNCTION(NetMulticast, Reliable, Category = "Firearm|FX")
		void Multicast_OnFiringStopped();

	// ==================== COMPONENTS ====================

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Firearm")
		USkeletalMeshComponent* Weapon;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Firearm")
		UParticleSystemComponent* MuzzleFlash;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Firearm")
		UAudioComponent* WeaponSound;

	// ==================== UTILITY ====================

	UFUNCTION(BlueprintPure, Category = "Firearm")
		void GetOwnerEyePoint(bool LocationFromWeapon, FVector& ViewLocation, FRotator& ViewRotation);

	/** Get the owning character (cached for performance) */
	UFUNCTION(BlueprintPure, Category = "Firearm")
		AHordeBaseCharacter* GetOwningCharacter() const;

	virtual void BeginPlay() override;
};
