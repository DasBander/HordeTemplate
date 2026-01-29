// Copyright (c) 2018 - 2026 Marc Fraedrich
// Horde Template V2 (Native) - A cooperative zombie survival game framework
// Licensed under the MIT License

#include "ZedSpawner.h"
#include "ZedPawn.h"
#include "HordeTemplateV2Native.h"
#include "Character/HordeBaseCharacter.h"
#include "Gameplay/HordeGameState.h"
#include "NavigationSystem.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Runtime/Engine/Public/EngineUtils.h"
#include "ConstructorHelpers.h"
#include "Net/UnrealNetwork.h"

/**
 * @file ZedSpawner.cpp
 * @brief Implementation of the dynamic zombie spawner with AAA-style spawning behavior
 * @author Marc Fraedrich
 */

AZedSpawner::AZedSpawner()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
	SetReplicates(true);

	// Create spawn volume
	SpawnVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("SpawnVolume"));
	SpawnVolume->SetBoxExtent(FVector(200.f, 200.f, 100.f));
	SpawnVolume->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SpawnVolume->SetVisibility(true);
	SpawnVolume->bHiddenInGame = true;
	RootComponent = SpawnVolume;

	// Create editor icon
	EditorIcon = CreateDefaultSubobject<UBillboardComponent>(TEXT("EditorIcon"));
	EditorIcon->SetupAttachment(RootComponent);
	EditorIcon->SetRelativeLocation(FVector(0.f, 0.f, 100.f));
	EditorIcon->bHiddenInGame = true;

	const ConstructorHelpers::FObjectFinder<UTexture2D> IconAsset(TEXT("Texture2D'/Engine/EditorResources/S_NavP.S_NavP'"));
	if (IconAsset.Succeeded())
	{
		EditorIcon->SetSprite(IconAsset.Object);
	}

	// Set default zombie class
	ZombieClass = AZedPawn::StaticClass();
}


void AZedSpawner::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AZedSpawner, CurrentState);
	DOREPLIFETIME(AZedSpawner, TotalKills);
}


void AZedSpawner::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		// Start inactive, will activate when player enters range
		SetState(EZedSpawnerState::Inactive);
	}
}


void AZedSpawner::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Only run spawning logic on server
	if (!HasAuthority())
	{
		return;
	}

	// Clean up dead zombie references periodically
	CleanupDeadZombies();

	// Update state based on current conditions
	UpdateState();

	// Handle spawning based on mode
	if (CurrentState == EZedSpawnerState::Active)
	{
		TimeSinceLastSpawn += DeltaTime;

		float EffectiveInterval = (SpawnInterval + FMath::FRandRange(-SpawnIntervalDeviation, SpawnIntervalDeviation)) / GetDifficultyMultiplier();

		if (TimeSinceLastSpawn >= EffectiveInterval)
		{
			AttemptSpawn();
			TimeSinceLastSpawn = 0.f;
		}
	}

	// Debug visualization
	if (bShowDebug)
	{
		// Draw spawn volume
		FColor VolumeColor = FColor::Green;
		if (CurrentState == EZedSpawnerState::Exhausted) VolumeColor = FColor::Yellow;
		else if (CurrentState == EZedSpawnerState::Inactive) VolumeColor = FColor::Blue;
		else if (CurrentState == EZedSpawnerState::Disabled) VolumeColor = FColor::Red;

		DrawDebugBox(GetWorld(), GetActorLocation(), SpawnVolume->GetScaledBoxExtent(), VolumeColor, false, -1.f, 0, 2.f);

		// Draw activation radius
		DrawDebugSphere(GetWorld(), GetActorLocation(), MinActivationDistance, 16, FColor::Cyan, false, -1.f, 0, 1.f);
		DrawDebugSphere(GetWorld(), GetActorLocation(), MaxActivationDistance, 16, FColor::Blue, false, -1.f, 0, 1.f);

		// Draw state info
		FString StateStr = FString::Printf(TEXT("State: %d | Kills: %d/%d | Alive: %d/%d"),
			(int32)CurrentState, TotalKills, MaxKillsBeforeExhaustion, CurrentAliveZombies, MaxConcurrentZombies);
		DrawDebugString(GetWorld(), GetActorLocation() + FVector(0, 0, 150), StateStr, nullptr, FColor::White, 0.f, true);
	}
}


