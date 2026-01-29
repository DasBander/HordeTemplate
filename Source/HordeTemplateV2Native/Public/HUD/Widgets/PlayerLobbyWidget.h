// Copyright (c) 2018 - 2026 Marc Fraedrich
// Horde Template V2 (Native) - A cooperative zombie survival game framework
// Licensed under the MIT License

#pragma once

/**
 * @file PlayerLobbyWidget.h
 * @brief Main lobby interface for character selection, ready state, and game start
 * @author Marc Fraedrich
 */

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Gameplay/LobbyStructures.h"
#include "PlayerLobbyWidget.generated.h"
UCLASS()
class HORDETEMPLATEV2NATIVE_API UPlayerLobbyWidget : public UUserWidget
{
	GENERATED_BODY()
	
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLobbyPlayersUpdated, const TArray<FPlayerInfo>&, LobbyPlayers);
protected:
		virtual void NativeConstruct() override;

	

public:
	UPROPERTY()
		FOnLobbyPlayersUpdated OnLobbyPlayersUpdateDelegate;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lobby")
		FLobbyInfo LobbyInfo;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lobby")
		FPlayableLevel MapInfo;

	UFUNCTION(BlueprintImplementableEvent, Category = "Lobby")
		void OnLobbyPlayersUpdated(const TArray<FPlayerInfo>& LobbyPlayers);

	UFUNCTION(BlueprintPure, Category = "Lobby")
		FText GetLobbyTime();

	UFUNCTION(BlueprintPure, Category = "Lobby")
		bool IsDisconnectBlocked();

	UFUNCTION(BlueprintPure, Category = "Lobby")
		ESlateVisibility IsCharacterTrading();
	
	UFUNCTION(BlueprintPure, Category = "Lobby")
		bool bIsGameStarting();
};
