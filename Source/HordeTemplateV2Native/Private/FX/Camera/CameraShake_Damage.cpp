// Copyright (c) 2018 - 2026 Marc Fraedrich
// Horde Template V2 (Native) - A cooperative zombie survival game framework
// Licensed under the MIT License

#include "CameraShake_Damage.h"
#include "PerlinNoiseCameraShakePattern.h"

/**
 * @file CameraShake_Damage.cpp
 * @brief AAA-quality damage camera shake implementation
 * @author Marc Fraedrich
 */

UCameraShake_Damage::UCameraShake_Damage()
{
	// Create Perlin noise pattern for organic hit feel
	UPerlinNoiseCameraShakePattern* ShakePattern = CreateDefaultSubobject<UPerlinNoiseCameraShakePattern>(TEXT("ShakePattern"));

	// Quick sharp hit with disorienting recovery
	ShakePattern->Duration = 0.4f;
	ShakePattern->BlendInTime = 0.02f;
	ShakePattern->BlendOutTime = 0.25f;

	// Location shake - jarring impact feel
	ShakePattern->LocationAmplitudeMultiplier = 1.0f;
	ShakePattern->LocationFrequencyMultiplier = 1.0f;

	ShakePattern->X.Amplitude = 8.0f;
	ShakePattern->X.Frequency = 45.0f;

	ShakePattern->Y.Amplitude = 8.0f;
	ShakePattern->Y.Frequency = 40.0f;

	ShakePattern->Z.Amplitude = 10.0f;
	ShakePattern->Z.Frequency = 50.0f;

	// Strong rotation for disorientation
	ShakePattern->RotationAmplitudeMultiplier = 1.0f;
	ShakePattern->RotationFrequencyMultiplier = 1.0f;

	ShakePattern->Pitch.Amplitude = 4.0f;
	ShakePattern->Pitch.Frequency = 35.0f;

	ShakePattern->Yaw.Amplitude = 3.0f;
	ShakePattern->Yaw.Frequency = 30.0f;

	ShakePattern->Roll.Amplitude = 2.5f;
	ShakePattern->Roll.Frequency = 40.0f;

	// Subtle FOV punch for impact
	ShakePattern->FOV.Amplitude = 2.0f;
	ShakePattern->FOV.Frequency = 25.0f;

	SetRootShakePattern(ShakePattern);
}
