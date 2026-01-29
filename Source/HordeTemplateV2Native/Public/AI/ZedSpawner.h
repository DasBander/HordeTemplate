// Copyright (c) 2018 - 2026 Marc Fraedrich
// Horde Template V2 (Native) - A cooperative zombie survival game framework
// Licensed under the MIT License

#pragma once

/**
 * @file ZedSpawner.h
 * @brief Dynamic zombie spawner with distance-based activation, vision checks, and exhaustion mechanics
 * @author Marc Fraedrich
 */

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "Components/BillboardComponent.h"
#include "ZedSpawner.generated.h"

class AZedPawn;

/**
 * Spawn mode for the ZedSpawner
 */
UENUM(BlueprintType)
enum class EZedSpawnMode : uint8
{
	Continuous,		// Spawns continuously while active
	Wave,			// Spawns a wave then waits
	Triggered		// Only spawns when triggered externally
};

/**
 * Current state of the spawner
 */
UENUM(BlueprintType)
enum class EZedSpawnerState : uint8
{
	Inactive,		// Not spawning (player too far or not yet activated)
	Active,			// Currently able to spawn
	Exhausted,		// Reached max kills, waiting for reactivation
	Disabled		// Permanently disabled
};

/**
 * Dynamic zombie spawner that mimics AAA-style spawning behavior.
 * Spawns zombies based on player distance, line of sight, and exhaustion limits.
 */
UCLASS(ClassGroup = "Horde AI", Blueprintable)
class HORDETEMPLATEV2NATIVE_API AZedSpawner : public AActor
{
	GENERATED_BODY()

public:
	AZedSpawner();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// ==================== CONFIGURATION ====================

	/** The zombie class to spawn */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner|Configuration")
		TSubclassOf<AZedPawn> ZombieClass;

	/** Spawn mode determines how zombies are spawned */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner|Configuration")
		EZedSpawnMode SpawnMode = EZedSpawnMode::Continuous;

	/** Patrol tag assigned to spawned zombies */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner|Configuration")
		FName PatrolTag;

	// ==================== DISTANCE SETTINGS ====================

	/** Minimum distance from any player to activate spawning */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner|Distance", meta = (ClampMin = "0"))
		float MinActivationDistance = 500.f;

	/** Maximum distance from any player to activate spawning */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner|Distance", meta = (ClampMin = "0"))
		float MaxActivationDistance = 3000.f;

	/** If true, deactivates when all players are outside max distance */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner|Distance")
		bool bDeactivateWhenFar = true;

	// ==================== VISION SETTINGS ====================

	/** If true, won't spawn if any player is looking at the spawn area */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner|Vision")
		bool bCheckPlayerVision = true;

	/** Field of view angle for player vision check (degrees from center) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner|Vision", meta = (ClampMin = "0", ClampMax = "180"))
		float PlayerVisionAngle = 60.f;

	/** Additional buffer distance behind objects for spawning (prevents spawning just out of sight) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner|Vision", meta = (ClampMin = "0"))
		float VisionBufferDistance = 200.f;

	// ==================== SPAWN RATE SETTINGS ====================

	/** Time between spawn attempts (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner|Rate", meta = (ClampMin = "0.1"))
		float SpawnInterval = 3.f;

	/** Random deviation added to spawn interval */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner|Rate", meta = (ClampMin = "0"))
		float SpawnIntervalDeviation = 1.f;

	/** Maximum zombies this spawner can have alive at once */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner|Rate", meta = (ClampMin = "1"))
		int32 MaxConcurrentZombies = 5;

	/** Number of zombies to spawn per wave (Wave mode only) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner|Rate", meta = (ClampMin = "1"))
		int32 WaveSize = 3;

	/** Delay between waves (Wave mode only) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner|Rate", meta = (ClampMin = "0"))
		float WaveDelay = 10.f;

	// ==================== EXHAUSTION SETTINGS ====================

	/** If true, spawner will exhaust after reaching kill limit */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner|Exhaustion")
		bool bCanExhaust = true;

	/** Number of zombies that must be killed before spawner exhausts */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner|Exhaustion", meta = (ClampMin = "1", EditCondition = "bCanExhaust"))
		int32 MaxKillsBeforeExhaustion = 10;

	/** If true, spawner can reactivate after exhaustion */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner|Exhaustion", meta = (EditCondition = "bCanExhaust"))
		bool bCanReactivate = true;

	/** Time to wait before reactivation after exhaustion (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner|Exhaustion", meta = (ClampMin = "0", EditCondition = "bCanExhaust && bCanReactivate"))
		float ReactivationDelay = 60.f;

	/** If true, reactivation requires player to leave and re-enter activation range */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner|Exhaustion", meta = (EditCondition = "bCanExhaust && bCanReactivate"))
		bool bRequireReEntryForReactivation = true;

	// ==================== DIFFICULTY SCALING ====================

	/** If true, adjusts spawn rate based on global zombie count */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner|Difficulty")
		bool bScaleWithGlobalZombieCount = true;

