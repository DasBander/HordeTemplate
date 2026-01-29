// Copyright (c) 2018 - 2026 Marc Fraedrich
// Horde Template V2 (Native) - A cooperative zombie survival game framework
// Licensed under the MIT License

#include "ZedPawn.h"
#include "ZedAIController.h"
#include "Gameplay/HordeGameState.h"
#include "Materials/MaterialInstanceConstant.h"
#include "Character/HordeBaseCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimBlueprintGeneratedClass.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "Sound/SoundCue.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Engine/DamageEvents.h"
#include "HordeTemplateV2Native.h"

/**
 * @file ZedPawn.cpp
 * @brief Implementation of the zombie pawn with movement, damage handling, and visual/audio effects
 * @author Marc Fraedrich
 */

/**
 *	Constructor
 *
 * @param
 * @return
 */
AZedPawn::AZedPawn()
{
	// Enable tick for smooth speed interpolation
	PrimaryActorTick.bStartWithTickEnabled = true;
	PrimaryActorTick.bCanEverTick = true;
	SetReplicates(true);

	const ConstructorHelpers::FObjectFinder<USkeletalMesh> PlayerMeshAsset(TEXT("SkeletalMesh'/Game/HordeTemplateBP/Assets/Mannequin/Character/Mesh/SK_Mannequin.SK_Mannequin'"));
	if (PlayerMeshAsset.Succeeded()) {
		GetMesh()->SetSkeletalMesh(PlayerMeshAsset.Object);
		GetMesh()->SetRelativeLocation(FVector(0.f, 0.f, -90.f));
		GetMesh()->SetRelativeRotation(FRotator(0.f, -90.f, 0.f).Quaternion());
		GetMesh()->SetCollisionProfileName(FName(TEXT("Zed")));
	}

	// static ConstructorHelpers::FClassFinder<UAnimInstance> AnimBP(TEXT("/Game/HordeTemplateBP/Assets/Animations/Zombie/ABP_Zombie"));
	// if (AnimBP.Succeeded() && GetMesh())
	// {
	// 	GetMesh()->SetAnimInstanceClass(AnimBP.Class);
	// }

	const ConstructorHelpers::FObjectFinder<UMaterialInstanceConstant> ZedCharacterMaterial(TEXT("MaterialInstanceConstant'/Game/HordeTemplateBP/Assets/Mannequin/Character/Materials/M_UE4Man_Zombie.M_UE4Man_Zombie'"));
	if (ZedCharacterMaterial.Succeeded())
	{
		GetMesh()->SetMaterial(0, ZedCharacterMaterial.Object);
	}

	AttackPoint = CreateDefaultSubobject<UArrowComponent>(TEXT("AttackPoint"));
	AttackPoint->SetupAttachment(RootComponent);
	AttackPoint->SetRelativeLocation(FVector(0.f, 0.f, 56.f));

	ZedIdleSound = CreateDefaultSubobject<UAudioComponent>(TEXT("Zed Idle Sound"));
	ZedIdleSound->SetupAttachment(RootComponent);

	const ConstructorHelpers::FObjectFinder<USoundCue> ZedIdleSoundAsset(TEXT("SoundCue'/Game/HordeTemplateBP/Assets/Sounds/A_Zed_RND_Idle.A_Zed_RND_Idle'"));
	if (ZedIdleSoundAsset.Succeeded())
	{
		ZedIdleSound->SetSound(ZedIdleSoundAsset.Object);
	}

	// Chase breathing audio component
	ZedChaseBreathing = CreateDefaultSubobject<UAudioComponent>(TEXT("Zed Chase Breathing"));
	ZedChaseBreathing->SetupAttachment(RootComponent);
	ZedChaseBreathing->bAutoActivate = false;

	PlayerRangeCollision = CreateDefaultSubobject<USphereComponent>(TEXT("Player Collision Sphere"));
	PlayerRangeCollision->SetupAttachment(RootComponent);
	PlayerRangeCollision->SetRelativeLocation(FVector(74.f, 0.f, 0.f));
	PlayerRangeCollision->SetRelativeScale3D(FVector(3.8125f, 4.75f, 7.75f));
	PlayerRangeCollision->SetCollisionProfileName("CharacterDetection");
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	AIControllerClass = AZedAIController::StaticClass();

	PlayerRangeCollision->OnComponentBeginOverlap.AddDynamic(this, &AZedPawn::OnCharacterInRange);
	PlayerRangeCollision->OnComponentEndOverlap.AddDynamic(this, &AZedPawn::OnCharacterOutRange);

	// Initialize speed values
	GetCharacterMovement()->MaxWalkSpeed = ZED_SPEED_PATROL;
	CurrentWalkSpeed = ZED_SPEED_PATROL;
	TargetWalkSpeed = ZED_SPEED_PATROL;
}


