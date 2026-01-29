// Copyright (c) 2018 - 2026 Marc Fraedrich
// Horde Template V2 (Native) - A cooperative zombie survival game framework
// Licensed under the MIT License

#include "LobbyChat.h"
#include "Gameplay/HordePlayerState.h"

/**
 * @file LobbyChat.cpp
 * @brief Implementation of lobby chat functionality for pre-game communication
 * @author Marc Fraedrich
 */

/** ( Virtual; Overridden )
 * Binds delegate for On Message Received Function.
 *
 * @param
 * @return void
 */
void ULobbyChat::NativeConstruct()
{
	Super::NativeConstruct();

	AHordeBaseController* PC = Cast<AHordeBaseController>(GetOwningPlayer());
	if (PC)
	{
		PC->OnLobbyMessageReceivedDelegate.AddDynamic(this, &ULobbyChat::OnMessageReceived);
	}
}

/**
 * Submits a chat message with given text to the server.
 *
 * @param The Text Message.
 * @return void
 */
void ULobbyChat::SubmitChatMessage(const FText& Message)
{
	AHordeBaseController* PC = Cast<AHordeBaseController>(GetOwningPlayer());
	if (PC)
	{
		AHordePlayerState* PS = Cast<AHordePlayerState>(PC->PlayerState);
		if (PS)
		{
			PS->SubmitMessage(Message);
		}
	}
}

/** ( Virtual; Overridden )
 *	Getting called if widget gets destroyed.
 *
 * @param
 * @return void
 */
void ULobbyChat::NativeDestruct()
{
	Super::NativeDestruct();
}