	/** Maximum global zombies - spawner slows down as this is approached */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner|Difficulty", meta = (ClampMin = "1", EditCondition = "bScaleWithGlobalZombieCount"))
		int32 GlobalZombieLimit = 30;

	/** If true, scales spawn rate with player count */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner|Difficulty")
		bool bScaleWithPlayerCount = true;

	/** Spawn rate multiplier per additional player (1.0 = no change) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner|Difficulty", meta = (ClampMin = "0.1", EditCondition = "bScaleWithPlayerCount"))
		float PlayerCountMultiplier = 1.25f;

	// ==================== DEBUG ====================

	/** Enable debug visualization */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner|Debug")
		bool bShowDebug = false;

	// ==================== RUNTIME STATE (Read-Only) ====================

	/** Current spawner state */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "Spawner|State")
		EZedSpawnerState CurrentState = EZedSpawnerState::Inactive;

	/** Number of zombies killed from this spawner */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "Spawner|State")
		int32 TotalKills = 0;

	/** Currently alive zombies from this spawner */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spawner|State")
		int32 CurrentAliveZombies = 0;

	// ==================== BLUEPRINT EVENTS ====================

	/** Called when spawner state changes */
	UFUNCTION(BlueprintImplementableEvent, Category = "Spawner|Events")
		void OnStateChanged(EZedSpawnerState OldState, EZedSpawnerState NewState);

	/** Called when a zombie is spawned */
	UFUNCTION(BlueprintImplementableEvent, Category = "Spawner|Events")
		void OnZombieSpawned(AZedPawn* SpawnedZombie);

	/** Called when spawner exhausts */
	UFUNCTION(BlueprintImplementableEvent, Category = "Spawner|Events")
		void OnSpawnerExhausted();

	/** Called when spawner reactivates */
	UFUNCTION(BlueprintImplementableEvent, Category = "Spawner|Events")
		void OnSpawnerReactivated();

	// ==================== PUBLIC FUNCTIONS ====================

	/** Manually trigger a spawn (Triggered mode) */
	UFUNCTION(BlueprintCallable, Category = "Spawner")
		void TriggerSpawn(int32 Count = 1);

	/** Force spawner to exhaust */
	UFUNCTION(BlueprintCallable, Category = "Spawner")
		void ForceExhaust();

	/** Force spawner to reactivate */
	UFUNCTION(BlueprintCallable, Category = "Spawner")
		void ForceReactivate();

	/** Permanently disable the spawner */
	UFUNCTION(BlueprintCallable, Category = "Spawner")
		void DisableSpawner();

	/** Enable a disabled spawner */
	UFUNCTION(BlueprintCallable, Category = "Spawner")
		void EnableSpawner();

	/** Called when a zombie from this spawner dies */
	UFUNCTION()
		void OnZombieDied(AZedPawn* DeadZombie);

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	// ==================== COMPONENTS ====================

	UPROPERTY(VisibleDefaultsOnly, Category = "Components")
		class UBoxComponent* SpawnVolume;

	UPROPERTY(VisibleDefaultsOnly, Category = "Components")
		class UBillboardComponent* EditorIcon;

	// ==================== INTERNAL STATE ====================

	UPROPERTY()
		TArray<TWeakObjectPtr<AZedPawn>> SpawnedZombies;

	UPROPERTY()
		FTimerHandle SpawnTimerHandle;

	UPROPERTY()
		FTimerHandle ReactivationTimerHandle;

	UPROPERTY()
		bool bPlayerWasInRange = false;

	UPROPERTY()
		float TimeSinceLastSpawn = 0.f;

	UPROPERTY()
		int32 CurrentWaveSpawned = 0;

	// ==================== INTERNAL FUNCTIONS ====================

	/** Main spawning logic */
	UFUNCTION()
		void AttemptSpawn();

	/** Check if spawning conditions are met */
	UFUNCTION()
		bool CanSpawn() const;

	/** Check if any player is within activation distance */
	UFUNCTION()
		bool IsPlayerInRange() const;

	/** Check if any player is looking at the spawn area */
	UFUNCTION()
		bool IsPlayerLookingAtSpawnArea() const;

	/** Get a valid spawn location within the spawn volume */
	UFUNCTION()
		bool GetValidSpawnLocation(FVector& OutLocation) const;

	/** Actually spawn a zombie at the given location */
	UFUNCTION()
		AZedPawn* SpawnZombieAtLocation(const FVector& Location);

	/** Update spawner state based on conditions */
	UFUNCTION()
		void UpdateState();

	/** Set new state and fire events */
	UFUNCTION()
		void SetState(EZedSpawnerState NewState);

	/** Clean up dead zombie references */
	UFUNCTION()
		void CleanupDeadZombies();

	/** Get spawn rate multiplier based on difficulty settings */
	UFUNCTION()
		float GetDifficultyMultiplier() const;

	/** Timer callback for reactivation */
	UFUNCTION()
		void OnReactivationTimer();

	/** Get closest player distance */
	UFUNCTION()
		float GetClosestPlayerDistance() const;

	/** Get all alive players */
	UFUNCTION()
		TArray<ACharacter*> GetAlivePlayers() const;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};
