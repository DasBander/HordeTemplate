// Copyright (c) 2018 - 2026 Marc Fraedrich
// Horde Template V2 (Native) - A cooperative zombie survival game framework
// Licensed under the MIT License

#include "HordePlayerState.h"
#include "HordeGameState.h"
#include "TimerManager.h"
#include "HUD/HordeBaseHUD.h"
#include "Gameplay/HordeBaseController.h"
#include "Character/HordeBaseCharacter.h"
#include "Gameplay/HordeGameSession.h"
#include "GameFramework/GameMode.h"
#include "Runtime/Core/Public/Math/NumericLimits.h"
#include "Misc/HordeTrader.h"
#include "HUD/HordeBaseHUD.h"

/**
 * @file HordePlayerState.cpp
 * @brief Implementation of player state with scoring, economy, character trading, and chat handling
 * @author Marc Fraedrich
 */

/** ( Overridden )
 *	Defines Replicated Props.
 *
 * @param TArray - FLifetimeProperty ( Out Lifetime Props ) const
 * @return void
 */
void AHordePlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AHordePlayerState, Player);
	DOREPLIFETIME(AHordePlayerState, ZedKills);
	DOREPLIFETIME(AHordePlayerState, Points);
	DOREPLIFETIME(AHordePlayerState, HeadShots);
	DOREPLIFETIME(AHordePlayerState, PlayerMoney);
	DOREPLIFETIME(AHordePlayerState, bIsDead);
}

/** ( Client )
 *	Updates the Game Status on the client side HUD Class.
 *
 * @param The current game status.
 * @return void
 */
void AHordePlayerState::ClientUpdateGameStatus_Implementation(EGameStatus GameStatus)
{
	APlayerController* PC = Cast<APlayerController>(GetOwner());
	if (PC)
	{
		AHordeBaseHUD* HUD = Cast<AHordeBaseHUD>(PC->GetHUD());
		if (HUD)
		{
			HUD->OnGameStatusChanged.Broadcast((uint8)GameStatus);
		}
	}
}

/** ( Client )
 *	Gets called when client receives a message.
 *
 * @param The Message that the client receives.
 * @return void
 */
void AHordePlayerState::OnMessageReceived_Implementation(FHordeChatMessage Msg)
{
	AHordeGameState* GS = Cast<AHordeGameState>(GetWorld()->GetGameState());
	AHordeBaseController* PC = Cast<AHordeBaseController>(GetOwner());
	if (GS && PC)
	{
		switch (GS->GameStatus) {
		case EGameStatus::ELOBBY:
			PC->OnLobbyMessageReceivedDelegate.Broadcast(Msg);
			break;

		case EGameStatus::EINGAME:
			PC->OnMessageReceivedDelegate.Broadcast(Msg);
			break;
		}
	}
}

/** ( Client )
 * Updates with given Player List the Player List inside HUD.
 *
 * @param Player List
 * @return void
 */
void AHordePlayerState::UpdateLobbyPlayerList_Implementation(const TArray<FPlayerInfo>& Players)
{
	APlayerController* PC = Cast<APlayerController>(GetOwner());
	if (PC)
	{
		AHordeBaseHUD* HUD = Cast<AHordeBaseHUD>(PC->GetHUD());
		if (HUD)
		{
			// Fixed: Added null check for LobbyWidget before accessing delegate
			UPlayerLobbyWidget* LobbyWidget = HUD->GetLobbyWidget();
			if (LobbyWidget)
			{
				LobbyWidget->OnLobbyPlayersUpdateDelegate.Broadcast(Players);
			}
		}
	}
}


/** ( Server )
 * Toggles the Ready Status on the Server.
 *
 * @param
 * @return void
 */
void AHordePlayerState::ToggleReadyStatus_Implementation()
{
	Player.PlayerReady = (Player.PlayerReady) ? false : true;
	AHordeGameState* GS = Cast<AHordeGameState>(GetWorld()->GetGameState());
	if (GS)
	{
		GS->UpdatePlayerLobby();
	}
}

bool AHordePlayerState::ToggleReadyStatus_Validate()
{
	return true;
}

/** ( Server )
 * Request Character Trade with Instigator Player ID and Target Player ID.
 *
 * @param The Instigator of the Trade and the Target Player ID.
 * @return void
 */
void AHordePlayerState::RequestCharacterTrade_Implementation(const FString& InstigatorPlayer, const FString& TargetPlayer)
{
	AHordeGameState* GS = Cast<AHordeGameState>(GetWorld()->GetGameState());
	if (GS)
	{
		GS->StartCharacterTrade(InstigatorPlayer, TargetPlayer);
	}
}

