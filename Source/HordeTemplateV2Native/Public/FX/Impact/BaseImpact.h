// Copyright (c) 2018 - 2026 Marc Fraedrich
// Horde Template V2 (Native) - A cooperative zombie survival game framework
// Licensed under the MIT License

#pragma once

/**
 * @file BaseImpact.h
 * @brief Base impact effect actor with particle system and audio for projectile hits
 * @author Marc Fraedrich
 */

#include "CoreMinimal.h"
#include "Particles/ParticleSystemComponent.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "GameFramework/Actor.h"
#include "BaseImpact.generated.h"

UCLASS()
class HORDETEMPLATEV2NATIVE_API ABaseImpact : public AActor
{
	GENERATED_BODY()
	
public:	
	ABaseImpact();

protected:
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, Category = "FX")
		class UParticleSystemComponent* ImpactFX;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, Category = "FX")
		class UAudioComponent* ImpactSound;

};
