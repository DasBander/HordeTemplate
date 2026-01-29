// Copyright (c) 2018 - 2026 Marc Fraedrich
// Horde Template V2 (Native) - A cooperative zombie survival game framework
// Licensed under the MIT License

#pragma once

/**
 * @file PlayerTravelWidget.h
 * @brief Loading screen widget displayed during server travel and map transitions
 * @author Marc Fraedrich
 */

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PlayerTravelWidget.generated.h"
UCLASS()
class HORDETEMPLATEV2NATIVE_API UPlayerTravelWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "Travel Widget")
		FText GetServerInfo();
	
};