/**
 *	Sets replicated variables.
 *
 * @param OutLifetimeProps
 * @return void
 */
void AZedPawn::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AZedPawn, Health);
	DOREPLIFETIME(AZedPawn, IsDead);
	DOREPLIFETIME(AZedPawn, TargetWalkSpeed);
	DOREPLIFETIME(AZedPawn, BaseSpeedMultiplier);
	// Optimization: PatrolTag is only used server-side for AI behavior, no need to replicate
	// DOREPLIFETIME(AZedPawn, PatrolTag);
}


/**
 *	Begin Play -> Sets Patrol Tag in Blackboard and Updates Alive Zombies in GameState.
 *
 * @param
 * @return void
 */
void AZedPawn::BeginPlay()
{
	Super::BeginPlay();

	// Set random speed multiplier on server (will replicate to clients)
	if (HasAuthority())
	{
		BaseSpeedMultiplier = FMath::FRandRange(ZED_SPEED_MULTIPLIER_MIN, ZED_SPEED_MULTIPLIER_MAX);

		// Set random attack cooldown
		AttackCooldownDuration = FMath::FRandRange(ZED_ATTACK_COOLDOWN_MIN, ZED_ATTACK_COOLDOWN_MAX);
	}

	FTimerHandle DelayedBeginPlayHandle;
	FTimerDelegate DelayedBeginPlayDelegate;

	DelayedBeginPlayDelegate.BindLambda([this] {
		AHordeGameState* GS = Cast<AHordeGameState>(GetWorld()->GetGameState());
		if (GS)
		{
			GS->UpdateAliveZeds();
			AZedAIController* AIC = Cast<AZedAIController>(GetController());
			if (AIC && AIC->GetBlackboardComponent())
			{
				AIC->GetBlackboardComponent()->SetValueAsName("PatrolTag", PatrolTag);
			}
		}
	});

	GetWorld()->GetTimerManager().SetTimer(DelayedBeginPlayHandle, DelayedBeginPlayDelegate, 1.f, false);
}


/**
 *	Tick - Updates smooth speed interpolation.
 *
 * @param DeltaTime - Frame delta time
 * @return void
 */
void AZedPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Update speed interpolation
	UpdateWalkSpeedInterpolation(DeltaTime);
}



/**
 *	Gives specified player points.
 *
 * @param The Player Character to give points, the amount of points and the point type.
 * @return void
 */
void AZedPawn::GivePlayerPoints(ACharacter* Player, int32 Points, EPointType PointType)
{
	AHordeBaseCharacter* Char = Cast<AHordeBaseCharacter>(Player);
	if (Char)
	{
		AHordePlayerState* PS = Cast<AHordePlayerState>(Char->GetPlayerState());
		if (PS)
		{
			PS->AddPoints(Points, PointType);
		}
	}
}



/**
 *	Received Damage and executes the following functions depending on the AI's health. If AI gets an head shot it dies instantly.
 *
 * @param The Damage, Damage Event, Event Instigator and the Damage Causer.
 * @return
 */
float AZedPawn::TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	Super::TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);

	if (DamageEvent.IsOfType(FRadialDamageEvent::ClassID))
	{
		if (!IsDead)
		{
			ACharacter* PLY = Cast<ACharacter>(DamageCauser);
			if (PLY)
			{
				KillAI(PLY, EPointType::EPointCasual);
			}
		}
	}
	else if(DamageEvent.IsOfType(FPointDamageEvent::ClassID))
	{
		if (!IsDead)
		{
			FHitResult HitRes;
			FVector HitDirection;
			DamageEvent.GetBestHitInfo(this, DamageCauser, HitRes, HitDirection);
			if (HitRes.BoneName != NAME_None)
			{
				if (HitRes.BoneName == "head")
				{
					PlayHeadShotFX();
					DeathFX(HitDirection);
					ACharacter* PLY = Cast<ACharacter>(DamageCauser);
					if (PLY)
					{
						KillAI(PLY, EPointType::EPointHeadShot);
					}
				}
				else
				{
					Health = FMath::Clamp<float>((Health - Damage), 0.f, 100.f);
					if (Health <= 0)
					{
						DeathFX(HitDirection);
						ACharacter* PLY = Cast<ACharacter>(DamageCauser);
						if (PLY)
						{
							KillAI(PLY, EPointType::EPointCasual);
						}
					}
					else
					{
						// Process hit reaction for non-lethal damage
						ProcessHitReaction();
					}
				}
			}
			else {
				UE_LOG(LogTemp, Error, TEXT("ZedPawn: Point Damage BoneName == NAME_None! There might be a collision in the way."));
			}

		}
	}

	return Health;
}


