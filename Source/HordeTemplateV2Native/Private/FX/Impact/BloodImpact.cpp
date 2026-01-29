
// Copyright (c) 2018 - 2026 Marc Fraedrich
// Horde Template V2 (Native) - A cooperative zombie survival game framework
// Licensed under the MIT License

#include "BloodImpact.h"

/**
 * @file BloodImpact.cpp
 * @brief Implementation of blood impact with flesh-specific particle and sound effects
 * @author Marc Fraedrich
 */
#include "UObject/ConstructorHelpers.h"

/**
 *	Constructor for ABloodImpact. Overriding Impact Sound and Particle.
 *
 * @param
 * @return
 */
ABloodImpact::ABloodImpact()
{
	const ConstructorHelpers::FObjectFinder<USoundCue> BloodImpactSound(TEXT("SoundCue'/Game/HordeTemplateBP/Assets/Sounds/A_FleshImpact_Cue.A_FleshImpact_Cue'"));
	if (BloodImpactSound.Succeeded())
	{
		ImpactSound->SetSound(BloodImpactSound.Object);
	}

	const ConstructorHelpers::FObjectFinder<UParticleSystem> BloodParticleAsset(TEXT("ParticleSystem'/Game/HordeTemplateBP/Assets/Effects/ParticleSystems/Gameplay/Player/P_body_bullet_impact.P_body_bullet_impact'"));
	if (BloodParticleAsset.Succeeded())
	{
		ImpactFX->SetTemplate(BloodParticleAsset.Object);
	}

	SetLifeSpan(3.f);
}
