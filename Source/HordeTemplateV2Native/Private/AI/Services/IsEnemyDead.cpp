

#include "IsEnemyDead.h"
#include "Character/HordeBaseCharacter.h"
#include "AI/ZedPawn.h"
#include "AI/ZedAIController.h"
#include "AIModule/Classes/BehaviorTree/BlackboardComponent.h"
#include "HordeTemplateV2Native.h"

/**
 * Constructor
 *
 * @param
 * @return
 */
UIsEnemyDead::UIsEnemyDead()
{
	NodeName = "Check: Is Enemy Dead?";
	Interval = 0.5f;
	RandomDeviation = 0.1f;
}


/**
 * Tick Node which checks if Enemy is Dead ( Player Character ). If zombie killed player he ignores him.
 * Also handles state-based speed management and LOS tracking.
 *
 * @param Owner Behavior Tree Component the node memory and delta seconds
 * @return void
 */
void UIsEnemyDead::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	UBlackboardComponent* BBC = Cast<UBlackboardComponent>(OwnerComp.GetBlackboardComponent());
	AAIController* AIOwner = OwnerComp.GetAIOwner();
	if (!BBC || !AIOwner)
	{
		return;
	}

	AZedAIController* ZedController = Cast<AZedAIController>(AIOwner);
	AZedPawn* Zed = Cast<AZedPawn>(AIOwner->GetPawn());
	AHordeBaseCharacter* PLY = Cast<AHordeBaseCharacter>(BBC->GetValueAsObject("Enemy"));

	if (!Zed)
	{
		return;
	}

	// Check if enemy is dead
	if (PLY && PLY->GetIsDead())
	{
		BBC->SetValueAsObject("Enemy", nullptr);
		AIOwner->SetFocus(nullptr);

		if (ZedController)
		{
			// Check if we have a last known location to investigate
			if (ZedController->HasLastKnownEnemyLocation())
			{
				ZedController->SetAIState(EZedAIState::Alert);
			}
			else
			{
				ZedController->SetAIState(EZedAIState::Patrol);
			}
		}
		return;
	}

	// Handle active enemy - update state based on presence
	if (PLY && !PLY->GetIsDead() && ZedController)
	{
		EZedAIState CurrentState = ZedController->GetAIState();

		// Update last known location while we have an enemy
		ZedController->SetLastKnownEnemyLocation(PLY->GetActorLocation());

		// If not already chasing/attacking/staggered, transition to chase
		if (CurrentState == EZedAIState::Patrol || CurrentState == EZedAIState::Alert || CurrentState == EZedAIState::Idle)
		{
			ZedController->SetAIState(EZedAIState::Chase);
		}

		// Check line of sight during chase
		if (CurrentState == EZedAIState::Chase)
		{
			if (!ZedController->HasLineOfSightToEnemy())
			{
				// LOS lost - check if we've lost it for too long
				if (ZedController->GetTimeSinceLostLOS() >= ZED_LOS_LOST_THRESHOLD)
				{
					// Transition to alert state, clear enemy, investigate last known location
					BBC->SetValueAsObject("Enemy", nullptr);
					AIOwner->SetFocus(nullptr);
					ZedController->SetAIState(EZedAIState::Alert);
				}
			}
		}
	}

	// No enemy - ensure we're in patrol or alert state
	if (!PLY && ZedController)
	{
		EZedAIState CurrentState = ZedController->GetAIState();

		if (CurrentState == EZedAIState::Chase)
		{
			// We were chasing but lost target - investigate
			if (ZedController->HasLastKnownEnemyLocation())
			{
				ZedController->SetAIState(EZedAIState::Alert);
			}
			else
			{
				ZedController->SetAIState(EZedAIState::Patrol);
			}

			AIOwner->SetFocus(nullptr);
		}
	}
}
