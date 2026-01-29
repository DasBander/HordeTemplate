// Copyright (c) 2018 - 2026 Marc Fraedrich
// Horde Template V2 (Native) - A cooperative zombie survival game framework
// Licensed under the MIT License

#pragma once

/**
 * @file EndGameTrigger.h
 * @brief Collision trigger that ends the game when a player enters
 * @author Marc Fraedrich
 */

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "EndGameTrigger.generated.h"

UCLASS()
class HORDETEMPLATEV2NATIVE_API AEndGameTrigger : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AEndGameTrigger();

protected:

	UPROPERTY()
	class UBoxComponent* BoxComponent;

	UFUNCTION()
		void OnColide(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

};
