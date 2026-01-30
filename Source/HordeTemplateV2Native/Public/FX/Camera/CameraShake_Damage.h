// Copyright (c) 2018 - 2026 Marc Fraedrich
// Horde Template V2 (Native) - A cooperative zombie survival game framework
// Licensed under the MIT License

#pragma once

/**
 * @file CameraShake_Damage.h
 * @brief AAA-quality camera shake when player takes damage
 * @author Marc Fraedrich
 */

#include "CoreMinimal.h"
#include "Camera/CameraShakeBase.h"
#include "PerlinNoiseCameraShakePattern.h"
#include "CameraShake_Damage.generated.h"

/**
 * AAA-quality damage camera shake - sharp hit with disorienting feel
 */
UCLASS(Blueprintable, BlueprintType)
class HORDETEMPLATEV2NATIVE_API UCameraShake_Damage : public UCameraShakeBase
{
	GENERATED_BODY()

public:
	UCameraShake_Damage();
};
