

#include "IsEnemyDead.h"
#include "Character/HordeBaseCharacter.h"
#include "AI/ZedPawn.h"
#include "AI/ZedAIController.h"
#include "AIModule/Classes/BehaviorTree/BlackboardComponent.h"

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
 *
 * @param Owner Behavior Tree Component the node memory and delta seconds
 * @return void
 */
void UIsEnemyDead::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	UBlackboardComponent* BBC = Cast<UBlackboardComponent>(OwnerComp.GetBlackboardComponent());
	// Fixed: Added null check for GetAIOwner() before accessing GetPawn() or SetFocus()
	AAIController* AIOwner = OwnerComp.GetAIOwner();
	if (BBC && AIOwner)
	{
		AHordeBaseCharacter* PLY = Cast<AHordeBaseCharacter>(BBC->GetValueAsObject("Enemy"));
		AZedPawn* Zed = Cast<AZedPawn>(AIOwner->GetPawn());
		if (PLY && PLY->GetIsDead())
		{
			BBC->SetValueAsObject("Enemy", nullptr);
			if (Zed && Zed->GetCharacterMovement()->MaxWalkSpeed != 200.f)
			{
				Zed->ModifyWalkSpeed(200.f);
			}
			AIOwner->SetFocus(nullptr);
		}
		else if(PLY && !PLY->GetIsDead())
		{
			if (Zed && Zed->GetCharacterMovement()->MaxWalkSpeed != 500.f)
			{
				Zed->ModifyWalkSpeed(500.f);
			}
		}
	}
}