bool AHordePlayerState::RequestCharacterTrade_Validate(const FString& InstigatorPlayer, const FString& TargetPlayer)
{
	return true;
}

/** ( Server )
 * Cancels the current Character Trade if Target Player ID is own Player ID.
 *
 * @param
 * @return void
 */
void AHordePlayerState::CancelCharacterTrade_Implementation()
{
	AHordeGameState* GS = Cast<AHordeGameState>(GetWorld()->GetGameState());
	if (GS)
	{
		if (GS->TradeProgress.Target == Player.PlayerID)
		{
			GS->AbortLobbyTrade();
		}
	}
}

bool AHordePlayerState::CancelCharacterTrade_Validate()
{
	return true;
}

/** ( Server )
 * Client Accepts Character Trade if Characters ID is the Target Player ID.
 *
 * @param
 * @return void
 */
void AHordePlayerState::AcceptCharacterTrade_Implementation()
{
	AHordeGameState* GS = Cast<AHordeGameState>(GetWorld()->GetGameState());
	if (GS)
	{
		if (GS->TradeProgress.Target == Player.PlayerID)
		{
			GS->AcceptCharacterTrade();
		}
	}
}

bool AHordePlayerState::AcceptCharacterTrade_Validate()
{
	return true;
}

/** ( Client )
 * Function that is getting called when getting kicked. Will End Session and disconnects from the current game.
 *
 * @param
 * @return void
 */
void AHordePlayerState::GettingKicked_Implementation()
{
	APlayerController* PC = Cast<APlayerController>(GetOwner());
	if (PC)
	{
		if (GetWorld())
		{
			AGameMode* GM = Cast<AGameMode>(GetWorld()->GetAuthGameMode());
			if (GM)
			{
				AHordeGameSession* GSS = Cast<AHordeGameSession>(GetWorld()->GetAuthGameMode()->GameSession);
				if (GSS)
				{
					UE_LOG(LogTemp, Warning, TEXT("Ending Session."));
					GSS->EndGameSession();
				}
				else
				{
					UE_LOG(LogTemp, Warning, TEXT("Unable to get Game Session."));
				}
			}

			PC->ConsoleCommand("disconnect?message=disconnectedfromgame", false);
		}
	}
}

/** ( Server )
 * Submits a chat message to the server or  ( WIP ) parses an command.
 *
 * @param The Message that should be sent.
 * @return void
 */
void AHordePlayerState::SubmitMessage_Implementation(const FText& Message)
{
	AHordeGameState* GS = Cast<AHordeGameState>(GetWorld()->GetGameState());
	if(GS && Message.ToString() != "")
	{
		if (Message.ToString()[0] == *"/")
		{
			GS->ParseChatCommand(Player.PlayerID, Message.ToString());
		}
		else {
			GS->PopMessage(FHordeChatMessage(Player.UserName, Message));
		}

	}
}

bool AHordePlayerState::SubmitMessage_Validate(const FText& Message)
{
	return true;
}

/** ( Server )
 * Requests the server to kick the given Player by ID.
 *
 * @param The Player Info that should be kicked.
 * @return void
 */
void AHordePlayerState::RequestPlayerKick_Implementation(FPlayerInfo InPlayer)
{
	AHordeGameState* GS = Cast<AHordeGameState>(GetWorld()->GetGameState());
	if (GS)
	{
		if (GS->LobbyInformation.OwnerID == Player.PlayerID)
		{
			GS->KickPlayer(InPlayer.PlayerID);
		}
	}
}

bool AHordePlayerState::RequestPlayerKick_Validate(FPlayerInfo InPlayer)
{
	return true;
}

/** ( Server )
 *	Modify the Player Money on the Server.
 *
 * @param Money to Add / Remove.
 * @return void
 */
void AHordePlayerState::ModifyMoney_Implementation(int32 Modifier)
{
	PlayerMoney = FMath::Clamp<int32>(PlayerMoney + Modifier, 0, MAX_int32);
}

bool AHordePlayerState::ModifyMoney_Validate(int32 Modifier)
{
	return true;
}

/**
 * Returns the Player Money.
 *
 * @param
 * @return Player Money.
 */
int32 AHordePlayerState::GetPlayerMoney()
{
	return PlayerMoney;
}

/** ( Server )
 * Buys an Item, removes money from player if he can afford the item and gives him the item.
 *
 * @param Trader Item ID
 * @return void
 */
