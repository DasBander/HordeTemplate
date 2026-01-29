// Copyright (c) 2018 - 2026 Marc Fraedrich
// Horde Template V2 (Native) - A cooperative zombie survival game framework
// Licensed under the MIT License

#pragma once

/**
 * @file ZedAIController.h
 * @brief AI controller for zombies with state management, perception, and combat mechanics
 * @author Marc Fraedrich
 */

#include "CoreMinimal.h"
#include "AIController.h"
#include "AIModule/Classes/Perception/AISenseConfig_Sight.h"
#include "AIModule/Classes/Perception/AIPerceptionComponent.h"
#include "ZedAIController.generated.h"

/**
 * AI State enum for zombie behavior
 */
UENUM(BlueprintType)
enum class EZedAIState : uint8
{
	Idle,           // Standing still, occasional shuffle
	Patrol,         // Slow wandering (200 u/s)
	Alert,          // Heard/saw something, investigating (300 u/s)
	Chase,          // Actively pursuing target (500 u/s)
	Attack,         // In melee range, attacking
	Staggered       // Hit reaction, temporarily slowed
};

/**
 * Zombie AI Controller with state-based behavior
 */
UCLASS()
class HORDETEMPLATEV2NATIVE_API AZedAIController : public AAIController
{
	GENERATED_BODY()

public:
	AZedAIController();

	UFUNCTION()
		void EnemyInSight(AActor* Actor, FAIStimulus Stimulus);

	// State Management
	UFUNCTION(BlueprintCallable, Category = "Zed AI|State")
		void SetAIState(EZedAIState NewState);

	UFUNCTION(BlueprintPure, Category = "Zed AI|State")
		EZedAIState GetAIState() const { return CurrentAIState; }

	UFUNCTION(BlueprintPure, Category = "Zed AI|State")
		float GetTargetSpeedForState(EZedAIState State) const;

	// Last Known Location
	UFUNCTION(BlueprintCallable, Category = "Zed AI|Perception")
		void SetLastKnownEnemyLocation(const FVector& Location);

	UFUNCTION(BlueprintPure, Category = "Zed AI|Perception")
		FVector GetLastKnownEnemyLocation() const { return LastKnownEnemyLocation; }

	UFUNCTION(BlueprintPure, Category = "Zed AI|Perception")
		bool HasLastKnownEnemyLocation() const { return bHasLastKnownEnemyLocation; }

	UFUNCTION(BlueprintCallable, Category = "Zed AI|Perception")
		void ClearLastKnownEnemyLocation();

	// Line of Sight Tracking
	UFUNCTION(BlueprintPure, Category = "Zed AI|Perception")
		bool HasLineOfSightToEnemy() const;

	UFUNCTION(BlueprintPure, Category = "Zed AI|Perception")
		float GetTimeSinceLostLOS() const { return TimeSinceLostLOS; }

	// Stagger Management
	UFUNCTION(BlueprintCallable, Category = "Zed AI|Combat")
		void ApplyStagger();

	UFUNCTION(BlueprintCallable, Category = "Zed AI|Combat")
		void ApplyAngerBurst();

protected:

	UPROPERTY()
		FTimerHandle SightClearTimer;

	UFUNCTION()
		void ClearSight();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Target")
		class UAIPerceptionComponent* PCC;

	// Current AI State
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Zed AI|State")
		EZedAIState CurrentAIState = EZedAIState::Patrol;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Zed AI|State")
		EZedAIState PreviousAIState = EZedAIState::Patrol;

	// Last Known Enemy Location
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Zed AI|Perception")
		FVector LastKnownEnemyLocation = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Zed AI|Perception")
		bool bHasLastKnownEnemyLocation = false;

	// Line of Sight Tracking
	UPROPERTY()
		float TimeSinceLostLOS = 0.f;

	UPROPERTY()
		bool bHadLOSLastCheck = false;

	UPROPERTY()
		FTimerHandle LOSCheckTimer;

	UFUNCTION()
		void CheckLineOfSight();

	// Stagger/Anger state management
	UPROPERTY()
		FTimerHandle StaggerTimer;

	UPROPERTY()
		FTimerHandle AngerTimer;

	UPROPERTY()
		bool bIsStaggered = false;

	UPROPERTY()
		bool bIsAngered = false;

	UFUNCTION()
		void EndStagger();

	UFUNCTION()
		void EndAnger();

	// State change notification
	UFUNCTION()
		void OnStateChanged(EZedAIState OldState, EZedAIState NewState);
};
