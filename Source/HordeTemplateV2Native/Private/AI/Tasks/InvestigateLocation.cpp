

#include "InvestigateLocation.h"
#include "AI/ZedPawn.h"
#include "AI/ZedAIController.h"
#include "AIModule/Classes/BehaviorTree/BlackboardComponent.h"
#include "AIModule/Classes/AIController.h"
#include "NavigationSystem.h"
#include "HordeTemplateV2Native.h"


/**
 * Constructor
 *
 * @param
 * @return
 */
UInvestigateLocation::UInvestigateLocation()
{
	NodeName = "Investigate Location";
	bNotifyTick = true;

	MinInvestigateTime = ZED_INVESTIGATE_DURATION_MIN;
	MaxInvestigateTime = ZED_INVESTIGATE_DURATION_MAX;
}


/**
 * Executes the investigate task - moves AI to last known enemy location.
 *
 * @param OwnerComp - Owning Behavior Tree Component
 * @param NodeMemory - Node memory pointer
 * @return EBTNodeResult::type
 */
EBTNodeResult::Type UInvestigateLocation::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	FInvestigateTaskMemory* Memory = reinterpret_cast<FInvestigateTaskMemory*>(NodeMemory);

	AAIController* AIOwner = OwnerComp.GetAIOwner();
	if (!AIOwner)
	{
		return EBTNodeResult::Failed;
	}

	AZedAIController* ZedController = Cast<AZedAIController>(AIOwner);
	if (!ZedController)
	{
		return EBTNodeResult::Failed;
	}

	// Check if we have a location to investigate
	if (!ZedController->HasLastKnownEnemyLocation())
	{
		return EBTNodeResult::Failed;
	}

	// Initialize memory
	Memory->bHasReachedLocation = false;
	Memory->InvestigateTimer = 0.f;
	Memory->InvestigateDuration = FMath::FRandRange(MinInvestigateTime, MaxInvestigateTime);
	Memory->TargetLocation = ZedController->GetLastKnownEnemyLocation();

	// Ensure we're in alert state
	ZedController->SetAIState(EZedAIState::Alert);

	// Start moving to the location
	EPathFollowingRequestResult::Type MoveResult = AIOwner->MoveToLocation(
		Memory->TargetLocation,
		AcceptanceRadius,
		true,	// bStopOnOverlap
		true,	// bUsePathfinding
		false,	// bProjectDestinationToNavigation
		true,	// bCanStrafe
		nullptr,
		true	// bAllowPartialPath
	);

	if (MoveResult == EPathFollowingRequestResult::Failed)
	{
		// Can't reach location, clear it and return to patrol
		ZedController->ClearLastKnownEnemyLocation();
		ZedController->SetAIState(EZedAIState::Patrol);
		return EBTNodeResult::Failed;
	}

	return EBTNodeResult::InProgress;
}


/**
 * Tick the investigate task - handles arrival and looking around.
 *
 * @param OwnerComp - Owning Behavior Tree Component
 * @param NodeMemory - Node memory pointer
 * @param DeltaSeconds - Time since last tick
 * @return void
 */
void UInvestigateLocation::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	FInvestigateTaskMemory* Memory = reinterpret_cast<FInvestigateTaskMemory*>(NodeMemory);

	AAIController* AIOwner = OwnerComp.GetAIOwner();
	if (!AIOwner)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	AZedAIController* ZedController = Cast<AZedAIController>(AIOwner);
	AZedPawn* Zed = Cast<AZedPawn>(AIOwner->GetPawn());

	if (!ZedController || !Zed)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	// Check if an enemy was spotted during investigation
	UBlackboardComponent* BBC = OwnerComp.GetBlackboardComponent();
	if (BBC)
	{
		UObject* EnemyObj = BBC->GetValueAsObject("Enemy");
		if (EnemyObj != nullptr)
		{
			// Enemy spotted! Transition to chase
			ZedController->SetAIState(EZedAIState::Chase);
			FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
			return;
		}
	}

	// Check if we've reached the location
	if (!Memory->bHasReachedLocation)
	{
		float DistanceToTarget = FVector::Dist(Zed->GetActorLocation(), Memory->TargetLocation);
		if (DistanceToTarget <= AcceptanceRadius)
		{
			Memory->bHasReachedLocation = true;
			AIOwner->StopMovement();
		}
		else
		{
			// Check if movement is complete (path following finished)
			EPathFollowingStatus::Type MoveStatus = AIOwner->GetMoveStatus();
			if (MoveStatus == EPathFollowingStatus::Idle)
			{
				// Movement completed (reached as close as possible)
				Memory->bHasReachedLocation = true;
			}
		}
	}

	// If we've reached the location, look around
	if (Memory->bHasReachedLocation)
	{
		Memory->InvestigateTimer += DeltaSeconds;

		// Optional: Rotate to look around (simple rotation)
		float LookAngle = FMath::Sin(Memory->InvestigateTimer * 2.f) * 45.f;
		FRotator NewRotation = Zed->GetActorRotation();
		NewRotation.Yaw += LookAngle * DeltaSeconds;
		Zed->SetActorRotation(NewRotation);

		// Check if investigation time is complete
		if (Memory->InvestigateTimer >= Memory->InvestigateDuration)
		{
			// Investigation complete, clear location and return to patrol
			ZedController->ClearLastKnownEnemyLocation();
			ZedController->SetAIState(EZedAIState::Patrol);
			FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
			return;
		}
	}
}
