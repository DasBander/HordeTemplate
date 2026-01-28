

#pragma once

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
