// Copyright (c) 2018 - 2026 Marc Fraedrich
// Horde Template V2 (Native) - A cooperative zombie survival game framework
// Licensed under the MIT License

#include "AICorePoint.h"
#include "HordeTemplateV2Native.h"
#include "UObject/ConstructorHelpers.h"

/**
 * @file AICorePoint.cpp
 * @brief Implementation of the AI patrol waypoint actor
 * @author Marc Fraedrich
 */

/**
 *	Constructor
 *
 * @param
 * @return
 */
AAICorePoint::AAICorePoint()
{
	PrimaryActorTick.bStartWithTickEnabled = false;
	PrimaryActorTick.bCanEverTick = false;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Scene Root"));
	Icon = CreateDefaultSubobject<UBillboardComponent>(TEXT("Icon"));
	Icon->SetupAttachment(RootComponent);
	const ConstructorHelpers::FObjectFinder<UTexture2D> IconAsset(TEXT("Texture2D'/Engine/EditorMaterials/TargetIcon.TargetIcon'"));
	if (IconAsset.Succeeded())
	{
		Icon->SetSprite(IconAsset.Object);
	}
}