/**
 *	Sets AI as Dead. Gives killer points depending on kill type and updates alive zombies.
 *
 * @param ACharacter as killer and the Point Type
 * @return void
 */
void AZedPawn::KillAI(ACharacter* Killer, EPointType KillType)
{
	IsDead = true;
	Health = 0.f;

	switch (KillType) {
	case EPointType::EPointCasual:
		GivePlayerPoints(Killer, 100, KillType);
		break;

	case EPointType::EPointHeadShot:
		GivePlayerPoints(Killer, 250, KillType);
		break;

	default:
		break;
	}

	AZedAIController* AIC = Cast<AZedAIController>(GetController());
	if (AIC && AIC->GetBlackboardComponent())
	{
		AIC->GetBlackboardComponent()->SetValueAsBool("IsDead", true);
	}
	DeathFX(FVector(0.f, 0.f, 0.f));
	SetLifeSpan(10.f);
	

	AHordeGameState* GS = Cast<AHordeGameState>(GetWorld()->GetGameState());
	if (GS)
	{
		GS->UpdateAliveZeds();
	}

}


/** ( Bound to Delegate )
 *	Check if Player is colliding with AI. Sets the Blackboard Variable "PlayerInRange".
 *	Uses a counter to properly track multiple players entering/exiting range.
 *
 * @param UPrimitiveComponent ( Overlapping Component ), AActor ( Other Actor ), UPrimitiveComponent ( Other Component ), int32 ( Other Body Index ), bool ( bFromSweep ), FHitResult ( Sweep Result )
 * @return void
 */
void AZedPawn::OnCharacterInRange(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AHordeBaseCharacter* Char = Cast<AHordeBaseCharacter>(OtherActor);
	if (Char && !Char->GetIsDead())
	{
		PlayersInRangeCount++;
		AZedAIController* AIC = Cast<AZedAIController>(GetController());
		if (AIC && AIC->GetBlackboardComponent())
		{
			AIC->GetBlackboardComponent()->SetValueAsBool("PlayerInRange", true);
		}
	}
}

/** ( Bound to Delegate )
 *	Resets BlackboardKey "PlayerInRange" if all Horde Characters stepped out of collision.
 *	Only sets PlayerInRange to false when no players remain in range.
 *	Fixed: Only decrement counter for alive characters (matching OnCharacterInRange logic)
 *
 * @param UPrimitiveComponent ( Overlapping Component ), AActor ( Other Actor ), UPrimitiveComponent ( Other Component ), int32 ( Other Body Index )
 * @return void
 */
void AZedPawn::OnCharacterOutRange(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	AHordeBaseCharacter* Char = Cast<AHordeBaseCharacter>(OtherActor);
	// Fixed: Only decrement if the character is alive (matching the check in OnCharacterInRange)
	// Dead characters were never counted, so we shouldn't decrement for them
	if (Char && !Char->GetIsDead())
	{
		PlayersInRangeCount = FMath::Max(0, PlayersInRangeCount - 1);
		if (PlayersInRangeCount == 0)
		{
			AZedAIController* AIC = Cast<AZedAIController>(GetController());
			if (AIC && AIC->GetBlackboardComponent())
			{
				AIC->GetBlackboardComponent()->SetValueAsBool("PlayerInRange", false);
			}
		}
	}
}


/** ( Multicast )
 *	Simulates Physics - Stops Sounds and plays death sound.
 *
 * @param FVector ( Direction "Impact Direction where it got shot from.")
 * @return
 */
void AZedPawn::DeathFX_Implementation(FVector Direction)
{
	GetMesh()->SetSimulatePhysics(true);
	GetCapsuleComponent()->SetCollisionProfileName(FName(TEXT("DeadAI")));
	ZedIdleSound->Stop();

	USoundCue* DeathSound = ObjectFromPath<USoundCue>(TEXT("SoundCue'/Game/HordeTemplateBP/Assets/Sounds/A_ZedDeath.A_ZedDeath'"));
	if (DeathSound)
	{
		UGameplayStatics::SpawnSoundAtLocation(GetWorld(), DeathSound, GetMesh()->GetComponentLocation());
	}

	if (GetMesh()->IsSimulatingPhysics(NAME_None))
	{
		GetMesh()->AddForce(Direction * 500.f, NAME_None, true);
	}
}

