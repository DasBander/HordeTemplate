// Copyright (c) 2018 - 2026 Marc Fraedrich
// Horde Template V2 (Native) - A cooperative zombie survival game framework
// Licensed under the MIT License

#include "CameraShake_Explosion.h"
#include "PerlinNoiseCameraShakePattern.h"

/**
 * @file CameraShake_Explosion.cpp
 * @brief AAA-quality explosion camera shake implementation
 * @author Marc Fraedrich
 */

UCameraShake_Explosion::UCameraShake_Explosion()
{
	// Create and configure Perlin noise shake pattern for organic, high-quality feel
	UPerlinNoiseCameraShakePattern* ShakePattern = CreateDefaultSubobject<UPerlinNoiseCameraShakePattern>(TEXT("ShakePattern"));

	// Timing - quick impact with smooth decay
	ShakePattern->Duration = 0.75f;
	ShakePattern->BlendInTime = 0.05f;
	ShakePattern->BlendOutTime = 0.4f;

	// Location shake (camera position offset) - creates visceral impact feel
	ShakePattern->LocationAmplitudeMultiplier = 1.0f;
	ShakePattern->LocationFrequencyMultiplier = 1.0f;

	ShakePattern->X.Amplitude = 15.0f;
	ShakePattern->X.Frequency = 40.0f;

	ShakePattern->Y.Amplitude = 15.0f;
	ShakePattern->Y.Frequency = 35.0f;

	ShakePattern->Z.Amplitude = 25.0f;
	ShakePattern->Z.Frequency = 50.0f;

	// Rotation shake (camera rotation) - adds disorientation
	ShakePattern->RotationAmplitudeMultiplier = 1.0f;
	ShakePattern->RotationFrequencyMultiplier = 1.0f;

	ShakePattern->Pitch.Amplitude = 5.0f;
	ShakePattern->Pitch.Frequency = 30.0f;

	ShakePattern->Yaw.Amplitude = 3.0f;
	ShakePattern->Yaw.Frequency = 25.0f;

	ShakePattern->Roll.Amplitude = 4.0f;
	ShakePattern->Roll.Frequency = 35.0f;

	// FOV shake - subtle zoom punch for impact
	ShakePattern->FOV.Amplitude = 3.0f;
	ShakePattern->FOV.Frequency = 20.0f;

	// Set the pattern
	SetRootShakePattern(ShakePattern);
}
