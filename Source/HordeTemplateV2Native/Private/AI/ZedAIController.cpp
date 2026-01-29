// Copyright (c) 2018 - 2026 Marc Fraedrich
// Horde Template V2 (Native) - A cooperative zombie survival game framework
// Licensed under the MIT License

#include "ZedAIController.h"
#include "ZedPawn.h"
#include "AIModule/Classes/BehaviorTree/BehaviorTree.h"
#include "HordeTemplateV2Native.h"
#include "Character/HordeBaseCharacter.h"
#include "AIModule/Classes/BehaviorTree/BlackboardComponent.h"
#include "Kismet/KismetSystemLibrary.h"

/**
 * @file ZedAIController.cpp
 * @brief Implementation of zombie AI controller with state machine, perception, and combat logic
 * @author Marc Fraedrich
 */

/**
 *	Constructor
 *
 * @param
 * @return
 */
AZedAIController::AZedAIController()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

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
 *	Tick - Updates LOS tracking timer
 */
void AZedAIController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Only run on server
	if (!HasAuthority())
	{
		return;
	}

	// Update LOS lost time tracking when in chase state
	if (CurrentAIState == EZedAIState::Chase)
	{
		if (!HasLineOfSightToEnemy())
		{
			TimeSinceLostLOS += DeltaTime;
		}
		else
		{
			TimeSinceLostLOS = 0.f;
		}
	}
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

		// Update last known enemy location
		SetLastKnownEnemyLocation(Enemy->GetActorLocation());

		// Reset LOS tracking
		TimeSinceLostLOS = 0.f;
		bHadLOSLastCheck = true;

		UBlackboardComponent* BB = GetBlackboardComponent();
		if (BB)
		{
			BB->SetValueAsObject("Enemy", Enemy);
			BB->SetValueAsVector("LastKnownEnemyLocation", LastKnownEnemyLocation);
		}

		// Transition to Chase state if not already attacking or staggered
		if (CurrentAIState != EZedAIState::Attack && CurrentAIState != EZedAIState::Staggered)
		{
			SetAIState(EZedAIState::Chase);
		}
	}
}


/**
 *	Sets Enemy as nullptr in Blackboard and transitions to Alert/Investigate state.
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

	// Transition to Alert state to investigate last known position
	if (bHasLastKnownEnemyLocation)
	{
		SetAIState(EZedAIState::Alert);
	}
	else
	{
		SetAIState(EZedAIState::Patrol);
	}
}


/**
 *	Begin Play -> Runs Behavior Tree on Server and starts LOS check timer.
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

		// Start periodic LOS check timer
		GetWorld()->GetTimerManager().SetTimer(LOSCheckTimer, this, &AZedAIController::CheckLineOfSight, ZED_LOS_CHECK_INTERVAL, true);
	}
}


/**
 *	Sets the current AI state and notifies pawn of state change.
 *
 * @param NewState - The new AI state to transition to
 * @return void
 */
void AZedAIController::SetAIState(EZedAIState NewState)
{
	if (CurrentAIState == NewState)
	{
		return;
	}

	// Don't allow state changes while staggered (except to Patrol/Idle when stagger ends)
	if (bIsStaggered && NewState != EZedAIState::Patrol && NewState != EZedAIState::Idle)
	{
		return;
	}

	EZedAIState OldState = CurrentAIState;
	PreviousAIState = CurrentAIState;
	CurrentAIState = NewState;

	// Update blackboard with current state
	UBlackboardComponent* BB = GetBlackboardComponent();
	if (BB)
	{
		BB->SetValueAsEnum("AIState", static_cast<uint8>(NewState));
	}

	OnStateChanged(OldState, NewState);
}


/**
 *	Returns the target movement speed for a given AI state.
 *
 * @param State - The AI state to get speed for
 * @return Target walk speed in units per second
 */
float AZedAIController::GetTargetSpeedForState(EZedAIState State) const
{
	switch (State)
	{
	case EZedAIState::Idle:
		return ZED_SPEED_IDLE;
	case EZedAIState::Patrol:
		return ZED_SPEED_PATROL;
	case EZedAIState::Alert:
		return ZED_SPEED_ALERT;
	case EZedAIState::Chase:
	case EZedAIState::Attack:
		return ZED_SPEED_CHASE;
	case EZedAIState::Staggered:
		return ZED_SPEED_PATROL * ZED_STAGGER_SPEED_MULTIPLIER;
	default:
		return ZED_SPEED_PATROL;
	}
}


/**
 *	Called when the AI state changes.
 *
 * @param OldState - Previous AI state
 * @param NewState - New AI state
 * @return void
 */
void AZedAIController::OnStateChanged(EZedAIState OldState, EZedAIState NewState)
{
	AZedPawn* ZedPawn = Cast<AZedPawn>(GetPawn());
	if (ZedPawn)
	{
		// Update target speed based on new state
		float TargetSpeed = GetTargetSpeedForState(NewState);

		// Apply anger multiplier if active
		if (bIsAngered)
		{
			TargetSpeed *= ZED_ANGER_SPEED_MULTIPLIER;
		}

		ZedPawn->SetTargetWalkSpeed(TargetSpeed);

		// Play alert sound when first spotting enemy
		if (OldState == EZedAIState::Patrol && (NewState == EZedAIState::Alert || NewState == EZedAIState::Chase))
		{
			ZedPawn->PlayAlertSound();
		}

		// Start/stop chase breathing
		if (NewState == EZedAIState::Chase)
		{
			ZedPawn->StartChaseBreathing();
		}
		else if (OldState == EZedAIState::Chase)
		{
			ZedPawn->StopChaseBreathing();
		}
	}
}


/**
 *	Sets the last known enemy location for investigation.
 *
 * @param Location - World location where enemy was last seen
 * @return void
 */
