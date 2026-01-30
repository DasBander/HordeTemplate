// Copyright (c) 2018 - 2026 Marc Fraedrich
// Horde Template V2 (Native) - A cooperative zombie survival game framework
// Licensed under the MIT License

#include "CameraShake_RifleFire.h"
#include "PerlinNoiseCameraShakePattern.h"

/**
 * @file CameraShake_RifleFire.cpp
 * @brief AAA-quality weapon fire camera shake implementation
 * @author Marc Fraedrich
 */

UCameraShake_RifleFire::UCameraShake_RifleFire()
{
	// Create Perlin noise pattern for organic weapon kick
	UPerlinNoiseCameraShakePattern* ShakePattern = CreateDefaultSubobject<UPerlinNoiseCameraShakePattern>(TEXT("ShakePattern"));

	// Very quick timing - snappy recoil feel
	ShakePattern->Duration = 0.15f;
	ShakePattern->BlendInTime = 0.01f;
	ShakePattern->BlendOutTime = 0.1f;

	// Minimal location shake for weapon fire
	ShakePattern->LocationAmplitudeMultiplier = 0.5f;
	ShakePattern->LocationFrequencyMultiplier = 1.0f;

	ShakePattern->X.Amplitude = 0.5f;
	ShakePattern->X.Frequency = 60.0f;

	ShakePattern->Y.Amplitude = 0.3f;
	ShakePattern->Y.Frequency = 50.0f;

	ShakePattern->Z.Amplitude = 1.0f;
	ShakePattern->Z.Frequency = 70.0f;

	// Rotation - primary recoil feel (upward kick)
	ShakePattern->RotationAmplitudeMultiplier = 1.0f;
	ShakePattern->RotationFrequencyMultiplier = 1.0f;

	ShakePattern->Pitch.Amplitude = 1.5f;
	ShakePattern->Pitch.Frequency = 50.0f;

	ShakePattern->Yaw.Amplitude = 0.5f;
	ShakePattern->Yaw.Frequency = 40.0f;

	ShakePattern->Roll.Amplitude = 0.3f;
	ShakePattern->Roll.Frequency = 45.0f;

	// No FOV shake for weapon fire
	ShakePattern->FOV.Amplitude = 0.0f;
	ShakePattern->FOV.Frequency = 0.0f;

	SetRootShakePattern(ShakePattern);
}
