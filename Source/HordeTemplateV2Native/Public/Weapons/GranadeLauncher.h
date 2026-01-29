// Copyright (c) 2018 - 2026 Marc Fraedrich
// Horde Template V2 (Native) - A cooperative zombie survival game framework
// Licensed under the MIT License

#pragma once

/**
 * @file GranadeLauncher.h
 * @brief Grenade launcher weapon that fires explosive projectiles without muzzle flash
 * @author Marc Fraedrich
 */

#include "CoreMinimal.h"
#include "Weapons/BaseFirearm.h"
#include "GranadeLauncher.generated.h"

/**
 * 
 */
UCLASS()
class HORDETEMPLATEV2NATIVE_API AGranadeLauncher : public ABaseFirearm
{
	GENERATED_BODY()

public:

	AGranadeLauncher();
	
};
