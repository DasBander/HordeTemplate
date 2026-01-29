// Copyright (c) 2018 - 2026 Marc Fraedrich
// Horde Template V2 (Native) - A cooperative zombie survival game framework
// Licensed under the MIT License

#include "BaseSpectator.h"

/**
 * @file BaseSpectator.cpp
 * @brief Implementation of spectator pawn with random player focus functionality
 * @author Marc Fraedrich
 */
#include "EngineUtils.h"

/**
 * Constructor for ABaseSpectator
 *
 * @param
 * @return
 */
ABaseSpectator::ABaseSpectator()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
}


/** ( Virtual; Overridden )
 *	Sets up Key Bindings for Player.
 *
 * @param Player Input Component
 * @return void
 */
void ABaseSpectator::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &ABaseSpectator::ServerFocusPlayer);

}

/**
 *	Tick - Smoothly interpolate camera when spectating another player.
 *
 * @param DeltaTime
 * @return void
 */
void ABaseSpectator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Only smooth camera on the local client when spectating someone
	if (CurrentSpectateTarget && IsLocallyControlled())
	{
		APlayerController* PC = Cast<APlayerController>(GetController());
		if (PC && PC->PlayerCameraManager)
		{
			// Get target camera location from the spectated player's camera
			UCameraComponent* TargetCamera = CurrentSpectateTarget->GetCamera();
			if (TargetCamera)
			{
				FVector TargetLocation = TargetCamera->GetComponentLocation();
				FRotator TargetRotation = TargetCamera->GetComponentRotation();

				// Initialize smoothed values on first tick
				if (!bSmoothedValuesInitialized)
				{
					SmoothedCameraLocation = TargetLocation;
					SmoothedCameraRotation = TargetRotation;
					bSmoothedValuesInitialized = true;
				}

				// Smoothly interpolate towards target
				SmoothedCameraLocation = FMath::VInterpTo(SmoothedCameraLocation, TargetLocation, DeltaTime, CameraInterpSpeed);
				SmoothedCameraRotation = FMath::RInterpTo(SmoothedCameraRotation, TargetRotation, DeltaTime, CameraInterpSpeed);

				// Apply smoothed transform to the spectator's location (camera will follow)
				SetActorLocation(SmoothedCameraLocation);
				PC->SetControlRotation(SmoothedCameraRotation);
			}
		}
	}
}

/** ( Client )
 *	Focuses Player by Player Object.
 *
 * @param Player to Focus
 * @return void
 */
void ABaseSpectator::ClientFocusPlayer_Implementation(AHordeBaseCharacter* Player)
{
	APlayerController* PC = Cast<APlayerController>(GetController());
	// Fixed: Added null check for Player parameter
	if (PC && Player)
	{
		// Store the current spectate target for smooth following
		CurrentSpectateTarget = Player;
		bSmoothedValuesInitialized = false; // Reset to snap to new target initially

		// Set view target to ourselves - we'll manually track the target smoothly in Tick
		PC->SetViewTarget(this);
	}
}


/**
 *	Gets random alive player from the World.
 *
 * @param
 * @return Random Alive Player
 */
AHordeBaseCharacter* ABaseSpectator::GetRandomAlivePlayer()
{
	TArray<AHordeBaseCharacter*> AliveCharacter;

	// Fixed: Use TActorIterator instead of TObjectIterator to only iterate actors in current world
	// TObjectIterator iterates ALL objects in memory which could include objects from other worlds/PIE instances
	for (TActorIterator<AHordeBaseCharacter> Itr(GetWorld()); Itr; ++Itr)
	{
		AHordeBaseCharacter* PLY = *Itr;
		if (PLY && !PLY->GetIsDead())
		{
			AliveCharacter.Add(PLY);
		}
	}
	return (AliveCharacter.Num() > 0) ? AliveCharacter[FMath::RandRange(0, AliveCharacter.Num() - 1)] : nullptr;
}


/**	( Server )
 *	Gets Random Alive Player and focuses him on client.
 *
 * @param
 * @return void
 */
void ABaseSpectator::ServerFocusPlayer_Implementation()
{
	AHordeBaseCharacter* PLY = GetRandomAlivePlayer();
	if (PLY)
	{
		ClientFocusPlayer(PLY);
	}
}

bool ABaseSpectator::ServerFocusPlayer_Validate()
{
	return true;
}
