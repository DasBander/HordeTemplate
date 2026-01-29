// Copyright (c) 2018 - 2026 Marc Fraedrich
// Horde Template V2 (Native) - A cooperative zombie survival game framework
// Licensed under the MIT License

#pragma once

/**
 * @file LobbyCharacterTradeWidget.h
 * @brief Widget for character trading interface between players in the lobby
 * @author Marc Fraedrich
 */

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LobbyCharacterTradeWidget.generated.h"
UCLASS()
class HORDETEMPLATEV2NATIVE_API ULobbyCharacterTradeWidget : public UUserWidget
{
	GENERATED_BODY()
	

public:

	UFUNCTION(BlueprintPure, Category = "Character Trade Widget")
		FText GetTradeTime();

	UFUNCTION(BlueprintPure, Category = "Character Trade Widget")
		FText GetTargetCharacterName();

	UFUNCTION(BlueprintPure, Category = "Character Trade Widget")
		FSlateBrush GetTargetCharacterImage();

	UFUNCTION(BlueprintPure, Category = "Character Trade Widget")
		FText GetOwnCharacterName();

	UFUNCTION(BlueprintPure, Category = "Character Trade Widget")
		FSlateBrush GetOwnCharacterImage();

	UFUNCTION(BlueprintPure, Category = "Character Trade Widget")
		ESlateVisibility IsInCharacterTrade();
};
