// Copyright (c) 2018 - 2026 Marc Fraedrich
// Horde Template V2 (Native) - A cooperative zombie survival game framework
// Licensed under the MIT License

#pragma once

/**
 * @file BaseSpectator.h
 * @brief Spectator pawn that allows dead players to view other alive players
 * @author Marc Fraedrich
 */

#include "CoreMinimal.h"
#include "GameFramework/SpectatorPawn.h"
#include "Character/HordeBaseCharacter.h"
#include "BaseSpectator.generated.h"

/**
 * 
 */
UCLASS()
class HORDETEMPLATEV2NATIVE_API ABaseSpectator : public ASpectatorPawn
{
	GENERATED_BODY()

public:

	ABaseSpectator();

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION(Server, WithValidation, Reliable, Category = "Spectator")
		void ServerFocusPlayer();

	UFUNCTION(Client, Reliable, Category = "Spectator")
		void ClientFocusPlayer(AHordeBaseCharacter* Player);

	AHordeBaseCharacter* GetRandomAlivePlayer();
};
