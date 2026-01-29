// Copyright (c) 2018 - 2026 Marc Fraedrich
// Horde Template V2 (Native) - A cooperative zombie survival game framework
// Licensed under the MIT License

#include "HordeGameMode.h"
#include "HordeTemplateV2Native.h"
#include "Character/HordeBaseCharacter.h"
#include "Gameplay/HordeBaseController.h"
#include "Gameplay/HordeGameState.h"
#include "AI/AISpawnPoint.h"
#include "Gameplay/HordePlayerState.h"
#include "GameFramework/PlayerStart.h"
#include "HordeGameSession.h"
#include "Character/BaseSpectator.h"
#include "HUD/HordeBaseHUD.h"
#include "Gameplay/HordeWorldSettings.h"
#include "AI/ZedPawn.h"
#include "Runtime/Engine/Public/EngineUtils.h"

/**
 * @file HordeGameMode.cpp
 * @brief Implementation of main game mode with wave spawning, player management, and round logic
 * @author Marc Fraedrich
 */

/**
 * Constructor
 *
 * @param
 * @return
 */
AHordeGameMode::AHordeGameMode()
{
	GameStateClass = AHordeGameState::StaticClass();
	DefaultPawnClass = nullptr;
	PlayerControllerClass = AHordeBaseController::StaticClass();
	PlayerStateClass = AHordePlayerState::StaticClass();
	HUDClass = AHordeBaseHUD::StaticClass();
	GameSessionClass = AHordeGameSession::StaticClass();

	bUseSeamlessTravel = true;
	bStartPlayersAsSpectators = 0;
}

/**
 *	Returns Free AI Spawner Actors and the amount of Free Points.
 *
 * @param
 * @output TArray - The Spawner Actors on the Level and the amount of free Points.
 * @return void
 */
void AHordeGameMode::GetAISpawner(TArray<AActor*>& Spawner, int32& FreePoints)
{
	int32 FreeSpawnPoints = 0;
	TArray<AActor*> TempSpawner;
	for (TActorIterator<AAISpawnPoint> ActorItr(GetWorld()); ActorItr; ++ActorItr)
	{

		AAISpawnPoint* SpawnPoint = *ActorItr;
		if (SpawnPoint && !SpawnPoint->SpawnNotFree)
		{
			FreeSpawnPoints++;
			TempSpawner.Add(SpawnPoint);
		}
	}
	Spawner = TempSpawner;
	FreePoints = FreeSpawnPoints;
}

/**
 *	Checks if all Zombies are dead. If yes it should end the game or end the current game round.
 *
 * @param
 * @return void
 */
void AHordeGameMode::CheckGameOver()
{
	AHordeWorldSettings* WS = Cast<AHordeWorldSettings>(GetWorld()->GetWorldSettings());
	if (WS)
	{
		AHordeGameState* GS = Cast<AHordeGameState>(GetWorld()->GetGameState());
		if (GS && WS->MatchMode == EMatchMode::EMatchModeNonLinear)
		{
			if (GS->ZedsLeft == 0 && (GS->GameRound >= WS->MaxRounds) && ZedsLeftToSpawn == 0)
			{
				GS->EndGame(false);
			}
			else 
			{
				if (GS->CountAliveZeds() == 0 && ZedsLeftToSpawn == 0)
				{
					GS->EndGameRound();
				}
			}
		}
	}
}

/**
 *	Spawns an Spectator Pawn and lets the Player Controller possess with.
 *
 * @param Owning Player Controller of new Spectator
 * @return void
 */
void AHordeGameMode::SpawnSpectator(APlayerController* PC)
{
	FTransform SpawnTransform;
	SpawnTransform.SetLocation(GetSpectatorSpawnLocation());
	FActorSpawnParameters SpawnParam;
	ABaseSpectator* Spectator = GetWorld()->SpawnActor<ABaseSpectator>(ABaseSpectator::StaticClass(), SpawnTransform, SpawnParam);
	if (Spectator && PC)
	{
		PC->Possess(Spectator);
	}
	AHordeGameState* GS = Cast<AHordeGameState>(GetWorld()->GetGameState());
	if (GS)
	{
		GS->AllPlayerDeadCheck();
	}
}

/**
 *	Returns a Random Spawn Location depending on players that are still alive.
 *
 * @param
 * @return Random Spawn Location from Alive Players.
 */
FVector AHordeGameMode::GetSpectatorSpawnLocation()
{
	TArray<FVector> LocalLocations;
	for (TActorIterator<AHordeBaseCharacter> ActorItr(GetWorld()); ActorItr; ++ActorItr)
	{

		AHordeBaseCharacter* AliveCharacter = *ActorItr;
		if (AliveCharacter && !AliveCharacter->GetIsDead())
		{
			LocalLocations.Add(AliveCharacter->GetActorLocation() + FVector(0.f, 0.f, 50.f));
		}
	}
	if (LocalLocations.Num() > 0)
	{
		return LocalLocations[FMath::RandRange(0, LocalLocations.Num() - 1)];
	}
	else
	{
		return GetRandomPlayerSpawn().GetLocation();
	}
}

/**
 *	Returns Player Controller Object owned by the given Player ID.
 *
 * @param The Player ID we want the Player Controller of.
 * @return The Controller of the given Player ID
 */
APlayerController* AHordeGameMode::GetControllerByID(FString PlayerID)
{
	APlayerController* TempCTRL = nullptr;
	for (TActorIterator<APlayerController> ActorItr(GetWorld()); ActorItr; ++ActorItr)
	{
		APlayerController* APC = *ActorItr;
		// Fixed: Added null check for PlayerState before accessing GetUniqueId
		if (APC && APC->PlayerState && APC->PlayerState->GetUniqueId().IsValid())
		{
			if (APC->PlayerState->GetUniqueId()->ToString() == PlayerID)
			{
				TempCTRL = APC;
			}
		}
	}
	return TempCTRL;
}

