

#include "AIAttackPlayer.h"
#include "AI/ZedPawn.h"
#include "AIModule/Classes/BehaviorTree/BlackboardComponent.h"
#include "AIModule/Classes/AIController.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Character/HordeBaseCharacter.h"
#include "Kismet/GameplayStatics.h"


/**
 * Constructor
 *
 * @param
 * @return
 */
UAIAttackPlayer::UAIAttackPlayer()
{
	NodeName = "Attack Player";
}


/**
 * Executes Task to Attack the Player. Traces Player with SphereTraceSingle and applys damage to him.
 *
 * @param Owning Behavior Tree Component and the node memory
 * @return EBTNodeResult::type 
 */
EBTNodeResult::Type UAIAttackPlayer::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::ExecuteTask(OwnerComp, NodeMemory);

	UBlackboardComponent* BBC = Cast<UBlackboardComponent>(OwnerComp.GetBlackboardComponent());
	// Fixed: Added null check for GetAIOwner() before accessing GetPawn()
	AAIController* AIOwner = OwnerComp.GetAIOwner();
	if (BBC && AIOwner)
	{
		AZedPawn* Zed = Cast<AZedPawn>(AIOwner->GetPawn());
		if (Zed && !Zed->GetIsDead())
		{
			TArray<AActor*> ActorsToIgnore = { Zed };
			FHitResult OutResult;
			if (UKismetSystemLibrary::SphereTraceSingle(GetWorld(), Zed->AttackPoint->GetComponentLocation(), (Zed->AttackPoint->GetForwardVector() * 150.f) + Zed->AttackPoint->GetComponentLocation(), 16.f, ETraceTypeQuery::TraceTypeQuery15, false, ActorsToIgnore, EDrawDebugTrace::None, OutResult, true)) 
			{
				AHordeBaseCharacter* Char = Cast<AHordeBaseCharacter>(OutResult.GetActor());
				if (Char && !Char->GetIsDead())
				{
					UGameplayStatics::ApplyPointDamage(Char, FMath::RandRange(5.f, 9.f), Zed->GetActorLocation(), OutResult, AIOwner, Zed, nullptr);
					Zed->PlayAttackFX();
				}
			}
		}
	}
	return EBTNodeResult::Succeeded;
}