void AZedSpawner::UpdateState()
{
	// Don't update if disabled
	if (CurrentState == EZedSpawnerState::Disabled)
	{
		return;
	}

	bool bPlayerInRange = IsPlayerInRange();

	// Handle state transitions
	switch (CurrentState)
	{
	case EZedSpawnerState::Inactive:
		if (bPlayerInRange)
		{
			SetState(EZedSpawnerState::Active);
		}
		break;

	case EZedSpawnerState::Active:
		// Check for exhaustion
		if (bCanExhaust && TotalKills >= MaxKillsBeforeExhaustion)
		{
			SetState(EZedSpawnerState::Exhausted);
			OnSpawnerExhausted();

			if (bCanReactivate)
			{
				GetWorld()->GetTimerManager().SetTimer(
					ReactivationTimerHandle,
					this,
					&AZedSpawner::OnReactivationTimer,
					ReactivationDelay,
					false
				);
			}
		}
		// Check if player left range
		else if (bDeactivateWhenFar && !bPlayerInRange)
		{
			SetState(EZedSpawnerState::Inactive);
		}
		break;

	case EZedSpawnerState::Exhausted:
		// Track if player leaves range (for re-entry requirement)
		if (!bPlayerInRange)
		{
			bPlayerWasInRange = false;
		}
		break;

	default:
		break;
	}

	bPlayerWasInRange = bPlayerInRange;
}


void AZedSpawner::SetState(EZedSpawnerState NewState)
{
	if (CurrentState == NewState)
	{
		return;
	}

	EZedSpawnerState OldState = CurrentState;
	CurrentState = NewState;

	OnStateChanged(OldState, NewState);
}


void AZedSpawner::AttemptSpawn()
{
	if (!CanSpawn())
	{
		return;
	}

	int32 SpawnCount = 1;
	if (SpawnMode == EZedSpawnMode::Wave)
	{
		SpawnCount = FMath::Min(WaveSize - CurrentWaveSpawned, MaxConcurrentZombies - CurrentAliveZombies);
	}

	for (int32 i = 0; i < SpawnCount; i++)
	{
		FVector SpawnLocation;
		if (GetValidSpawnLocation(SpawnLocation))
		{
			AZedPawn* NewZombie = SpawnZombieAtLocation(SpawnLocation);
			if (NewZombie)
			{
				OnZombieSpawned(NewZombie);

				if (SpawnMode == EZedSpawnMode::Wave)
				{
					CurrentWaveSpawned++;
					if (CurrentWaveSpawned >= WaveSize)
					{
						CurrentWaveSpawned = 0;
						TimeSinceLastSpawn = -WaveDelay; // Add wave delay
					}
				}
			}
		}
	}
}


bool AZedSpawner::CanSpawn() const
{
	// Check basic conditions
	if (!ZombieClass)
	{
		return false;
	}

	// Check concurrent limit
	if (CurrentAliveZombies >= MaxConcurrentZombies)
	{
		return false;
	}

	// Check global zombie limit
	if (bScaleWithGlobalZombieCount)
	{
		AHordeGameState* GS = Cast<AHordeGameState>(GetWorld()->GetGameState());
		if (GS && GS->CountAliveZeds() >= GlobalZombieLimit)
		{
			return false;
		}
	}

	// Check player vision
	if (bCheckPlayerVision && IsPlayerLookingAtSpawnArea())
	{
		return false;
	}

	// Triggered mode requires explicit trigger
	if (SpawnMode == EZedSpawnMode::Triggered)
	{
		return false;
	}

	return true;
}


