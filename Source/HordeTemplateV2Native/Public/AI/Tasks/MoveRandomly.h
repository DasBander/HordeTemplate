// Copyright (c) 2018 - 2026 Marc Fraedrich
// Horde Template V2 (Native) - A cooperative zombie survival game framework
// Licensed under the MIT License

#pragma once

/**
 * @file MoveRandomly.h
 * @brief Behavior tree task for random wandering movement during patrol state
 * @author Marc Fraedrich
 */

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "MoveRandomly.generated.h"

/**
 * 
 */
UCLASS()
class HORDETEMPLATEV2NATIVE_API UMoveRandomly : public UBTTaskNode
{
	GENERATED_BODY()
	
public:

	UMoveRandomly();


	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

};