bool AZedPawn::DeathFX_Validate(FVector Direction)
{
	return true;
}


/**	( Multicast )
 *	Plays Attack Animation on all Clients. Also plays randomly AttackSound.
 *
 * @param
 * @return void
 */
void AZedPawn::PlayAttackFX_Implementation()
{
	UAnimMontage* AttackAnimation = ObjectFromPath<UAnimMontage>(TEXT("AnimMontage'/Game/HordeTemplateBP/Assets/Animations/Zombie/A_UEZomb_Attack_Montage.A_UEZomb_Attack_Montage'"));
	if (GetMesh()->GetAnimInstance() && AttackAnimation)
	{
		GetMesh()->GetAnimInstance()->Montage_Play(AttackAnimation);
	}

	if (FMath::RandBool())
	{
		USoundCue* AttackSound = ObjectFromPath<USoundCue>(TEXT("SoundCue'/Game/HordeTemplateBP/Assets/Sounds/A_ZedAttack.A_ZedAttack'"));
		if (AttackSound)
		{
			UGameplayStatics::SpawnSoundAtLocation(GetWorld(), AttackSound, AttackPoint->GetComponentLocation());
		}
	}
}

bool AZedPawn::PlayAttackFX_Validate()
{
	return true;
}


/**	( Multicast )
 *	Hides head bone and spawns head shot particle on the head bone. Also spawns head shot sound.
 *
 * @param
 * @return void
 */
void AZedPawn::PlayHeadShotFX_Implementation()
{
	//Hide Headbone
	GetMesh()->HideBone(GetMesh()->GetBoneIndex("head"), EPhysBodyOp::PBO_None);

	//Spawn Blood Splatter
	UParticleSystem* BloodSplat = ObjectFromPath<UParticleSystem>(TEXT("ParticleSystem'/Game/HordeTemplateBP/Assets/Effects/ParticleSystems/Gameplay/Player/P_bloodSplatter.P_bloodSplatter'"));
	if (BloodSplat)
	{
		/*
		We need to use SpawnEmitterAtLocation instead of Attached. https://issues.unrealengine.com/issue/UE-29018
		*/
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), BloodSplat, GetMesh()->GetBoneLocation("head", EBoneSpaces::WorldSpace), FRotator(0.f, 0.f, 0.f), true, EPSCPoolMethod::None);
	}
	
	//Spawn Headshot Sound at head.
	USoundCue* GoreShotSound = ObjectFromPath<USoundCue>(TEXT("SoundCue'/Game/HordeTemplateBP/Assets/Sounds/A_Horde_Goreshot_Cue.A_Horde_Goreshot_Cue'"));
	if (GoreShotSound)
	{
		UGameplayStatics::SpawnSoundAtLocation(GetWorld(), GoreShotSound, GetMesh()->GetSocketLocation("head"));
	}

}

bool AZedPawn::PlayHeadShotFX_Validate()
{
	return true;
}


/**	( Multicast )
 *	Sets the Walkspeed on all clients.
 *
 * @param New Max Walkspeed
 * @return void
 */
void AZedPawn::ModifyWalkSpeed_Implementation(float MaxWalkSpeed)
{
	GetCharacterMovement()->MaxWalkSpeed = MaxWalkSpeed;
}

bool AZedPawn::ModifyWalkSpeed_Validate(float MaxWalkSpeed)
{
	return true;
}


/**
 *	Sets the target walk speed for smooth interpolation.
 *
 * @param NewTargetSpeed - Target speed to interpolate towards
 * @return void
 */
void AZedPawn::SetTargetWalkSpeed(float NewTargetSpeed)
{
	// Apply base speed multiplier
	TargetWalkSpeed = NewTargetSpeed * BaseSpeedMultiplier;
}


/**
 *	Updates walk speed interpolation each frame.
 *
 * @param DeltaTime - Frame delta time
 * @return void
 */