bool AZedSpawner::IsPlayerInRange() const
{
	float ClosestDistance = GetClosestPlayerDistance();

	if (ClosestDistance < 0.f)
	{
		return false; // No players found
	}

	return ClosestDistance >= MinActivationDistance && ClosestDistance <= MaxActivationDistance;
}


bool AZedSpawner::IsPlayerLookingAtSpawnArea() const
{
	TArray<ACharacter*> Players = GetAlivePlayers();
	FVector SpawnCenter = GetActorLocation();
	FVector SpawnExtent = SpawnVolume->GetScaledBoxExtent();

	for (ACharacter* Player : Players)
	{
		if (!Player)
		{
			continue;
		}

		APlayerController* PC = Cast<APlayerController>(Player->GetController());
		if (!PC)
		{
			continue;
		}

		// Get player view direction
		FVector ViewLocation;
		FRotator ViewRotation;
		PC->GetPlayerViewPoint(ViewLocation, ViewRotation);
		FVector ViewDirection = ViewRotation.Vector();

		// Check if spawn center is within player's field of view
		FVector ToSpawner = (SpawnCenter - ViewLocation).GetSafeNormal();
		float DotProduct = FVector::DotProduct(ViewDirection, ToSpawner);
		float AngleRadians = FMath::Acos(DotProduct);
		float AngleDegrees = FMath::RadiansToDegrees(AngleRadians);

		if (AngleDegrees <= PlayerVisionAngle)
		{
			// Player is looking in this direction - check line of sight
			FHitResult HitResult;
			FCollisionQueryParams QueryParams;
			QueryParams.AddIgnoredActor(Player);

			// Trace to spawn area center
			bool bHit = GetWorld()->LineTraceSingleByChannel(
				HitResult,
				ViewLocation,
				SpawnCenter,
				ECC_Visibility,
				QueryParams
			);

			// If nothing was hit or hit is close to spawn area, player can see it
			if (!bHit)
			{
				return true;
			}

			float DistanceToHit = FVector::Dist(ViewLocation, HitResult.Location);
			float DistanceToSpawn = FVector::Dist(ViewLocation, SpawnCenter);

			// If the hit is close to the spawn area (within buffer), consider it visible
			if (DistanceToHit >= DistanceToSpawn - VisionBufferDistance)
			{
				return true;
			}
		}
	}

	return false;
}


bool AZedSpawner::GetValidSpawnLocation(FVector& OutLocation) const
{
	FVector SpawnCenter = GetActorLocation();
	FVector SpawnExtent = SpawnVolume->GetScaledBoxExtent();

	// Try multiple times to find a valid location
	for (int32 Attempt = 0; Attempt < 10; Attempt++)
	{
		// Random point within spawn volume
		FVector RandomOffset(
			FMath::FRandRange(-SpawnExtent.X, SpawnExtent.X),
			FMath::FRandRange(-SpawnExtent.Y, SpawnExtent.Y),
			0.f // Keep Z at ground level
		);

		FVector TestLocation = SpawnCenter + RandomOffset;

		// Project to navigation mesh
		UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
		if (NavSys)
		{
			FNavLocation NavLocation;
			if (NavSys->ProjectPointToNavigation(TestLocation, NavLocation, FVector(100.f, 100.f, 200.f)))
			{
				TestLocation = NavLocation.Location;
			}
		}

		// Check for collision at spawn location
		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(this);

		// Capsule check for zombie size
		bool bBlocked = GetWorld()->OverlapBlockingTestByChannel(
			TestLocation + FVector(0.f, 0.f, 88.f), // Capsule center
			FQuat::Identity,
			ECC_Pawn,
			FCollisionShape::MakeCapsule(34.f, 88.f),
			QueryParams
		);

		if (!bBlocked)
		{
			// Check if any player is looking at this specific location
			if (bCheckPlayerVision)
			{
				bool bVisible = false;
				TArray<ACharacter*> Players = GetAlivePlayers();

				for (ACharacter* Player : Players)
				{
					if (!Player) continue;

					APlayerController* PC = Cast<APlayerController>(Player->GetController());
					if (!PC) continue;

					FVector ViewLocation;
					FRotator ViewRotation;
					PC->GetPlayerViewPoint(ViewLocation, ViewRotation);
					FVector ViewDirection = ViewRotation.Vector();

					FVector ToLocation = (TestLocation - ViewLocation).GetSafeNormal();
					float DotProduct = FVector::DotProduct(ViewDirection, ToLocation);
					float AngleDegrees = FMath::RadiansToDegrees(FMath::Acos(DotProduct));

					if (AngleDegrees <= PlayerVisionAngle)
					{
						// Check line of sight to this specific point
						FHitResult HitResult;
						FCollisionQueryParams LOSParams;
						LOSParams.AddIgnoredActor(Player);

						bool bLOSBlocked = GetWorld()->LineTraceSingleByChannel(
							HitResult,
							ViewLocation,
							TestLocation + FVector(0.f, 0.f, 50.f),
							ECC_Visibility,
							LOSParams
						);

						if (!bLOSBlocked)
						{
							bVisible = true;
							break;
						}
					}
				}

				if (bVisible)
				{
					continue; // Try another location
				}
			}

			OutLocation = TestLocation;
			return true;
		}
	}

	return false;
}


