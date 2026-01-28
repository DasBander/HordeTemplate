

#pragma once
#include "CoreMinimal.h"
#include "Gameplay/HordePlayerState.h"
#include "GameFramework/Character.h"
#include "Components/ArrowComponent.h"
#include "Components/SphereComponent.h"
#include "Components/AudioComponent.h"
#include "ZedPawn.generated.h"

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

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Zed AI")
		FName PatrolTag;

	UFUNCTION(NetMulticast, Unreliable, WithValidation, BlueprintCallable, Category = "Zed AI")
		void ModifyWalkSpeed(float MaxWalkSpeed);

	UFUNCTION(NetMulticast, WithValidation, Unreliable, BlueprintCallable, Category = "Zed AI|FX")
		void PlayAttackFX();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Zed AI")
		float Health = 100.f;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Zed AI")
		bool IsDead = false;

	

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
public:	
	FORCEINLINE float GetHealth() { return Health; };
	FORCEINLINE bool GetIsDead() { return IsDead; };
};
