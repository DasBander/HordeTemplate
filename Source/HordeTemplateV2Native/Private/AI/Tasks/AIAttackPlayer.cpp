

#include "AIAttackPlayer.h"
#include "AI/ZedPawn.h"
#include "AI/ZedAIController.h"
#include "AIModule/Classes/BehaviorTree/BlackboardComponent.h"
#include "AIModule/Classes/AIController.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Character/HordeBaseCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "HordeTemplateV2Native.h"


/**
 * Constructor
 *
 * @param
 * @return
 */
UAIAttackPlayer::UAIAttackPlayer()
{
	NodeName = "Attack Player";
	bNotifyTick = true;
}


/**
 * Executes Task to Attack the Player. Starts wind-up before dealing damage.
 *
 * @param Owning Behavior Tree Component and the node memory
 * @return EBTNodeResult::type
 */
EBTNodeResult::Type UAIAttackPlayer::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::ExecuteTask(OwnerComp, NodeMemory);

	FAttackTaskMemory* Memory = reinterpret_cast<FAttackTaskMemory*>(NodeMemory);

	AAIController* AIOwner = OwnerComp.GetAIOwner();
	if (!AIOwner)
	{
		return EBTNodeResult::Failed;
	}

	AZedPawn* Zed = Cast<AZedPawn>(AIOwner->GetPawn());
	if (!Zed || Zed->GetIsDead())
	{
		return EBTNodeResult::Failed;
	}

	// Check cooldown - if not ready to attack, fail immediately
	if (!Zed->CanAttack())
	{
		return EBTNodeResult::Failed;
	}

	// Check if we have a valid target
	UBlackboardComponent* BBC = OwnerComp.GetBlackboardComponent();
	if (!BBC)
	{
		return EBTNodeResult::Failed;
	}

	AHordeBaseCharacter* Enemy = Cast<AHordeBaseCharacter>(BBC->GetValueAsObject("Enemy"));
	if (!Enemy || Enemy->GetIsDead())
	{
		return EBTNodeResult::Failed;
	}

	// Initialize wind-up
	Memory->bWindupComplete = false;
	Memory->WindupTimer = 0.f;
	Memory->WindupDuration = FMath::FRandRange(ZED_ATTACK_WINDUP_MIN, ZED_ATTACK_WINDUP_MAX);
	Memory->bAttackExecuted = false;

	// Set AI to attack state
	AZedAIController* ZedController = Cast<AZedAIController>(AIOwner);
	if (ZedController)
	{
		ZedController->SetAIState(EZedAIState::Attack);
	}

	// Play attack animation immediately (wind-up is animation start)
	Zed->PlayAttackFX();

	return EBTNodeResult::InProgress;
}


/**
 * Tick the attack task - handles wind-up timing before damage.
 *
 * @param OwnerComp - Owning Behavior Tree Component
 * @param NodeMemory - Node memory pointer
 * @param DeltaSeconds - Time since last tick
 * @return void
 */
void UAIAttackPlayer::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	FAttackTaskMemory* Memory = reinterpret_cast<FAttackTaskMemory*>(NodeMemory);

	// Already executed the attack, wait briefly then complete
	if (Memory->bAttackExecuted)
	{
		AAIController* AIOwner = OwnerComp.GetAIOwner();
		AZedAIController* ZedController = Cast<AZedAIController>(AIOwner);
		if (ZedController)
		{
			// Return to chase state after attack
			ZedController->SetAIState(EZedAIState::Chase);
		}
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		return;
	}

	// Update wind-up timer
	Memory->WindupTimer += DeltaSeconds;

	// Check if wind-up is complete
	if (Memory->WindupTimer >= Memory->WindupDuration)
	{
		Memory->bWindupComplete = true;
		PerformAttack(OwnerComp);
		Memory->bAttackExecuted = true;
	}
}


/**
 * Performs the actual attack damage.
 *
 * @param OwnerComp - Owning Behavior Tree Component
 * @return void
 */
void UAIAttackPlayer::PerformAttack(UBehaviorTreeComponent& OwnerComp)
{
	UBlackboardComponent* BBC = Cast<UBlackboardComponent>(OwnerComp.GetBlackboardComponent());
	AAIController* AIOwner = OwnerComp.GetAIOwner();

	if (!BBC || !AIOwner)
	{
		return;
	}

	AZedPawn* Zed = Cast<AZedPawn>(AIOwner->GetPawn());
	if (!Zed || Zed->GetIsDead())
	{
		return;
	}

	// Perform sphere trace for attack
	TArray<AActor*> ActorsToIgnore = { Zed };
	FHitResult OutResult;
	if (UKismetSystemLibrary::SphereTraceSingle(
		GetWorld(),
		Zed->AttackPoint->GetComponentLocation(),
		(Zed->AttackPoint->GetForwardVector() * 150.f) + Zed->AttackPoint->GetComponentLocation(),
		16.f,
		ETraceTypeQuery::TraceTypeQuery15,
		false,
		ActorsToIgnore,
		EDrawDebugTrace::None,
		OutResult,
		true))
	{
		AHordeBaseCharacter* Char = Cast<AHordeBaseCharacter>(OutResult.GetActor());
		if (Char && !Char->GetIsDead())
		{
			// Apply varied damage
			float Damage = FMath::FRandRange(ZED_ATTACK_DAMAGE_MIN, ZED_ATTACK_DAMAGE_MAX);
			UGameplayStatics::ApplyPointDamage(Char, Damage, Zed->GetActorLocation(), OutResult, AIOwner, Zed, nullptr);
		}
	}

	// Reset attack cooldown
	Zed->ResetAttackCooldown();
}
