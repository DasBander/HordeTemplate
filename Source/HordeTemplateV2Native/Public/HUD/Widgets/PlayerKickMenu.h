// Copyright (c) 2018 - 2026 Marc Fraedrich
// Horde Template V2 (Native) - A cooperative zombie survival game framework
// Licensed under the MIT License

#pragma once

/**
 * @file PlayerKickMenu.h
 * @brief Admin menu widget for kicking players from the lobby
 * @author Marc Fraedrich
 */

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Gameplay/LobbyStructures.h"
#include "PlayerKickMenu.generated.h"
UCLASS()
class HORDETEMPLATEV2NATIVE_API UPlayerKickMenu : public UUserWidget
{
	GENERATED_BODY()
	
public:

	UPROPERTY(BlueprintReadOnly, meta = (ExposeOnSpawn = true))
		FPlayerInfo PlyInfo;

	UFUNCTION(BlueprintCallable, Category = "Lobby")
		void KickPlayer();
};
