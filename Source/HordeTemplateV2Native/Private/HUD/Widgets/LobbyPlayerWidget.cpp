// Copyright (c) 2018 - 2026 Marc Fraedrich
// Horde Template V2 (Native) - A cooperative zombie survival game framework
// Licensed under the MIT License

#include "LobbyPlayerWidget.h"
#include "Gameplay/HordePlayerState.h"
#include "Gameplay/HordeGameState.h"

/**
 * @file LobbyPlayerWidget.cpp
 * @brief Implementation of lobby player widget showing ready state and trade options
 * @author Marc Fraedrich
 */

/**
 * Returns Ready Color if Player is Ready or not.
 *
 * @param
 * @return Color that should be applied to the character object widget.
 */
FLinearColor ULobbyPlayerWidget::GetReadyColor()
{
	if (PlayerInfo.PlayerReady)
	{
		return FLinearColor(0.003907f, 0.628472f, 0.f, 0.8f);
	}
	else
	{
		return FLinearColor(0.f, 0.f, 0.f, 0.8);
	}
}

/**
 * Returns visible if selected character id of owning player is != None.
 *
 * @param
 * @return Returns visiblity based on if owning player character id != None.
 */
ESlateVisibility ULobbyPlayerWidget::GetCharacterAvailableVisibility()
{
	if (PlayerInfo.SelectedCharacter != NAME_None)
	{
		return ESlateVisibility::Visible;
	}
	else
	{
		return ESlateVisibility::Hidden;
	}
}

/**
 * Returns Visibility based on if Player is involved in the Character Trading Process.
 *
 * @param
 * @return Visible if Character is involved in Trader. Hidden if not.
 */
ESlateVisibility ULobbyPlayerWidget::GetTradingAvailableVisibility()
{
	AHordePlayerState* PS = Cast<AHordePlayerState>(GetOwningPlayer()->PlayerState);
	if (PS)
	{
		AHordeGameState* GS = Cast<AHordeGameState>(GetWorld()->GetGameState());
		if (GS)
		{
			if (PlayerInfo.PlayerReady || PlayerInfo.PlayerID == PS->GetPlayerInfo().PlayerID || GS->IsTradeInProgress || PS->GetPlayerInfo().PlayerReady)
			{
				return ESlateVisibility::Hidden;
			}
			else {
				return ESlateVisibility::Visible;
			}
		}
		else {
			return ESlateVisibility::Hidden;
		}
	}
	else
	{
		return ESlateVisibility::Hidden;
	}
}

/**
 * Initiates Trade with given Player.
 *
 * @param
 * @return void
 */
void ULobbyPlayerWidget::InitiateTrade()
{
	AHordePlayerState* PS = Cast<AHordePlayerState>(GetOwningPlayer()->PlayerState);
	if (PS)
	{
		PS->RequestCharacterTrade(PS->GetPlayerInfo().PlayerID, PlayerInfo.PlayerID);
	}
}

/**
 * Checks if Player is owner of current session.
 *
 * @param
 * @return bool if player is admin or not.
 */
bool ULobbyPlayerWidget::IsAdmin()
{
	bool RetIs = false;
	AHordePlayerState* PS = Cast<AHordePlayerState>(GetOwningPlayer()->PlayerState);
	if (PS)
	{
		AHordeGameState* GS = Cast<AHordeGameState>(GetWorld()->GetGameState());
		if (GS)
		{
			if (PS->GetPlayerInfo().PlayerID == GS->LobbyInformation.OwnerID)
			{
				 RetIs = true;
			}
		}
	}
	return RetIs;
}


/**
 * Checks if Player is Owner of the Current Widget
 *
 * @param
 * @return bool if it's your own Player Widget.
 */
bool ULobbyPlayerWidget::IsOwner()
{
	bool RetIs = false;
	AHordePlayerState* PS = Cast<AHordePlayerState>(GetOwningPlayer()->PlayerState);
	if (PS)
	{
		AHordeGameState* GS = Cast<AHordeGameState>(GetWorld()->GetGameState());
		if (GS)
		{
			if (PS->GetPlayerInfo().PlayerID == PlayerInfo.PlayerID)
			{
				RetIs = true;
			}
		}
	}
	return RetIs;
}

/** ( Virtual; Overridden )
 * Gets the character from the Character Datatable.
 *
 * @param
 * @return void
 */
void ULobbyPlayerWidget::NativeConstruct()
{
	Super::NativeConstruct();

	Character = FromDatatable<FPlayableCharacter>(CHARACTER_DATATABLE_PATH, PlayerInfo.SelectedCharacter);
}
