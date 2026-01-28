

#include "ZedAIController.h"
#include "AIModule/Classes/BehaviorTree/BehaviorTree.h"
#include "HordeTemplateV2Native.h"
#include "Character/HordeBaseCharacter.h"
#include "AIModule/Classes/BehaviorTree/BlackboardComponent.h"


/**
 *	Constructor
 *
 * @param
 * @return
 */
AZedAIController::AZedAIController()
{
	PCC = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("Perception Component"));

	UAISenseConfig_Sight* sightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("Sight Config"));
	sightConfig->DetectionByAffiliation.bDetectEnemies = true;
	sightConfig->DetectionByAffiliation.bDetectFriendlies = true;
	sightConfig->DetectionByAffiliation.bDetectNeutrals = true;
	sightConfig->SightRadius = ZED_SIGHT_RADIUS;
	sightConfig->LoseSightRadius = ZED_LOSE_SIGHT_RADIUS;
	sightConfig->SetMaxAge(10.f);

	PCC->ConfigureSense(*sightConfig);
	PCC->SetDominantSense(sightConfig->GetSenseImplementation());
	PCC->OnTargetPerceptionUpdated.AddDynamic(this, &AZedAIController::EnemyInSight);
}


/**
 *	Detects Enemy Range Bases and sets it inside the Blackboard. Also starts clear sight timer in RandomRange Time Zone.
 *
 * @param The Enemy Actor and AI Stimulus
 * @return void
 */
void AZedAIController::EnemyInSight(AActor* Actor, FAIStimulus Stimulus)
{
	AHordeBaseCharacter* Enemy = Cast<AHordeBaseCharacter>(Actor);
	if (Enemy && !Enemy->GetIsDead())
	{
		// Fixed: Always set/reset the sight clear timer when enemy is detected, not only when already active
		// The previous logic only reset the timer if it was already running, meaning it wouldn't set the timer on first sight
		GetWorld()->GetTimerManager().SetTimer(SightClearTimer, this, &AZedAIController::ClearSight, FMath::FRandRange(ZED_LOSE_SIGHT_TIME_MIN, ZED_LOSE_SIGHT_TIME_MAX), false);

		UBlackboardComponent* BB = GetBlackboardComponent();
		if (BB)
		{
			BB->SetValueAsObject("Enemy", Enemy);
		}
	}
}


/**
 *	Sets Enemy as nullptr in Blackboard.
 *
 * @param
 * @return void
 */
void AZedAIController::ClearSight()
{
	UBlackboardComponent* BB = GetBlackboardComponent();
	if (BB)
	{
		BB->SetValueAsObject("Enemy", nullptr);
	}
}


/**
 *	Begin Play -> Runs Behavior Tree on Server.
 *
 * @param
 * @return void
 */
void AZedAIController::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		UBehaviorTree* AITree = ObjectFromPath<UBehaviorTree>(ZED_BEHAVIORTREE_ASSET_PATH);
		if (AITree)
		{
			RunBehaviorTree(AITree);
		}

	}
}
