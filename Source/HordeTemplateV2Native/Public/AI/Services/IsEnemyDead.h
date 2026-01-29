// Copyright (c) 2018 - 2026 Marc Fraedrich
// Horde Template V2 (Native) - A cooperative zombie survival game framework
// Licensed under the MIT License

#pragma once

/**
 * @file IsEnemyDead.h
 * @brief Behavior tree service that monitors enemy status and manages AI state transitions
 * @author Marc Fraedrich
 */

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "IsEnemyDead.generated.h"

/**
 * 
 */
UCLASS()
class HORDETEMPLATEV2NATIVE_API UIsEnemyDead : public UBTService
{
	GENERATED_BODY()
public:

	UIsEnemyDead();

	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;


};
