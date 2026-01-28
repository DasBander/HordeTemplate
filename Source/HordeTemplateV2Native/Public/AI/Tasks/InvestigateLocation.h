

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "InvestigateLocation.generated.h"

/**
 * AI Task to investigate the last known enemy location.
 * Moves to the location, looks around, then returns to patrol.
 */
UCLASS()
class HORDETEMPLATEV2NATIVE_API UInvestigateLocation : public UBTTaskNode
{
	GENERATED_BODY()

public:

	UInvestigateLocation();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

protected:

	// How close the AI needs to get to the investigate location
	UPROPERTY(EditAnywhere, Category = "Investigation")
		float AcceptanceRadius = 100.f;

	// Minimum time to look around at the location
	UPROPERTY(EditAnywhere, Category = "Investigation")
		float MinInvestigateTime = 2.f;

	// Maximum time to look around at the location
	UPROPERTY(EditAnywhere, Category = "Investigation")
		float MaxInvestigateTime = 3.f;

private:

	// Internal state tracking
	struct FInvestigateTaskMemory
	{
		bool bHasReachedLocation = false;
		float InvestigateTimer = 0.f;
		float InvestigateDuration = 0.f;
		FVector TargetLocation = FVector::ZeroVector;
	};

	virtual uint16 GetInstanceMemorySize() const override { return sizeof(FInvestigateTaskMemory); }
};
