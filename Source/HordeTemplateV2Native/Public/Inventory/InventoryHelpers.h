// Copyright (c) 2018 - 2026 Marc Fraedrich
// Horde Template V2 (Native) - A cooperative zombie survival game framework
// Licensed under the MIT License

#pragma once

/**
 * @file InventoryHelpers.h
 * @brief Blueprint function library for inventory data table lookups
 * @author Marc Fraedrich
 */

#include "CoreMinimal.h"
#include "Gameplay/GameplayStructures.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "InventoryHelpers.generated.h"
UCLASS()
class HORDETEMPLATEV2NATIVE_API UInventoryHelpers : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:

	UInventoryHelpers();


	UFUNCTION(BlueprintPure, Category = "Inventory|Helper")
		static FItem FindItemByID(FName ItemID);
};
