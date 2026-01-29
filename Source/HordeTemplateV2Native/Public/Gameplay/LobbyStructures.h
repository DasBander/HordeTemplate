// Copyright (c) 2018 - 2026 Marc Fraedrich
// Horde Template V2 (Native) - A cooperative zombie survival game framework
// Licensed under the MIT License

#pragma once

/**
 * @file LobbyStructures.h
 * @brief Lobby and multiplayer data structures for chat, player info, characters, levels, trades, and scoring
 * @author Marc Fraedrich
 */

#include "Engine/DataTable.h"
#include "GameFramework/Character.h"
#include "LobbyStructures.generated.h"

UENUM(BlueprintType)
enum class EGameStatus : uint8
{
	ELOBBY UMETA(DisplayName = "In-Lobby"),
	EINGAME UMETA(DisplayName = "In-Game"),
	EGAMEOVER UMETA(DisplayName = "Game Over"),
	ESERVERTRAVEL UMETA(DisplayName = "Server Travel")
};

UENUM(BlueprintType)
enum class EPointType : uint8
{
	EPointCasual UMETA(DisplayName = "Casual Kill"),
	EPointHeadShot UMETA(DisplayName = "Headshot")
};

USTRUCT(BlueprintType)
struct FHordeChatMessage
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chat")
		FString Sender = "SERVER";

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Info")
		FText Message;

	FHordeChatMessage() {}

	FHordeChatMessage(FString InMessage)
	{
		Message = FText::FromString(InMessage);
	}

	FHordeChatMessage(FString InSender, FText InMessage)
	{
		Sender = InSender;
		Message = InMessage;
	}

};

USTRUCT(BlueprintType)
struct FPlayerInfo : public FTableRowBase
{
	GENERATED_USTRUCT_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Info")
		FString UserName = "None";

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Info")
		FString PlayerID = "None";

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Info")
		FName SelectedCharacter = "None";

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Info")
		bool PlayerReady = false;

	FPlayerInfo() {}


};

USTRUCT(BlueprintType)
struct FPlayableCharacter : public FTableRowBase
{
	GENERATED_USTRUCT_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Info")
		FName CharacterID = "None";

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Info")
		FString CharacterTitle = "None";

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Info")
		UTexture2D * CharacterImage = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Info")
		TSubclassOf<ACharacter> CharacterClass = nullptr;

	FPlayableCharacter() {}


};

USTRUCT(BlueprintType)
struct FLobbyAvailableCharacters : public FTableRowBase
{
	GENERATED_USTRUCT_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Characters")
		FName CharacterID = "None";

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Characters")
		FString PlayerID = "";

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Characters")
		FString PlayerUsername = "";

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Characters")
		bool CharacterTaken = false;

	FLobbyAvailableCharacters() {}


};

USTRUCT(BlueprintType)
struct FLobbyInfo : public FTableRowBase
{
	GENERATED_USTRUCT_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lobby Info")
		FString LobbyName = "None";

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lobby Info")
		FString OwnerID = "None";

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lobby Info")
		TArray<FName> LobbyMapRotation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lobby Info")
		TArray<FLobbyAvailableCharacters> AvailableCharacters;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lobby Info")
		int32 MinimumStartingPlayers = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lobby Info")
		float DefaultLobbyTime = 300.f;

	FLobbyInfo() {}


};


USTRUCT(BlueprintType)
struct FPlayableLevel : public FTableRowBase
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Playable Levels")
		 FText LevelName = FText();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Playable Levels")
		FName RawLevelName = "TestingMap";

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Playable Levels")
		UTexture2D* LevelImage = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Playable Levels")
		UTexture2D* LobbyImage = nullptr;

	FPlayableLevel() {}


};
	
USTRUCT(BlueprintType)
struct FLobbyTrade
{
	GENERATED_USTRUCT_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Characters")
		FString Instigator = "";

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Characters")
		FString Target = "";

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Characters")
		float TimeLeft = 20;


	FLobbyTrade() {}

	FLobbyTrade(FString InInstigator, FString InTarget, float InTimeLeft) 
	{
		Instigator = InInstigator;
		Target = InTarget;
		TimeLeft = InTimeLeft;
	}

};

USTRUCT(BlueprintType)
struct FPlayerScore
{
	GENERATED_USTRUCT_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Score")
		FString ScoreType = "None";

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Score")
		FString PlayerID = "";

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Score")
		int32 Score = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Score")
		FName Character;

	FPlayerScore() {}

	FPlayerScore(FString InScoreType)
	{
		ScoreType = InScoreType;
	}

	FPlayerScore(FString InScoreType, FString InPlayerID, int32 InScore, FName InCharacter)
	{
		ScoreType = InScoreType;
		PlayerID = InPlayerID;
		Score = InScore;
		Character = InCharacter;
	}

	

};