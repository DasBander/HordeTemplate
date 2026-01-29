// Copyright (c) 2018 - 2026 Marc Fraedrich
// Horde Template V2 (Native) - A cooperative zombie survival game framework
// Licensed under the MIT License

#include "PlayerScoreboardItem.h"
#include "Gameplay/HordePlayerState.h"

/**
 * @file PlayerScoreboardItem.cpp
 * @brief Implementation of individual scoreboard entry with player stats and status
 * @author Marc Fraedrich
 */

/**
 * Returns color depending on if the character is dead or not.
 *
 * @param
 * @return Color if Character is Dead or Not.
 */
FLinearColor UPlayerScoreboardItem::GetDeadBorderColor()
{
	FLinearColor RetColor = FLinearColor();
	AHordePlayerState* PS = Cast<AHordePlayerState>(PlayerState);
	if (PS)
	{
		
		if (PS->bIsDead)
		{
			RetColor = FLinearColor(0.229167f, 0.f, 0.f, 0.8f);
		}
		else {
			RetColor = FLinearColor(0.010417f, 0.010417f, 0.010417f, 0.8f);
		}
	}
	return RetColor;
}

/**
 * Returns the Players Ping as text with Ms.
 *
 * @param
 * @return Players ping with Ms ( 10Ms )
 */
FText UPlayerScoreboardItem::GetPlayerPing()
{
	FString RetPing = "/";
	AHordePlayerState* PS = Cast<AHordePlayerState>(PlayerState);
	if (PS)
	{
		RetPing = FString::FromInt(PS->GetPingInMilliseconds()) + " Ms";
	}
	return FText::FromString(RetPing);
}

/**
 * Returns the Players Score as Text.
 *
 * @param
 * @return Players Score as Text.
 */
FText UPlayerScoreboardItem::GetPlayerScore()
{
	FString RetScore = "/";
	AHordePlayerState* PS = Cast<AHordePlayerState>(PlayerState);
	if (PS)
	{
		RetScore = FString::FromInt(PS->Points) + " Points";
	}
	return FText::FromString(RetScore);
}

/**
 * Returns the Players Username as Text.
 *
 * @param
 * @return Players Username as Text.
 */
FText UPlayerScoreboardItem::GetPlayerName()
{
	FString RetUsername = "Unknown";
	AHordePlayerState* PS = Cast<AHordePlayerState>(PlayerState);
	if (PS)
	{
		RetUsername = PS->GetPlayerInfo().UserName;
	}
	return FText::FromString(RetUsername);
}