AZedPawn* AZedSpawner::SpawnZombieAtLocation(const FVector& Location)
{
	if (!ZombieClass)
	{
		return nullptr;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	AZedPawn* NewZombie = GetWorld()->SpawnActorDeferred<AZedPawn>(
		ZombieClass,
		FTransform(FRotator::ZeroRotator, Location),
		nullptr,
		nullptr,
		ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn
	);

	if (NewZombie)
	{
		NewZombie->PatrolTag = PatrolTag;
		NewZombie->FinishSpawning(FTransform(FRotator::ZeroRotator, Location));

		// Track this zombie
		SpawnedZombies.Add(NewZombie);
		CurrentAliveZombies++;

		return NewZombie;
	}

	return nullptr;
}


void AZedSpawner::OnZombieDied(AZedPawn* DeadZombie)
{
	if (!DeadZombie)
	{
		return;
	}

	TotalKills++;
	CurrentAliveZombies = FMath::Max(0, CurrentAliveZombies - 1);

	// Remove from tracked array
	SpawnedZombies.RemoveSingle(DeadZombie);
}


void AZedSpawner::CleanupDeadZombies()
{
	int32 AliveCount = 0;

	for (int32 i = SpawnedZombies.Num() - 1; i >= 0; i--)
	{
		if (!SpawnedZombies[i].IsValid())
		{
			SpawnedZombies.RemoveAt(i);
			TotalKills++; // Count as kill if reference is lost
		}
		else if (SpawnedZombies[i]->GetIsDead())
		{
			TotalKills++;
			SpawnedZombies.RemoveAt(i);
		}
		else
		{
			AliveCount++;
		}
	}

	CurrentAliveZombies = AliveCount;
}


float AZedSpawner::GetDifficultyMultiplier() const
{
	float Multiplier = 1.f;

	// Scale with global zombie count (slower as more zombies exist)
	if (bScaleWithGlobalZombieCount)
	{
		AHordeGameState* GS = Cast<AHordeGameState>(GetWorld()->GetGameState());
		if (GS)
		{
			int32 CurrentZombies = GS->CountAliveZeds();
			float ZombieRatio = (float)CurrentZombies / (float)GlobalZombieLimit;
			Multiplier *= FMath::Lerp(1.f, 0.25f, FMath::Clamp(ZombieRatio, 0.f, 1.f));
		}
	}

	// Scale with player count
	if (bScaleWithPlayerCount)
	{
		TArray<ACharacter*> Players = GetAlivePlayers();
		int32 PlayerCount = Players.Num();
		if (PlayerCount > 1)
		{
			Multiplier *= FMath::Pow(PlayerCountMultiplier, PlayerCount - 1);
		}
	}

	return FMath::Max(0.1f, Multiplier);
}


void AZedSpawner::OnReactivationTimer()
{
	if (CurrentState != EZedSpawnerState::Exhausted)
	{
		return;
	}

	// Check re-entry requirement
	if (bRequireReEntryForReactivation)
	{
		// If player is currently in range, they must have left and come back
		bool bPlayerInRange = IsPlayerInRange();
		if (bPlayerInRange && bPlayerWasInRange)
		{
			// Player never left - reschedule
			GetWorld()->GetTimerManager().SetTimer(
				ReactivationTimerHandle,
				this,
				&AZedSpawner::OnReactivationTimer,
				10.f, // Check again in 10 seconds
				false
			);
			return;
		}
	}

	ForceReactivate();
}


float AZedSpawner::GetClosestPlayerDistance() const
{
	TArray<ACharacter*> Players = GetAlivePlayers();
	FVector SpawnerLocation = GetActorLocation();

	float ClosestDistance = -1.f;

	for (ACharacter* Player : Players)
	{
		if (Player)
		{
			float Distance = FVector::Dist(SpawnerLocation, Player->GetActorLocation());
			if (ClosestDistance < 0.f || Distance < ClosestDistance)
			{
				ClosestDistance = Distance;
			}
		}
	}

	return ClosestDistance;
}


TArray<ACharacter*> AZedSpawner::GetAlivePlayers() const
{
	TArray<ACharacter*> Players;

	for (TActorIterator<AHordeBaseCharacter> ActorItr(GetWorld()); ActorItr; ++ActorItr)
	{
		AHordeBaseCharacter* Player = *ActorItr;
		if (Player && !Player->GetIsDead())
		{
			Players.Add(Player);
		}
	}

	return Players;
}


void AZedSpawner::TriggerSpawn(int32 Count)
{
	if (CurrentState == EZedSpawnerState::Disabled)
	{
		return;
	}

	for (int32 i = 0; i < Count; i++)
	{
		if (CurrentAliveZombies >= MaxConcurrentZombies)
		{
			break;
		}

		// Skip vision check for triggered spawns
		bool bOldVisionCheck = bCheckPlayerVision;
		bCheckPlayerVision = false;

		FVector SpawnLocation;
		if (GetValidSpawnLocation(SpawnLocation))
		{
			AZedPawn* NewZombie = SpawnZombieAtLocation(SpawnLocation);
			if (NewZombie)
			{
				OnZombieSpawned(NewZombie);
			}
		}

		bCheckPlayerVision = bOldVisionCheck;
	}
}


void AZedSpawner::ForceExhaust()
{
	if (CurrentState == EZedSpawnerState::Disabled)
	{
		return;
	}

	TotalKills = MaxKillsBeforeExhaustion;
	SetState(EZedSpawnerState::Exhausted);
	OnSpawnerExhausted();
}


void AZedSpawner::ForceReactivate()
{
	if (CurrentState == EZedSpawnerState::Disabled)
	{
		return;
	}

	TotalKills = 0;
	CurrentWaveSpawned = 0;
	TimeSinceLastSpawn = 0.f;

	SetState(EZedSpawnerState::Active);
	OnSpawnerReactivated();
}


void AZedSpawner::DisableSpawner()
{
	GetWorld()->GetTimerManager().ClearTimer(SpawnTimerHandle);
	GetWorld()->GetTimerManager().ClearTimer(ReactivationTimerHandle);
	SetState(EZedSpawnerState::Disabled);
}


void AZedSpawner::EnableSpawner()
{
	if (CurrentState == EZedSpawnerState::Disabled)
	{
		TotalKills = 0;
		SetState(EZedSpawnerState::Inactive);
	}
}


#if WITH_EDITOR
void AZedSpawner::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	// Ensure min distance is less than max
	if (MinActivationDistance > MaxActivationDistance)
	{
		MinActivationDistance = MaxActivationDistance;
	}
}
#endif
