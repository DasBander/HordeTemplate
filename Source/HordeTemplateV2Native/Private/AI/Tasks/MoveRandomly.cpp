// Copyright (c) 2018 - 2026 Marc Fraedrich
// Horde Template V2 (Native) - A cooperative zombie survival game framework
// Licensed under the MIT License

#include "MoveRandomly.h"
#include "AIController.h"
#include "GameFramework/Pawn.h"
#include "NavigationSystem/Public/NavigationSystem.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Navigation/PathFollowingComponent.h"

/**
 * @file MoveRandomly.cpp
 * @brief Implementation of random patrol movement behavior tree task
 * @author Marc Fraedrich
 */

/**
 *	Constructor
 *
 * @param
 * @return
 */
UMoveRandomly::UMoveRandomly()
{
	NodeName = "Zed Move Randomly";
	bNotifyTick = true;
}


/**
 * Checks if Enemy is Set or Moving is finished. If yes it finishes the latent Task.
 *
 * @param Owning Behavior Tree Component the Node Memory and the Delta Seconds
 * @return void
 */
void UMoveRandomly::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);

	AAIController* MyController = Cast<AAIController>(OwnerComp.GetAIOwner());
	if (MyController)
	{
		UBlackboardComponent* BCP = Cast<UBlackboardComponent>(OwnerComp.GetBlackboardComponent());
		if (BCP)
		{
			if (BCP->GetValueAsObject("Enemy") != nullptr)
			{
				FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
			}
		}
		if (MyController->GetMoveStatus() == EPathFollowingStatus::Idle)
		{
			FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		}
	}

}


/**
 *	Searches for random reachable point in radius and moves up there.
 *
 * @param Owning BehaviorTreeComponent and the Node Memory
 * @return EBTNodeResult::Type
 */
EBTNodeResult::Type UMoveRandomly::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::ExecuteTask(OwnerComp, NodeMemory);

	AAIController* MyController = Cast<AAIController>(OwnerComp.GetAIOwner());
	if (MyController)
	{
		APawn* Pawn = Cast<APawn>(MyController->GetPawn());
		if (Pawn)
		{
			if (MyController->GetMoveStatus() == EPathFollowingStatus::Idle)
			{
				FAIMoveRequest MoveRequest;
				FVector LocationToMove;
				UNavigationSystemV1::K2_GetRandomReachablePointInRadius(GetWorld(), Pawn->GetActorLocation(), LocationToMove, 500.f, nullptr, nullptr);
				MoveRequest.SetGoalLocation(LocationToMove);
				MyController->MoveTo(MoveRequest);
				return EBTNodeResult::InProgress;
			}
		}
	
	}
	return EBTNodeResult::Failed;
}
