

#include "MoveToPatrolPoint.h"
#include "EngineUtils.h"
#include "AI/AICorePoint.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Navigation/PathFollowingComponent.h"
#include "AIController.h"

/**
 *	Constructor
 *
 * @param
 * @return
 */
UMoveToPatrolPoint::UMoveToPatrolPoint()
{
	NodeName = "Move to Patrol Point";
	bNotifyTick = true;
}


/**
 *	Finishes latent Task if Enemy is Set ort Move Status is Idle. Also clears Patrol Tag;
 *
 * @param	Owning BehaviorTreeComponent the NodeMemory and Delta Seconds.
 * @return void
 */
void UMoveToPatrolPoint::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
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
			if (MyController->GetMoveStatus() == EPathFollowingStatus::Idle)
			{
				FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
			}
			BCP->SetValueAsName("PatrolTag", NAME_None);
		}
	
	}
}


/**
 *	Moves to Patrol Tag Location.
 *
 * @param Owning BehaviorTreeComponent and NodeMemory
 * @return EBTNodeResult::Type
 */
EBTNodeResult::Type UMoveToPatrolPoint::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{

	AAIController* MyController = Cast<AAIController>(OwnerComp.GetAIOwner());
	if (MyController)
	{
		UBlackboardComponent* BCP = Cast<UBlackboardComponent>(OwnerComp.GetBlackboardComponent());
		if (BCP)
		{
			if (MyController->GetMoveStatus() == EPathFollowingStatus::Idle)
			{
				FAIMoveRequest MoveRequest;
				FVector LocationToMove = GetPatrolLocation(BCP->GetValueAsName("PatrolTag"));
				MoveRequest.SetGoalLocation(LocationToMove);
				MyController->MoveTo(MoveRequest);
				return EBTNodeResult::InProgress;
			}
		}
	}
	return EBTNodeResult::Failed;
}


/**
 *	Returns random Patrol Location of AICorePoints by given Patrol Tag.
 *
 * @param The Patrol Tag
 * @return Random Patrol Location as FVector.
 */
FVector UMoveToPatrolPoint::GetPatrolLocation(FName PatrolTag)
{
	TArray<FVector> PatrolLocations;
	for(TActorIterator<AAICorePoint> ActorItr(GetWorld()); ActorItr; ++ActorItr)
	{
		AAICorePoint* CorePoint = *ActorItr;
		if (CorePoint && CorePoint->PatrolTag == PatrolTag)
		{
			PatrolLocations.Add(CorePoint->GetActorLocation());
		}
	}
	// Fixed: Check for empty array before RandRange to avoid undefined behavior (RandRange(0, -1))
	if (PatrolLocations.Num() == 0)
	{
		return FVector::ZeroVector;
	}
	int32 RetVal = FMath::RandRange(0, PatrolLocations.Num() - 1);
	return PatrolLocations[RetVal];
}
