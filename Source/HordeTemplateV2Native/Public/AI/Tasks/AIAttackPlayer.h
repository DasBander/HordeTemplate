// Copyright (c) 2018 - 2026 Marc Fraedrich
// Horde Template V2 (Native) - A cooperative zombie survival game framework
// Licensed under the MIT License

#pragma once

/**
 * @file AIAttackPlayer.h
 * @brief Behavior tree task for zombie melee attacks with wind-up delay and cooldown system
 * @author Marc Fraedrich
 */

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "AIAttackPlayer.generated.h"

/**
 * AI Task to attack the player with cooldown and wind-up delay.
 */
UCLASS()
class HORDETEMPLATEV2NATIVE_API UAIAttackPlayer : public UBTTaskNode
{
	GENERATED_BODY()

public:

	UAIAttackPlayer();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

protected:

	// Internal state tracking for wind-up
	struct FAttackTaskMemory
	{
		bool bWindupComplete = false;
		float WindupTimer = 0.f;
		float WindupDuration = 0.f;
		bool bAttackExecuted = false;
	};

	virtual uint16 GetInstanceMemorySize() const override { return sizeof(FAttackTaskMemory); }

	// Performs the actual attack damage
	void PerformAttack(UBehaviorTreeComponent& OwnerComp);
};
