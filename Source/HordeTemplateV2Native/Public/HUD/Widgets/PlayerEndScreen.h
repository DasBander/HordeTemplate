// Copyright (c) 2018 - 2026 Marc Fraedrich
// Horde Template V2 (Native) - A cooperative zombie survival game framework
// Licensed under the MIT License

#pragma once

/**
 * @file PlayerEndScreen.h
 * @brief End-game screen displaying match statistics, MVP, and player scores
 * @author Marc Fraedrich
 */

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Gameplay/LobbyStructures.h"
#include "PlayerEndScreen.generated.h"
UCLASS()
class HORDETEMPLATEV2NATIVE_API UPlayerEndScreen : public UUserWidget
{
	GENERATED_BODY()
	
protected:

	UFUNCTION(BlueprintPure, Category = "End Screen")
		FText GetMVPName();

	UFUNCTION(BlueprintPure, Category = "End Screen")
		FText GetMHSName();

	UFUNCTION(BlueprintPure, Category = "End Screen")
		FText GetMKSName();

	UFUNCTION(BlueprintPure, Category = "End Screen")
		FText GetMVPPoints();

	UFUNCTION(BlueprintPure, Category = "End Screen")
		FText GetMHSPoints();

	UFUNCTION(BlueprintPure, Category = "End Screen")
		FText GetMKSPoints();

	UFUNCTION(BlueprintPure, Category = "End Screen")
		FText GetEndTime();

	UFUNCTION(BlueprintPure, Category = "End Screen")
		FText GetNextLevel();

	UFUNCTION(BlueprintPure, Category = "End Screen")
	FPlayableLevel FindLevelByID(FName LevelID);
};
