// Copyright (c) 2018 - 2026 Marc Fraedrich
// Horde Template V2 (Native) - A cooperative zombie survival game framework
// Licensed under the MIT License

#include "PlayerTravelWidget.h"
#include "Gameplay/HordeGameState.h"
#include "Kismet/GameplayStatics.h"

/**
 * @file PlayerTravelWidget.cpp
 * @brief Implementation of server travel loading screen with map info display
 * @author Marc Fraedrich
 */

/**
 * Returns the current status of map loading as text.
 *
 * @param
 * @return current status of map loading as text.
 */
FText UPlayerTravelWidget::GetServerInfo()
{
	FString RetInfo = "Getting Serverinfo.....";
	AHordeGameState* GS = Cast<AHordeGameState>(GetWorld()->GetGameState());
	if (GS)
	{
		FString MapName = "None";
		if (GS->NextLevel != NAME_None)
		{
			MapName = GS->NextLevel.ToString();
		}
		else {
			MapName = UGameplayStatics::GetCurrentLevelName(GetWorld(), true);
		}
		RetInfo = "Loading " + MapName;
	}
	return FText::FromString(RetInfo);
}
