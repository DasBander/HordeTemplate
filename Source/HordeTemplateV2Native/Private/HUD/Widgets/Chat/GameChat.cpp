// Copyright (c) 2018 - 2026 Marc Fraedrich
// Horde Template V2 (Native) - A cooperative zombie survival game framework
// Licensed under the MIT License

#include "GameChat.h"
#include "Gameplay/HordePlayerState.h"

/**
 * @file GameChat.cpp
 * @brief Implementation of in-game chat functionality for message sending and receiving
 * @author Marc Fraedrich
 */

/** ( Virtual; Overridden )
 * Binds the delegates for the Message Received Function and the On Focus Game Chat Function.
 *
 * @param
 * @return void
 */
void UGameChat::NativeConstruct()
{
	Super::NativeConstruct();

	AHordeBaseController* PC = Cast<AHordeBaseController>(GetOwningPlayer());
	if (PC)
	{
		PC->OnMessageReceivedDelegate.AddDynamic(this, &UGameChat::OnMessageReceived);
		PC->OnFocusGameChat.AddDynamic(this, &UGameChat::OnGameFocusChat);
	}
}

/**
 * Submits a chat message with given text to the server.
 *
 * @param The Text Message.
 * @return void
 */
void UGameChat::SubmitChatMessage(const FText& Message)
{
	AHordeBaseController* PC = Cast<AHordeBaseController>(GetOwningPlayer());
	if (PC)
	{
		AHordePlayerState* PS = Cast<AHordePlayerState>(PC->PlayerState);
		if (PS)
		{
			PS->SubmitMessage(Message);
			PC->CloseChat();
		}
	}
}

/** ( Virtual; Overridden )
 *	Getting called if widget gets destroyed.
 *
 * @param
 * @return void
 */
void UGameChat::NativeDestruct()
{
	Super::NativeDestruct();
}
