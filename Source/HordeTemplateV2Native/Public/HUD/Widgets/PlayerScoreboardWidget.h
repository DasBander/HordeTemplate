// Copyright (c) 2018 - 2026 Marc Fraedrich
// Horde Template V2 (Native) - A cooperative zombie survival game framework
// Licensed under the MIT License

#pragma once

/**
 * @file PlayerScoreboardWidget.h
 * @brief Scoreboard widget displaying all players with their scores and status
 * @author Marc Fraedrich
 */

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PlayerScoreboardWidget.generated.h"
UCLASS()
class HORDETEMPLATEV2NATIVE_API UPlayerScoreboardWidget : public UUserWidget
{
	GENERATED_BODY()
public:

	void UpdatePlayerList(const TArray<APlayerState*>& PlayerList);

	UFUNCTION(BlueprintImplementableEvent, Category = "Player List")
		void OnPlayerListUpdated(const TArray<APlayerState*>& PlayerList);

	UFUNCTION(BlueprintPure, Category = "Player List")
		FText GetLobbyName();
};
