// Copyright (c) 2018 - 2026 Marc Fraedrich
// Horde Template V2 (Native) - A cooperative zombie survival game framework
// Licensed under the MIT License

#pragma once

/**
 * @file BloodImpact.h
 * @brief Blood impact effect for projectile hits on flesh surfaces
 * @author Marc Fraedrich
 */

#include "CoreMinimal.h"
#include "FX/Impact/BaseImpact.h"
#include "BloodImpact.generated.h"

/**
 * 
 */
UCLASS()
class HORDETEMPLATEV2NATIVE_API ABloodImpact : public ABaseImpact
{
	GENERATED_BODY()
	
public:

	ABloodImpact();
};
