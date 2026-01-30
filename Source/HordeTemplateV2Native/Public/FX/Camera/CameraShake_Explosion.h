// Copyright (c) 2018 - 2026 Marc Fraedrich
// Horde Template V2 (Native) - A cooperative zombie survival game framework
// Licensed under the MIT License

#pragma once

/**
 * @file CameraShake_Explosion.h
 * @brief AAA-quality camera shake system for explosions
 * @author Marc Fraedrich
 */

#include "CoreMinimal.h"
#include "Camera/CameraShakeBase.h"
#include "PerlinNoiseCameraShakePattern.h"
#include "CameraShake_Explosion.generated.h"

/**
 * AAA-quality explosion camera shake using Perlin noise for organic feel
 */
UCLASS(Blueprintable, BlueprintType)
class HORDETEMPLATEV2NATIVE_API UCameraShake_Explosion : public UCameraShakeBase
{
	GENERATED_BODY()

public:
	UCameraShake_Explosion();
};
