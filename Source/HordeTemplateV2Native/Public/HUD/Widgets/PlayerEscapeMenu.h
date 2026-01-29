// Copyright (c) 2018 - 2026 Marc Fraedrich
// Horde Template V2 (Native) - A cooperative zombie survival game framework
// Licensed under the MIT License

#pragma once

/**
 * @file PlayerEscapeMenu.h
 * @brief Pause menu widget with options to disconnect or close the menu
 * @author Marc Fraedrich
 */

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PlayerEscapeMenu.generated.h"
UCLASS()
class HORDETEMPLATEV2NATIVE_API UPlayerEscapeMenu : public UUserWidget
{
	GENERATED_BODY()
	
public:

	UFUNCTION(BlueprintCallable, Category = "Escape Menu")
		void DisconnectFromServer();

	UFUNCTION(BlueprintCallable, Category = "Escape Menu")
		void CloseEscapeMenu();
};