/**
 *	Returns Random Location of a Spawn Point placed inside the world.
 *
 * @param
 * @return Random Player Start Location as Transform
 */
FTransform AHordeGameMode::GetRandomPlayerSpawn()
{
	TArray<FTransform> SpawnPoints;
	for (TActorIterator<APlayerStart> ActorItr(GetWorld()); ActorItr; ++ActorItr)
	{

		APlayerStart* SpawnPoint = *ActorItr;
		if (SpawnPoint)
		{
			FTransform LocalTrans;
			LocalTrans.SetLocation(SpawnPoint->GetActorLocation());
			LocalTrans.SetRotation(FRotator(0.f, SpawnPoint->GetActorRotation().Yaw, 0.f).Quaternion());
			SpawnPoints.Add(LocalTrans);
		}
	}

	// Safety check: return default transform if no spawn points found
	if (SpawnPoints.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("GetRandomPlayerSpawn: No PlayerStart actors found in world! Returning default transform."));
		return FTransform();
	}

	return SpawnPoints[FMath::RandRange(0, (SpawnPoints.Num() - 1))];
}

/** ( Virtual; Overridden )
 *	Updates Player Lobby when a Player Exits the game.
 *
 * @param AController ( Exiting Player Controller )
 * @return
 */
void AHordeGameMode::Logout(AController* Exiting)
{
	FTimerHandle DelayedRemove;

	GetWorld()->GetTimerManager().SetTimer(DelayedRemove, this, &AHordeGameMode::UpdatePlayerLobby, 1.f, false);

}

/**
 *	Runs Update Player Lobby inside Game State.
 *
 * @param
 * @return void
 */
void AHordeGameMode::UpdatePlayerLobby()
{
	AHordeGameState* GS = Cast<AHordeGameState>(GetWorld()->GetGameState());
	if (GS)
	{
		GS->UpdatePlayerLobby();
	}
}

/**
 *	Spawns all Players inside World and possesses them with given Player Controller.
 *
 * @param TArray - FPlayerInfo ( Lobby Players )
 * @return void
 */
void AHordeGameMode::GameStart(const TArray<FPlayerInfo>& LobbyPlayers)
{
	for (auto PLY : LobbyPlayers)
	{
		APlayerController* PC = GetControllerByID(PLY.PlayerID);
		if (PC)
		{
			UDataTable* DTCharacters = Cast<UDataTable>(StaticLoadObject(UDataTable::StaticClass(), nullptr, CHARACTER_DATATABLE_PATH));
			if (DTCharacters) {
				FPlayableCharacter * Char = DTCharacters->FindRow<FPlayableCharacter>(PLY.SelectedCharacter, TEXT("GM Character DataTable"), true);
				if (Char && Char->CharacterID != "None")
				{
					ACharacter* Character = GetWorld()->SpawnActorDeferred<ACharacter>(Char->CharacterClass, FTransform(), nullptr, nullptr, ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);
					if (Character)
					{
						Character->FinishSpawning(GetRandomPlayerSpawn(), false, nullptr);
						// Fixed: Moved Possess inside null check to prevent crash if spawn fails
						PC->Possess(Character);
					}
				}
				else {
					//If Character not found, Spawn Default Pawn.
					ACharacter* Character = GetWorld()->SpawnActorDeferred<ACharacter>(AHordeBaseCharacter::StaticClass(), FTransform(), nullptr, nullptr, ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);
					if (Character)
					{
						Character->FinishSpawning(GetRandomPlayerSpawn(), false, nullptr);
						// Fixed: Moved Possess inside null check to prevent crash if spawn fails
						PC->Possess(Character);
					}
				}
			}

		}
	}
}

/**
 *	Starts Zombie Spawning with given amount. If no spawn point left we try to spawn until one is left.
 *
 * @param Amount of Zeds to be Spawned.
 * @return void
 */
void AHordeGameMode::InitiateZombieSpawning(int32 Amount)
{
	ZedsLeftToSpawn = Amount;
	TArray<AActor*> FreeSpawnPoints;
	int32 AmountOfFreePoints = 0;
	GetAISpawner(FreeSpawnPoints, AmountOfFreePoints);
	if (ZedsLeftToSpawn > 0)
	{
		for (int32 i = 0; i < ((Amount > AmountOfFreePoints) ? AmountOfFreePoints : Amount); i++)
		{
			AZedPawn* NewZed = GetWorld()->SpawnActorDeferred<AZedPawn>(AZedPawn::StaticClass(), FTransform(), nullptr, nullptr, ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);
			if (NewZed)
			{
				AAISpawnPoint* Spawner = Cast<AAISpawnPoint>(FreeSpawnPoints[FMath::RandRange(0, FreeSpawnPoints.Num() -1)]);
				if (Spawner)
				{
					NewZed->PatrolTag = Spawner->PatrolTag;
					ZedsLeftToSpawn--;
					FTransform SpawnTrans;
					SpawnTrans.SetLocation(Spawner->GetActorLocation());
					NewZed->FinishSpawning(SpawnTrans, false, nullptr);
				}
				else {
					NewZed->Destroy();
				}

			}
		}
		if (ZedsLeftToSpawn > 0)
		{
			FTimerHandle RestartSpawningHandle;
			FTimerDelegate RestartSpawningDelegate;
			RestartSpawningDelegate.BindLambda([this] {
				InitiateZombieSpawning(ZedsLeftToSpawn);
				});
			GetWorld()->GetTimerManager().SetTimer(RestartSpawningHandle, RestartSpawningDelegate, 3.f, false);
		}
	}
}

