

#pragma once
#include "CoreMinimal.h"
#include "Gameplay/HordePlayerState.h"
#include "GameFramework/Character.h"
#include "Components/ArrowComponent.h"
#include "Components/SphereComponent.h"
#include "Components/AudioComponent.h"
#include "ZedPawn.generated.h"

// Forward declare the enum
enum class EZedAIState : uint8;

UCLASS()
class HORDETEMPLATEV2NATIVE_API AZedPawn : public ACharacter
{
	GENERATED_BODY()

public:
	AZedPawn();

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Zed AI")
		class UArrowComponent* AttackPoint;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Zed AI")
		class USphereComponent* PlayerRangeCollision;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Zed AI")
		class UAudioComponent* ZedIdleSound;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Zed AI|Sound")
		class UAudioComponent* ZedChaseBreathing;

	// Optimization: PatrolTag is only used server-side for AI behavior, removed replication
	UPROPERTY(BlueprintReadOnly, Category = "Zed AI")
		FName PatrolTag;

	UFUNCTION(NetMulticast, Unreliable, WithValidation, BlueprintCallable, Category = "Zed AI")
		void ModifyWalkSpeed(float MaxWalkSpeed);

	UFUNCTION(NetMulticast, WithValidation, Unreliable, BlueprintCallable, Category = "Zed AI|FX")
		void PlayAttackFX();

	// Smooth Speed Transition System
	UFUNCTION(BlueprintCallable, Category = "Zed AI|Movement")
		void SetTargetWalkSpeed(float NewTargetSpeed);

	UFUNCTION(BlueprintPure, Category = "Zed AI|Movement")
		float GetTargetWalkSpeed() const { return TargetWalkSpeed; }

	UFUNCTION(BlueprintPure, Category = "Zed AI|Movement")
		float GetCurrentWalkSpeed() const { return CurrentWalkSpeed; }

	UFUNCTION(BlueprintPure, Category = "Zed AI|Movement")
		float GetBaseSpeedMultiplier() const { return BaseSpeedMultiplier; }

	// Attack Cooldown System
	UFUNCTION(BlueprintPure, Category = "Zed AI|Combat")
		bool CanAttack() const;

	UFUNCTION(BlueprintCallable, Category = "Zed AI|Combat")
		void ResetAttackCooldown();

	UFUNCTION(BlueprintPure, Category = "Zed AI|Combat")
		float GetAttackCooldownRemaining() const;

	// Sound Functions
	UFUNCTION(NetMulticast, Unreliable, WithValidation, BlueprintCallable, Category = "Zed AI|Sound")
		void PlayAlertSound();

	UFUNCTION(NetMulticast, Unreliable, WithValidation, BlueprintCallable, Category = "Zed AI|Sound")
		void StartChaseBreathing();

	UFUNCTION(NetMulticast, Unreliable, WithValidation, BlueprintCallable, Category = "Zed AI|Sound")
		void StopChaseBreathing();

	// Animation State Exposure (for Animation Blueprint)
	UFUNCTION(BlueprintPure, Category = "Zed AI|Animation")
		EZedAIState GetCurrentAIState() const;

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Zed AI")
		float Health = 100.f;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Zed AI")
		bool IsDead = false;

	// Smooth Speed Transition Properties
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Zed AI|Movement")
		float TargetWalkSpeed = 200.f;

	UPROPERTY(BlueprintReadOnly, Category = "Zed AI|Movement")
		float CurrentWalkSpeed = 200.f;

	// Speed variation multiplier (set on spawn)
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Zed AI|Movement")
		float BaseSpeedMultiplier = 1.f;

	// Attack Cooldown Properties
	UPROPERTY(BlueprintReadOnly, Category = "Zed AI|Combat")
		float LastAttackTime = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "Zed AI|Combat")
		float AttackCooldownDuration = 2.f;

	UFUNCTION()
		void GivePlayerPoints(ACharacter * Player, int32 Points, EPointType PointType);

	UFUNCTION(NetMulticast, WithValidation, Unreliable, BlueprintCallable, Category = "Zed AI|FX")
		void PlayHeadShotFX();

	UFUNCTION(NetMulticast, WithValidation, Unreliable, BlueprintCallable, Category = "Zed AI|FX")
		void DeathFX(FVector Direction);

	UFUNCTION()
		float TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	UFUNCTION()
		void KillAI(ACharacter* Killer, EPointType KillType);

	UFUNCTION()
		void OnCharacterInRange(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
		void OnCharacterOutRange(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	// Track number of players in range to properly handle multiple players
	UPROPERTY()
		int32 PlayersInRangeCount = 0;

	// Hit Reaction Processing
	UFUNCTION()
		void ProcessHitReaction();

	// Speed interpolation
	UFUNCTION()
		void UpdateWalkSpeedInterpolation(float DeltaTime);

public:
	FORCEINLINE float GetHealth() { return Health; };
	FORCEINLINE bool GetIsDead() { return IsDead; };
};
