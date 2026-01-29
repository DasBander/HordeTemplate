// Copyright (c) 2018 - 2026 Marc Fraedrich
// Horde Template V2 (Native) - A cooperative zombie survival game framework
// Licensed under the MIT License

#pragma once

/**
 * @file AICorePoint.h
 * @brief Actor that serves as a patrol waypoint for AI navigation
 * @author Marc Fraedrich
 */

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BillboardComponent.h"
#include "AICorePoint.generated.h"

UCLASS()
class HORDETEMPLATEV2NATIVE_API AAICorePoint : public AActor
{
	GENERATED_BODY()
	
public:	

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Target")
		FName PatrolTag = NAME_None;


	AAICorePoint();

protected:

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Components")
		class UBillboardComponent* Icon;

public:	


};
