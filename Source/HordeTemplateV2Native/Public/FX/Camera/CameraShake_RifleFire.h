// Copyright (c) 2018 - 2026 Marc Fraedrich
// Horde Template V2 (Native) - A cooperative zombie survival game framework
// Licensed under the MIT License

#pragma once

/**
 * @file CameraShake_RifleFire.h
 * @brief AAA-quality camera shake for weapon fire with snappy recoil feel
 * @author Marc Fraedrich
 */

#include "CoreMinimal.h"
#include "Camera/CameraShakeBase.h"
#include "PerlinNoiseCameraShakePattern.h"
#include "CameraShake_RifleFire.generated.h"

/**
 * AAA-quality weapon fire camera shake - snappy recoil with quick recovery
 */
UCLASS(Blueprintable, BlueprintType)
class HORDETEMPLATEV2NATIVE_API UCameraShake_RifleFire : public UCameraShakeBase
{
	GENERATED_BODY()

public:
	UCameraShake_RifleFire();
};