void AZedPawn::UpdateWalkSpeedInterpolation(float DeltaTime)
{
	if (FMath::IsNearlyEqual(CurrentWalkSpeed, TargetWalkSpeed, 1.f))
	{
		CurrentWalkSpeed = TargetWalkSpeed;
		return;
	}

	// Determine acceleration/deceleration rate
	float InterpSpeed;
	if (CurrentWalkSpeed < TargetWalkSpeed)
	{
		// Accelerating
		InterpSpeed = ZED_SPEED_ACCELERATION;
	}
	else
	{
		// Decelerating (faster stop)
		InterpSpeed = ZED_SPEED_DECELERATION;
	}

	// Interpolate towards target
	CurrentWalkSpeed = FMath::FInterpConstantTo(CurrentWalkSpeed, TargetWalkSpeed, DeltaTime, InterpSpeed);

	// Apply to character movement
	GetCharacterMovement()->MaxWalkSpeed = CurrentWalkSpeed;
}


/**
 *	Checks if the zombie can attack (cooldown elapsed).
 *
 * @return True if attack is available
 */
bool AZedPawn::CanAttack() const
{
	float CurrentTime = GetWorld()->GetTimeSeconds();
	return (CurrentTime - LastAttackTime) >= AttackCooldownDuration;
}


/**
 *	Resets the attack cooldown timer.
 *
 * @return void
 */
void AZedPawn::ResetAttackCooldown()
{
	LastAttackTime = GetWorld()->GetTimeSeconds();

	// Randomize next cooldown duration for variety
	AttackCooldownDuration = FMath::FRandRange(ZED_ATTACK_COOLDOWN_MIN, ZED_ATTACK_COOLDOWN_MAX);
}


/**
 *	Gets the remaining attack cooldown time.
 *
 * @return Remaining cooldown in seconds
 */
float AZedPawn::GetAttackCooldownRemaining() const
{
	float CurrentTime = GetWorld()->GetTimeSeconds();
	float Elapsed = CurrentTime - LastAttackTime;
	return FMath::Max(0.f, AttackCooldownDuration - Elapsed);
}


/**
 *	Processes hit reaction (stagger or anger).
 *
 * @return void
 */
void AZedPawn::ProcessHitReaction()
{
	if (!HasAuthority())
	{
		return;
	}

	AZedAIController* AIC = Cast<AZedAIController>(GetController());
	if (!AIC)
	{
		return;
	}

	// Roll for stagger vs anger
	if (FMath::FRand() < ZED_STAGGER_CHANCE)
	{
		// 30% chance: Stagger
		AIC->ApplyStagger();
	}
	else
	{
		// 70% chance: Anger burst
		AIC->ApplyAngerBurst();
	}
}


/**	( Multicast )
 *	Plays alert sound when first spotting the player.
 *
 * @return void
 */
void AZedPawn::PlayAlertSound_Implementation()
{
	USoundCue* AlertSound = ObjectFromPath<USoundCue>(TEXT("SoundCue'/Game/HordeTemplateBP/Assets/Sounds/A_ZedAttack.A_ZedAttack'"));
	if (AlertSound)
	{
		UGameplayStatics::SpawnSoundAtLocation(GetWorld(), AlertSound, GetActorLocation());
	}
}

bool AZedPawn::PlayAlertSound_Validate()
{
	return true;
}


/**	( Multicast )
 *	Starts playing chase breathing loop.
 *
 * @return void
 */
void AZedPawn::StartChaseBreathing_Implementation()
{
	if (ZedChaseBreathing && !ZedChaseBreathing->IsPlaying())
	{
		// Use the idle sound as placeholder - in production you'd use a dedicated breathing sound
		USoundCue* BreathingSound = ObjectFromPath<USoundCue>(TEXT("SoundCue'/Game/HordeTemplateBP/Assets/Sounds/A_Zed_RND_Idle.A_Zed_RND_Idle'"));
		if (BreathingSound)
		{
			ZedChaseBreathing->SetSound(BreathingSound);
			ZedChaseBreathing->Play();
		}
	}
}

bool AZedPawn::StartChaseBreathing_Validate()
{
	return true;
}


/**	( Multicast )
 *	Stops chase breathing loop.
 *
 * @return void
 */
void AZedPawn::StopChaseBreathing_Implementation()
{
	if (ZedChaseBreathing && ZedChaseBreathing->IsPlaying())
	{
		ZedChaseBreathing->Stop();
	}
}

bool AZedPawn::StopChaseBreathing_Validate()
{
	return true;
}


/**
 *	Gets the current AI state from the controller.
 *
 * @return Current AI state enum value
 */
EZedAIState AZedPawn::GetCurrentAIState() const
{
	AZedAIController* AIC = Cast<AZedAIController>(GetController());
	if (AIC)
	{
		return AIC->GetAIState();
	}
	return EZedAIState::Patrol;
}

