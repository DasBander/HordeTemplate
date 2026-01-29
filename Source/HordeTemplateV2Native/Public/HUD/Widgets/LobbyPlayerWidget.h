// Copyright (c) 2018 - 2026 Marc Fraedrich
// Horde Template V2 (Native) - A cooperative zombie survival game framework
// Licensed under the MIT License

#pragma once

/**
 * @file LobbyPlayerWidget.h
 * @brief Widget displaying individual player information in the lobby player list
 * @author Marc Fraedrich
 */

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Gameplay/LobbyStructures.h"
#include "LobbyPlayerWidget.generated.h"
UCLASS()
class HORDETEMPLATEV2NATIVE_API ULobbyPlayerWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintReadOnly, Category = "Lobby Player Info", meta=(ExposeOnSpawn=true))
		FPlayerInfo PlayerInfo;

	UPROPERTY(BlueprintReadOnly, Category = "Lobby Player Info")
		FPlayableCharacter Character;

	UFUNCTION(BlueprintPure, Category = "Lobby Player Info")
		FLinearColor GetReadyColor();

	UFUNCTION(BlueprintPure, Category = "Lobby Player Info")
		ESlateVisibility GetCharacterAvailableVisibility();

	UFUNCTION(BlueprintPure, Category = "Lobby Player Info")
		ESlateVisibility GetTradingAvailableVisibility();

	UFUNCTION(BlueprintCallable, Category = "Player Trading")
		void InitiateTrade();

	UFUNCTION(BlueprintPure, Category = "Lobby Player Info")
		bool IsAdmin();

	UFUNCTION(BlueprintPure, Category = "Lobby Player Info")
		bool IsOwner();

	virtual void NativeConstruct() override;
};
