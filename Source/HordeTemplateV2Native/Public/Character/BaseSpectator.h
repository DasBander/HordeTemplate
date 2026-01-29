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

	virtual void Tick(float DeltaTime) override;

	UFUNCTION(Server, WithValidation, Reliable, Category = "Spectator")
		void ServerFocusPlayer();

	UFUNCTION(Client, Reliable, Category = "Spectator")
		void ClientFocusPlayer(AHordeBaseCharacter* Player);

	AHordeBaseCharacter* GetRandomAlivePlayer();

protected:
	/** Current player being spectated */
	UPROPERTY()
	AHordeBaseCharacter* CurrentSpectateTarget = nullptr;

	/** Smoothed camera location for spectating */
	FVector SmoothedCameraLocation = FVector::ZeroVector;

	/** Smoothed camera rotation for spectating */
	FRotator SmoothedCameraRotation = FRotator::ZeroRotator;

	/** Whether we've initialized the smoothed values */
	bool bSmoothedValuesInitialized = false;

	/** Interpolation speed for camera smoothing */
	static constexpr float CameraInterpSpeed = 15.f;
};