void AHordePlayerState::BuyItem_Implementation(FName SellItemID)
{
	UDataTable* TraderSellData = Cast<UDataTable>(StaticLoadObject(UDataTable::StaticClass(), nullptr, ECONOMY_DATATABLE_PATH));

	if (TraderSellData) {
		FTraderSellItem* ItemFromRow = TraderSellData->FindRow<FTraderSellItem>(SellItemID, "PlayerState - Buy Item", false);
		// Fixed: Was comparing PlayerMoney >= PlayerMoney - ItemPrice (always true). Should check if player can afford item.
		if (ItemFromRow && PlayerMoney >= ItemFromRow->ItemPrice)
		{
			PlayerMoney = FMath::Clamp<int32>(PlayerMoney - ItemFromRow->ItemPrice, 0, MAX_int32);
			AHordeBaseController* PC = Cast<AHordeBaseController>(GetOwner());
			if (PC)
			{
				AHordeBaseCharacter* PLY = Cast<AHordeBaseCharacter>(PC->GetPawn());
				if (PLY)
				{
					PLY->Inventory->ServerAddItem(ItemFromRow->ItemID.ToString(), false, FItem());
				}
			}
		}
	}

}

bool AHordePlayerState::BuyItem_Validate(FName SellItemID)
{
	return true;
}

/**
 * Adds Points to the Replicated Values.
 *
 * @param The Points and the the Point Type
 * @return void
 */
void AHordePlayerState::AddPoints(int32 InPoints, EPointType PointsType)
{
	switch (PointsType)
	{
	case  EPointType::EPointCasual:
		ZedKills++;
		PlayerMoney = PlayerMoney + 150;
	break;

	case EPointType::EPointHeadShot:
		ZedKills++;
		HeadShots++;
		PlayerMoney = PlayerMoney + 250;
	break;

	default:
	break;
	}
	Points = InPoints + Points;

	ClientNotifyPoints(PointsType, InPoints);
}

/** ( Client )
 * Triggers Delegate to Update the Points inside the HUD Class
 *
 * @param The Point Type and the Points
 * @return void
 */
void AHordePlayerState::ClientNotifyPoints_Implementation(EPointType PointType, int32 OutPoints)
{
	AHordeBaseController* PC = Cast<AHordeBaseController>(GetOwner());
	if (PC)
	{
		AHordeBaseHUD* HUD = Cast<AHordeBaseHUD>(PC->GetHUD());
		if (HUD)
		{
			HUD->OnPlayerPointsReceivedDelegate.Broadcast(PointType, OutPoints);
		}
	}
}

/** ( Virtual; Overridden )
 *	Updates Game Status takes a free player and updates lobby.
 *	Fixed: Now handles mid-game joins by spawning player as spectator.
 *
 * @param
 * @return void
 */
void AHordePlayerState::BeginPlay()
{
	Super::BeginPlay();

	FTimerHandle DelayedBeginPlay;
	FTimerDelegate DelayedBeginPlayDelegate;

	DelayedBeginPlayDelegate.BindLambda([this] {
		if (HasAuthority())
		{
			AHordeGameState* GS = Cast<AHordeGameState>(GetWorld()->GetGameState());
			if (GS)
			{
				ClientUpdateGameStatus(GS->GameStatus);
				Player.SelectedCharacter = GS->GetFreeCharacter();
				// Fixed: Added null check for GetUniqueId()
				Player.PlayerID = GetUniqueId().IsValid() ? GetUniqueId()->ToString() : TEXT("");
				Player.UserName = GetPlayerName();

				// Fixed: Handle mid-game joins - spawn as spectator if game is already in progress
				if (GS->GameStatus == EGameStatus::EINGAME)
				{
					// Player joined mid-game, spawn them as spectator
					APlayerController* PC = Cast<APlayerController>(GetOwner());
					if (PC)
					{
						AHordeGameMode* GM = Cast<AHordeGameMode>(GetWorld()->GetAuthGameMode());
						if (GM)
						{
							GM->SpawnSpectator(PC);
							bIsDead = true; // Mark as dead so they spectate
						}
					}
					GS->PopMessage(FHordeChatMessage("SERVER", FText::FromString(GetPlayerName() + " joined as spectator.")));
				}
				else if (GS->GameStatus == EGameStatus::ELOBBY)
				{
					GS->TakePlayer(Player);
					GS->UpdatePlayerLobby();
					GS->PopMessage(FHordeChatMessage("SERVER", FText::FromString(GetPlayerName() + " has joined.")));
				}
				else
				{
					// Game over or server travel - just notify
					GS->PopMessage(FHordeChatMessage("SERVER", FText::FromString(GetPlayerName() + " has joined.")));
				}
			}
		}
	});

	GetWorld()->GetTimerManager().SetTimer(DelayedBeginPlay, DelayedBeginPlayDelegate, 2.f, false);
}