void AZedAIController::SetLastKnownEnemyLocation(const FVector& Location)
{
	LastKnownEnemyLocation = Location;
	bHasLastKnownEnemyLocation = true;

	UBlackboardComponent* BB = GetBlackboardComponent();
	if (BB)
	{
		BB->SetValueAsVector("LastKnownEnemyLocation", Location);
	}
}


/**
 *	Clears the last known enemy location.
 *
 * @return void
 */
void AZedAIController::ClearLastKnownEnemyLocation()
{
	bHasLastKnownEnemyLocation = false;
	LastKnownEnemyLocation = FVector::ZeroVector;

	UBlackboardComponent* BB = GetBlackboardComponent();
	if (BB)
	{
		BB->ClearValue("LastKnownEnemyLocation");
	}
}


/**
 *	Checks if the AI has line of sight to the current enemy.
 *
 * @return True if enemy is visible
 */
bool AZedAIController::HasLineOfSightToEnemy() const
{
	const UBlackboardComponent* BB = GetBlackboardComponent();
	if (!BB)
	{
		return false;
	}

	AHordeBaseCharacter* Enemy = Cast<AHordeBaseCharacter>(BB->GetValueAsObject("Enemy"));
	if (!Enemy || Enemy->GetIsDead())
	{
		return false;
	}

	APawn* ControlledPawn = GetPawn();
	if (!ControlledPawn)
	{
		return false;
	}

	// Perform line trace from pawn eyes to enemy
	FVector StartLocation = ControlledPawn->GetPawnViewLocation();
	FVector EndLocation = Enemy->GetActorLocation() + FVector(0.f, 0.f, 50.f); // Aim at chest height

	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(ControlledPawn);
	QueryParams.AddIgnoredActor(Enemy);

	bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECC_Visibility, QueryParams);

	// If nothing was hit, we have clear LOS
	return !bHit;
}


/**
 *	Periodic check for line of sight to enemy.
 *
 * @return void
 */
void AZedAIController::CheckLineOfSight()
{
	if (CurrentAIState != EZedAIState::Chase)
	{
		return;
	}

	UBlackboardComponent* BB = GetBlackboardComponent();
	if (!BB)
	{
		return;
	}

	AHordeBaseCharacter* Enemy = Cast<AHordeBaseCharacter>(BB->GetValueAsObject("Enemy"));
	if (!Enemy)
	{
		return;
	}

	bool bHasLOS = HasLineOfSightToEnemy();

	if (bHasLOS)
	{
		// Update last known location while we can see them
		SetLastKnownEnemyLocation(Enemy->GetActorLocation());
		TimeSinceLostLOS = 0.f;
	}

	// Check if we've lost LOS for too long
	if (!bHasLOS && TimeSinceLostLOS >= ZED_LOS_LOST_THRESHOLD)
	{
		// Transition to alert state to investigate
		SetAIState(EZedAIState::Alert);
		BB->SetValueAsObject("Enemy", nullptr);
	}

	bHadLOSLastCheck = bHasLOS;
}


/**
 *	Applies stagger effect, temporarily reducing movement speed.
 *
 * @return void
 */
void AZedAIController::ApplyStagger()
{
	if (bIsStaggered)
	{
		return;
	}

	bIsStaggered = true;
	SetAIState(EZedAIState::Staggered);

	// Clear any existing stagger timer
	GetWorld()->GetTimerManager().ClearTimer(StaggerTimer);
	GetWorld()->GetTimerManager().SetTimer(StaggerTimer, this, &AZedAIController::EndStagger, ZED_STAGGER_DURATION, false);
}


/**
 *	Ends the stagger effect and returns to previous state.
 *
 * @return void
 */
void AZedAIController::EndStagger()
{
	bIsStaggered = false;

	// Return to previous state or patrol
	UBlackboardComponent* BB = GetBlackboardComponent();
	if (BB)
	{
		AHordeBaseCharacter* Enemy = Cast<AHordeBaseCharacter>(BB->GetValueAsObject("Enemy"));
		if (Enemy && !Enemy->GetIsDead())
		{
			SetAIState(EZedAIState::Chase);
		}
		else if (bHasLastKnownEnemyLocation)
		{
			SetAIState(EZedAIState::Alert);
		}
		else
		{
			SetAIState(EZedAIState::Patrol);
		}
	}
	else
	{
		SetAIState(EZedAIState::Patrol);
	}
}


/**
 *	Applies anger burst effect, temporarily increasing movement speed.
 *
 * @return void
 */
void AZedAIController::ApplyAngerBurst()
{
	if (bIsStaggered)
	{
		return; // Can't get angry while staggered
	}

	bIsAngered = true;

	// Apply anger speed boost
	AZedPawn* ZedPawn = Cast<AZedPawn>(GetPawn());
	if (ZedPawn)
	{
		float BaseSpeed = GetTargetSpeedForState(CurrentAIState);
		ZedPawn->SetTargetWalkSpeed(BaseSpeed * ZED_ANGER_SPEED_MULTIPLIER);
	}

	// Clear any existing anger timer
	GetWorld()->GetTimerManager().ClearTimer(AngerTimer);
	GetWorld()->GetTimerManager().SetTimer(AngerTimer, this, &AZedAIController::EndAnger, ZED_ANGER_DURATION, false);
}


/**
 *	Ends the anger burst effect.
 *
 * @return void
 */
void AZedAIController::EndAnger()
{
	bIsAngered = false;

	// Return to normal speed for current state
	AZedPawn* ZedPawn = Cast<AZedPawn>(GetPawn());
	if (ZedPawn)
	{
		ZedPawn->SetTargetWalkSpeed(GetTargetSpeedForState(CurrentAIState));
	}
}
