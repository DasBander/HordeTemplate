// Copyright (c) 2018 - 2026 Marc Fraedrich
// Horde Template V2 (Native) - A cooperative zombie survival game framework
// Licensed under the MIT License

#pragma once

/**
 * @file PlayerHeadDisplay.h
 * @brief Overhead player widget displaying name and health bar above characters
 * @author Marc Fraedrich
 */

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PlayerHeadDisplay.generated.h"
UCLASS()
class HORDETEMPLATEV2NATIVE_API UPlayerHeadDisplay : public UUserWidget
{
	GENERATED_BODY()

		DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnShowWidget);
		DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnHideWidget);

public:
	virtual void NativeConstruct() override;

	UFUNCTION(BlueprintImplementableEvent, Category = "Interaction")
		void OnShowWidget();

	UFUNCTION(BlueprintImplementableEvent, Category = "Interaction")
		void OnHideWidget();

	UPROPERTY(BlueprintReadOnly, Category = "Player Head Display")
		FOnShowWidget OnShowWidgetDelegate;

	UPROPERTY(BlueprintReadOnly, Category = "Player Head Display")
		FOnShowWidget OnHideWidgetDelegate;

	UPROPERTY(BlueprintReadOnly, Category = "Player Head Display")
		FString PlayerName;

	UPROPERTY(BlueprintReadOnly, Category = "Player Head Display")
		float Health;

	UFUNCTION(BlueprintPure, Category = "Player Head Display")
		FText GetPlayerName();
	
	UFUNCTION(BlueprintPure, Category = "Player Head Display")
		float GetPlayerHealth();

	UPROPERTY()
		float InterpHealth = 100.f;
};
