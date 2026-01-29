// Copyright (c) 2018 - 2026 Marc Fraedrich
// Horde Template V2 (Native) - A cooperative zombie survival game framework
// Licensed under the MIT License

#pragma once

/**
 * @file HordeGameInstance.h
 * @brief Game instance storing persistent session data like lobby name and map rotation
 * @author Marc Fraedrich
 */

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "HordeGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class HORDETEMPLATEV2NATIVE_API UHordeGameInstance : public UGameInstance
{
	GENERATED_BODY()
public:

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category="Server Creation")
		FString LobbyName = "Horde Game - Lobby";

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Server Creation")
		TArray<FName> MapRotation;
};
