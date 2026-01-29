// Copyright (c) 2018 - 2026 Marc Fraedrich
// Horde Template V2 (Native) - A cooperative zombie survival game framework
// Licensed under the MIT License

#include "PlayerKickMenu.h"
#include "Gameplay/HordePlayerState.h"

/**
 * @file PlayerKickMenu.cpp
 * @brief Implementation of player kick functionality for lobby admins
 * @author Marc Fraedrich
 */

/**
 * Kicks the Player with Owning Player Info.
 *
 * @param
 * @return void
 */
void UPlayerKickMenu::KickPlayer()
{
	AHordePlayerState* PS = Cast<AHordePlayerState>(GetOwningPlayer()->PlayerState);
	if (PS)
	{
		PS->RequestPlayerKick(PlyInfo);
	}
}
