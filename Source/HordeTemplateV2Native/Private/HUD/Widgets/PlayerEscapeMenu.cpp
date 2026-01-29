// Copyright (c) 2018 - 2026 Marc Fraedrich
// Horde Template V2 (Native) - A cooperative zombie survival game framework
// Licensed under the MIT License

#include "PlayerEscapeMenu.h"
#include "Gameplay/HordeBaseController.h"
#include "HUD/HordeBaseHUD.h"

/**
 * @file PlayerEscapeMenu.cpp
 * @brief Implementation of pause menu with disconnect and close functionality
 * @author Marc Fraedrich
 */

/**
 * Disconnects owning player from server.
 *
 * @param
 * @return void
 */
void UPlayerEscapeMenu::DisconnectFromServer()
{
	AHordeBaseController* PC = Cast<AHordeBaseController>(GetOwningPlayer());
	if (PC)
	{
		PC->DisconnectFromServer();
	}
}

/**
 * Closes Escape Menu.
 *
 * @param
 * @return void
 */
void UPlayerEscapeMenu::CloseEscapeMenu()
{
	AHordeBaseHUD* HUD = Cast<AHordeBaseHUD>(GetOwningPlayer()->GetHUD());
	if (HUD)
	{
		HUD->CloseEscapeMenu();
	}
}
