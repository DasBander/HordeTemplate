// Copyright (c) 2018 - 2026 Marc Fraedrich
// Horde Template V2 (Native) - A cooperative zombie survival game framework
// Licensed under the MIT License

#pragma once

/**
 * @file HordeGameMode.h
 * @brief Main game mode handling wave-based survival, lobby management, player spawning, and round progression
 * @author Marc Fraedrich
 */

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "LobbyStructures.h"
#include "HordeGameMode.generated.h"


UENUM(BlueprintType)
enum class EMatchMode : uint8
{
	EMatchModeLinear UMETA(DisplayName = "Linear"),
	EMatchModeNonLinear UMETA(DisplayName = "Nonlinear")
};


/**
 * 
 */
UCLASS()
class HORDETEMPLATEV2NATIVE_API AHordeGameMode : public AGameMode
{
	GENERATED_BODY()
	
protected:
	UPROPERTY()
	int32 ZedsLeftToSpawn = 0;


public:
	AHordeGameMode();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Zeds")
	void GetAISpawner(TArray<AActor*>& Spawner, int32 &FreePoints);

	UFUNCTION()
		void CheckGameOver();

	UFUNCTION()
		void SpawnSpectator(APlayerController* PC);

	FVector GetSpectatorSpawnLocation();

	APlayerController* GetControllerByID(FString PlayerID);

	FTransform GetRandomPlayerSpawn();

	virtual void Logout(AController* Exiting) override;

	void UpdatePlayerLobby();
	void GameStart(const TArray<FPlayerInfo>& LobbyPlayers);

	void InitiateZombieSpawning(int32 Amount);
};
