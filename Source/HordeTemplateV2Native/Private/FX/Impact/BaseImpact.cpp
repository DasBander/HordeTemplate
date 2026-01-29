// Copyright (c) 2018 - 2026 Marc Fraedrich
// Horde Template V2 (Native) - A cooperative zombie survival game framework
// Licensed under the MIT License

#include "BaseImpact.h"

/**
 * @file BaseImpact.cpp
 * @brief Implementation of base impact effect with particle and sound setup
 * @author Marc Fraedrich
 */
#include "UObject/ConstructorHelpers.h"

/**
 *	Constructor for ABaseImpact. Constructs Base Impact by setting the Particle and Impact Sound.
 *
 * @param
 * @return
 */
ABaseImpact::ABaseImpact()
{
	/*
		We don't want this actor the be Tick or Replicated so we can Improve performance highly.
	*/
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	ImpactFX = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("Impact Particle"));
	ImpactFX->SetupAttachment(RootComponent);

	ImpactSound = CreateDefaultSubobject<UAudioComponent>(TEXT("Impact Sound"));
	ImpactSound->SetupAttachment(RootComponent);


	const ConstructorHelpers::FObjectFinder<USoundCue> BaseImpactSoundAsset(TEXT("SoundCue'/Game/HordeTemplateBP/Assets/Sounds/A_bullet_impact_Cue.A_bullet_impact_Cue'"));
	if (BaseImpactSoundAsset.Succeeded())
	{
		ImpactSound->SetSound(BaseImpactSoundAsset.Object);
	}

	const ConstructorHelpers::FObjectFinder<UParticleSystem> BaseImpactFXAsset(TEXT("ParticleSystem'/Game/HordeTemplateBP/Assets/Effects/ParticleSystems/Weapons/AssaultRifle/Impacts/P_AssaultRifle_IH.P_AssaultRifle_IH'"));
	if (BaseImpactFXAsset.Succeeded())
	{
		ImpactFX->SetTemplate(BaseImpactFXAsset.Object);
	}
}



