// Copyright (c) 2018 - 2026 Marc Fraedrich
// Horde Template V2 (Native) - A cooperative zombie survival game framework
// Licensed under the MIT License

#include "HordeGameSession.h"
#include "OnlineSubsystem.h"

/**
 * @file HordeGameSession.cpp
 * @brief Implementation of game session for online subsystem integration and session cleanup
 * @author Marc Fraedrich
 */

/**
 *	Ends the current game session in the Online Subsystem.
 *	
 * @param
 * @return void
 */
void AHordeGameSession::EndGameSession()
{
	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
	if (OnlineSub)
	{
		IOnlineSessionPtr Session = OnlineSub->GetSessionInterface();
		if (Session.IsValid())
		{
			if (Session->EndSession(NAME_GameSession))
			{
				Session->DestroySession(NAME_GameSession);
			}
		}
	}
}
