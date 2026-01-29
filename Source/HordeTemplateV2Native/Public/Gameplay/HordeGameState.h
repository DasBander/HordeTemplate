// Copyright (c) 2018 - 2026 Marc Fraedrich
// Horde Template V2 (Native) - A cooperative zombie survival game framework
// Licensed under the MIT License

#pragma once

/**
 * @file HordeGameState.h
 * @brief Replicated game state managing round info, player tracking, lobby, character trading, and scoring
 * @author Marc Fraedrich
 */

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "HordePlayerState.h"
#include "HordeGameMode.h"
#include "HordeWorldSettings.h"
#include "LobbyStructures.h"
#include "HordeGameState.generated.h"

/**
 * 
 */
UCLASS()
class HORDETEMPLATEV2NATIVE_API AHordeGameState : public AGameState
{
	GENERATED_BODY()

public:

	/*
	Lobby Setup	
	*/

	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Lobby")
		FLobbyInfo LobbyInformation;

	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Lobby")
		EGameStatus GameStatus  = EGameStatus::ELOBBY;

	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Lobby")
		EMatchMode MatchMode = EMatchMode::EMatchModeLinear;

	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Lobby")
		bool GameStarting = false;

	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Lobby")
		bool BlockDisconnect = false;

	UPROPERTY(BlueprintReadOnly, Category = "Lobby")
		TArray<FPlayerInfo> LobbyPlayers;

	virtual void BeginPlay() override;

	void ParseChatCommand(FString PlayerID, FString Command);
	
	UFUNCTION()
	void TakePlayer(FPlayerInfo Player);

	UFUNCTION()
		void KickPlayer(const FString& PlayerID);

	UFUNCTION(BlueprintPure, Category = "Settings")
		AHordeWorldSettings* GetHordeWorldSettings();

	UFUNCTION()
	void StartLobbyTimer();

	UFUNCTION()
		void GameOver(FName NextMap);

	UFUNCTION()
		void ProcessLobbyTime();

	UFUNCTION()
		void ResetLobbyTime();

	UFUNCTION()
		void UnreadyAllPlayers();

	UFUNCTION()
		void StartGame();

	UFUNCTION()
		bool IsCharacterTaken(FName CharacterID);

	UFUNCTION(BlueprintCallable, Category="Lobby")
		FName GetCharacterByID(FString PlayerID, int32 &CharacterIndex);

	UFUNCTION()
		AHordePlayerState* GetPlayerStateByID(FString PlayerID);

	UFUNCTION()
		void FreeupUnassignedCharacters();

	UFUNCTION()
	FString GetUsernameBySteamID(FString ID, bool& FoundPlayer);

	UPROPERTY(BlueprintReadOnly, Category = "Lobby")
	FTimerHandle LobbyTimer;

	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Lobby")
		float LobbyTime = 0.f;

	UFUNCTION()
	void UpdatePlayerLobby();

	UFUNCTION()
	bool CheckPlayersReady();

	UFUNCTION()
		void MessagePlayer(FHordeChatMessage Message, FString PlayerID);

	UFUNCTION()
	void PopMessage(FHordeChatMessage Message);

	UFUNCTION()
	FName GetFreeCharacter();

	UFUNCTION()
		void AllPlayerDeadCheck();

	UFUNCTION()
		void EndGame(bool ResetLevel);

	UFUNCTION()
		void ProcessEndTime();

	UFUNCTION()
		void ResetLobby();

	UFUNCTION()
		void CalcEndScore(FPlayerScore& MVP, FPlayerScore& HS, FPlayerScore& KS);

	UFUNCTION()
		int32 CountAlivePlayers();

	UFUNCTION()
		int32 CountAliveZeds();

	UFUNCTION()
		void UpdateAliveZeds();

	/*
	Lobby Character Trading
	*/
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Lobby")
		FLobbyTrade TradeProgress;

	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Lobby")
		bool IsTradeInProgress = false;

	UPROPERTY()
		FTimerHandle LobbyTradeTimer;

	UFUNCTION()
		void StartCharacterTrade(FString InstigatorPlayer, FString TargetPlayer);

	UFUNCTION()
		void ProcessCharacterTrade();

	UFUNCTION()
		void AcceptCharacterTrade();

	UFUNCTION()
		void AbortLobbyTrade();


	/*
	Round Base Game ( Non Linear )
	*/

	UFUNCTION()
		void StartRoundBasedGame();

	UFUNCTION()
		void ProcessPauseTime();

	UFUNCTION()
		void StartGameRound();

	UFUNCTION()
		void ProcessRoundTime();

	UFUNCTION()
		FName GetNextLevelInRotation(bool ResetLevel);

	UFUNCTION()
		void EndGameRound();

	UPROPERTY()
		FTimerHandle RoundTimer;

	UPROPERTY()
		FTimerHandle PauseTimer;

	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Round Based")
		float RoundTime = 0.f;

	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Round Based")
		float PauseTime = 0.f;

	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Round Based")
		bool IsRoundPaused = false;

	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Round Based")
		int32 GameRound = 0;

	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Game")
		int32 ZedsLeft = 0;

	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Score")
		FPlayerScore Score_MVP;
	
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Score")
		FPlayerScore Score_MostHeadshots;

	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Score")
		FPlayerScore Score_MostKills;

	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Game")
		FName NextLevel = TEXT("None");

	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Game")
		float EndTime = 20.f;

	UPROPERTY(BlueprintReadOnly, Category = "Game")
		FTimerHandle EndGameTimer;
};
