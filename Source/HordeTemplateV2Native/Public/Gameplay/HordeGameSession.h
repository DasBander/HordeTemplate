// Copyright (c) 2018 - 2026 Marc Fraedrich
// Horde Template V2 (Native) - A cooperative zombie survival game framework
// Licensed under the MIT License

#pragma once

/**
 * @file HordeGameSession.h
 * @brief Game session managing online subsystem session lifecycle and cleanup
 * @author Marc Fraedrich
 */

#include "CoreMinimal.h"
#include "Online.h"
#include "GameFramework/GameSession.h"
#include "HordeGameSession.generated.h"

/**
 * 
 */
UCLASS()
class HORDETEMPLATEV2NATIVE_API AHordeGameSession : public AGameSession
{
	GENERATED_BODY()

public:

	void EndGameSession();
	
};
