// Copyright (c) 2018 - 2026 Marc Fraedrich
// Horde Template V2 (Native) - A cooperative zombie survival game framework
// Licensed under the MIT License

#pragma once

/**
 * @file PlayerTraderWidget.h
 * @brief Trader shop interface for purchasing weapons and items between rounds
 * @author Marc Fraedrich
 */

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PlayerTraderWidget.generated.h"
UCLASS()
class HORDETEMPLATEV2NATIVE_API UPlayerTraderWidget : public UUserWidget
{
	GENERATED_BODY()
protected:
	virtual void NativeConstruct() override;

	UFUNCTION(BlueprintPure, Category = "Economy")
		FText GetPlayerMoney();
};
