

#include "HordeGameState.h"
#include "HordeTemplateV2Native.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Gameplay/HordeBaseController.h"
#include "Character/HordeBaseCharacter.h"
#include "HordeGameInstance.h"
#include "AI/ZedPawn.h"
#include "Runtime/Engine/Public/EngineUtils.h"
#include "Misc/HordeTrader.h"

/** ( Overridden )
 *	Definition of Replicated Props
 *
 * @param Array FLifetimeProperty
 * @return void
 */
void AHordeGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AHordeGameState, LobbyInformation);
	DOREPLIFETIME(AHordeGameState, GameStatus);
	DOREPLIFETIME(AHordeGameState, GameStarting);
	DOREPLIFETIME(AHordeGameState, LobbyTime);
	DOREPLIFETIME(AHordeGameState, BlockDisconnect);
	DOREPLIFETIME(AHordeGameState, TradeProgress);
	DOREPLIFETIME(AHordeGameState, IsTradeInProgress);
	DOREPLIFETIME(AHordeGameState, RoundTime);
	DOREPLIFETIME(AHordeGameState, IsRoundPaused);
	DOREPLIFETIME(AHordeGameState, GameRound);
	DOREPLIFETIME(AHordeGameState, ZedsLeft);
	DOREPLIFETIME(AHordeGameState, Score_MVP);
	DOREPLIFETIME(AHordeGameState, Score_MostHeadshots);
	DOREPLIFETIME(AHordeGameState, Score_MostKills);
	DOREPLIFETIME(AHordeGameState, NextLevel);
	DOREPLIFETIME(AHordeGameState, EndTime);
	DOREPLIFETIME(AHordeGameState, PauseTime);
	DOREPLIFETIME(AHordeGameState, MatchMode);
}

/** ( Virtual; Overridden )
 *	Initializes Lobby and Game Settings. Also sets the Lobby Owner.
 *
 * @param
 * @return void
 */
void AHordeGameState::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		GameStatus = EGameStatus::ELOBBY;
		
		AHordeWorldSettings* WorldSettings = Cast<AHordeWorldSettings>(GetWorld()->GetWorldSettings());
		if (WorldSettings)
		{
			MatchMode = WorldSettings->MatchMode;
			for (auto &Char : WorldSettings->PlayerCharacters)
			{
				FLobbyAvailableCharacters AChar;
				AChar.CharacterID = Char;
				LobbyInformation.AvailableCharacters.Add(AChar);
			}

			if (UKismetSystemLibrary::IsDedicatedServer(GetWorld()) && !PlayerArray.IsValidIndex(0))
			{
				UDataTable* DTMaps = Cast<UDataTable>(StaticLoadObject(UDataTable::StaticClass(), nullptr, MAPS_DATATABLE_PATH));
				TArray<FPlayableLevel*> PlayableLevels;
				if (DTMaps) {
					DTMaps->GetAllRows<FPlayableLevel>(TEXT("Game State Init"), PlayableLevels);
				}
				for (auto PLevel : PlayableLevels)
				{
					LobbyInformation.LobbyMapRotation.Add(PLevel->RawLevelName);
				}	
				LobbyInformation.OwnerID = "Dedicated Server";
				LobbyInformation.LobbyName = "Horde Game - Lobby";
			}
			else
			{
				UHordeGameInstance* GI = Cast<UHordeGameInstance>(GetGameInstance());
				if (GI)
				{
					APlayerController* PC = Cast<APlayerController>(GI->GetPrimaryPlayerController());
					if (PC)
					{
						// Fixed: Added null check for GetUniqueId()
					if (PC->PlayerState && PC->PlayerState->GetUniqueId().IsValid())
					{
						LobbyInformation.OwnerID = PC->PlayerState->GetUniqueId()->ToString();
					}
					}
					LobbyInformation.LobbyName = GI->LobbyName;
					LobbyInformation.LobbyMapRotation = GI->MapRotation;
				}
			}
		}
		
		LobbyTime = LobbyInformation.DefaultLobbyTime;
	}
}

/**
 *	@WIP - Parse Chat Commands and should run the Command
 *
 * @param Player that Executes Command and the Command
 * @return
 */
void AHordeGameState::ParseChatCommand(FString PlayerID, FString Command)
{
	FString ParsedCommand = Command;
	ParsedCommand.RemoveAt(0);
	TArray<FString> ParsedArray;
	ParsedCommand.ParseIntoArray(ParsedArray, TEXT(" "), true);
	if (ParsedArray.IsValidIndex(0))
	{
		if (ParsedArray[0] == "help")
		{
			MessagePlayer(FHordeChatMessage("List of Commands\n/votekick {PlayerName}- Votekick a Player\n/voteban {PlayerName} - Voteban a Player\n/kick {PlayerName} - Kicks a Player\n/ban {PlayerName} - Bans a Player\n/changelevel {levelname} - Changes Level of Server\n/votenextmap - Vote for Next Level in Map Rotation \n/endgame - Force Reset Map"), PlayerID);
		}
		else {
			MessagePlayer(FHordeChatMessage("Unknown Command"), PlayerID);
		}
	}
	else {
		MessagePlayer(FHordeChatMessage("Unknown Command"), PlayerID);
	}
}

/**
 * Takes Player Character in Lobby and Updates the Player Lobby List
 *
 * @param FPlayerInfo of Player with selected Character
 * @return void
 */
void AHordeGameState::TakePlayer(FPlayerInfo Player)
{
	if (!IsCharacterTaken(Player.SelectedCharacter))
	{
		TArray<FLobbyAvailableCharacters> Characters = LobbyInformation.AvailableCharacters;
		int32 CharacterIndex = -1;
		for (auto PChar : Characters)
		{
			CharacterIndex++;
			if (PChar.CharacterID == Player.SelectedCharacter)
			{
				break;
			}
		}
		if (Characters.IsValidIndex(CharacterIndex))
		{
			Characters[CharacterIndex].CharacterTaken = true;
			Characters[CharacterIndex].PlayerID = Player.PlayerID;
			Characters[CharacterIndex].PlayerUsername = Player.UserName;
			LobbyInformation.AvailableCharacters = Characters;
			UpdatePlayerLobby();
		}
	}
}

/**
 *	Kicks player by SteamID or PlayerID Depending on Online Subsystem
 *
 * @param Player ID to Kick
 * @return void
 */
void AHordeGameState::KickPlayer(const FString& PlayerID)
{
	AHordePlayerState* PS = GetPlayerStateByID(PlayerID);
	if (PS)
	{
		PopMessage(FHordeChatMessage(PS->GetPlayerInfo().UserName + " got kicked."));
		PS->GettingKicked();
	}
}

/**
 *	Gets World Settings Object from World.
 *
 * @param
 * @return Returns World Settings Object.
 */
AHordeWorldSettings* AHordeGameState::GetHordeWorldSettings()
{
	return Cast<AHordeWorldSettings>(GetWorld()->GetWorldSettings());
}

/**
 *	Starts RoundBase Game Timer ( Linear )
 *
 * @param
 * @return void
 */
void AHordeGameState::StartRoundBasedGame()
{
	if (!GetWorld()->GetTimerManager().IsTimerActive(PauseTimer))
	{
		GetWorld()->GetTimerManager().SetTimer(PauseTimer, this, &AHordeGameState::ProcessPauseTime, 1.f, true);
		IsRoundPaused = true;
		RoundTime = 0.f;
	}
}

/**
 *	Processes Pause Time ( Linear ) and initiates new Game Round.
 *
 * @param
 * @return void
 */
void AHordeGameState::ProcessPauseTime()
{
	AHordeWorldSettings* WS = Cast<AHordeWorldSettings>(GetWorld()->GetWorldSettings(false, true));
	if (WS)
	{
		if (PauseTime >= WS->PauseTime)
		{
			PauseTime = 0.f;
			GameRound++;
			GetWorld()->GetTimerManager().ClearTimer(PauseTimer);
			IsRoundPaused = false;

			for (TActorIterator<AHordeBaseController> ActorItr(GetWorld()); ActorItr; ++ActorItr)
			{
				AHordeBaseController* CTRL = *ActorItr;
				if (CTRL)
				{
					USoundCue* NewRoundSound = ObjectFromPath<USoundCue>(TEXT("SoundCue'/Game/HordeTemplateBP/Assets/Sounds/A_RoundWarmup_Cue.A_RoundWarmup_Cue'"));
					if (NewRoundSound)
					{
						CTRL->ClientPlay2DSound(NewRoundSound);
					}
				}

			}

			StartGameRound();
		}
		else 
		{
			PauseTime++;
		}
	}
}

/**
 *	Starts new GameRound and initiates Zombie Spawning. Closes Trader UI as well.
 *
 * @param
 * @return void
 */
void AHordeGameState::StartGameRound()
{
	if (!GetWorld()->GetTimerManager().IsTimerActive(RoundTimer))
	{
		GetWorld()->GetTimerManager().SetTimer(RoundTimer, this, &AHordeGameState::ProcessRoundTime, 1.f, true);
	}

	AHordeGameMode* GMO = Cast<AHordeGameMode>(GetWorld()->GetAuthGameMode());
	if (GMO)
	{
		AHordeWorldSettings* HWS = Cast<AHordeWorldSettings>(GetWorld()->GetWorldSettings(false, true));
		if (HWS)
		{
			GMO->InitiateZombieSpawning(GameRound * HWS->ZedMultiplier);
		}
	}
	for (auto& PLY : PlayerArray)
	{
		AHordeBaseController* PC = Cast<AHordeBaseController>(PLY->GetOwner());
		if (PC)
		{
			PC->ClientCloseTraderUI();
		}
	}

}

/**
 *	Ends Game Round ( Linear ) if round time is over.
 *
 * @param
 * @return void
 */
void AHordeGameState::ProcessRoundTime()
{
	AHordeWorldSettings* HWS = Cast<AHordeWorldSettings>(GetWorld()->GetWorldSettings());
	if (HWS)
	{
		if (RoundTime >= HWS->RoundTime)
		{
			EndGameRound();
		}
		else 
		{
			RoundTime++;
		}
	}
}

/**
 *	Gets next level from Lobby Map Rotation
 *
 * @param Bool ResetLevel ( If it should return current LevelName )
 * @return FName ( Raw Level Name )
 */
FName AHordeGameState::GetNextLevelInRotation(bool ResetLevel)
{
	FName RNextLevel = *UGameplayStatics::GetCurrentLevelName(GetWorld(), true);
	if (!ResetLevel)
	{
		int32 CurLevelIndex = LobbyInformation.LobbyMapRotation.Find(RNextLevel);
		if (CurLevelIndex >= (LobbyInformation.LobbyMapRotation.Num() - 1))
		{
			if (LobbyInformation.LobbyMapRotation.IsValidIndex(0))
			{
				RNextLevel = LobbyInformation.LobbyMapRotation[0];
			}
		}
		else
		{
			if (LobbyInformation.LobbyMapRotation.IsValidIndex(CurLevelIndex + 1))
			{
				RNextLevel = LobbyInformation.LobbyMapRotation[CurLevelIndex + 1];
			}
		}
	}
	return RNextLevel;
}

/**
 *	Ends the Current In-Game Round ( Linear ) and Starts the Pause Timer
 *
 * @param
 * @return void
 */
void AHordeGameState::EndGameRound()
{
	if (GetWorld()->GetTimerManager().IsTimerActive(RoundTimer))
	{
		GetWorld()->GetTimerManager().ClearTimer(RoundTimer);
	}
	AHordeWorldSettings* HWS = Cast<AHordeWorldSettings>(GetWorld()->GetWorldSettings());
	if (HWS)
	{
		if (GameRound >= HWS->MaxRounds)
		{
			EndGame(ZedsLeft > 0);
		}
		else 
		{
			StartRoundBasedGame();
		}
	}
	
}

/**
 *	Starts the Lobby Timer
 *
 * @param
 * @return void
 */
void AHordeGameState::StartLobbyTimer()
{
	if (!GetWorld()->GetTimerManager().IsTimerActive(LobbyTimer))
	{
		GetWorld()->GetTimerManager().SetTimer(LobbyTimer, this, &AHordeGameState::ProcessLobbyTime, 1.f, true);
	}
}

/**
 *	Initiates Game Over and Calculates Score for the End Screen. Updates Game Status to Game Over and Kills all remaining Player Characters and Firearm. Also starts End Screen Timer.
 *
 * @param Next Level to be Played.
 * @return void
 */
void AHordeGameState::GameOver(FName NextMap)
{
	GameStatus = EGameStatus::EGAMEOVER;
	NextLevel = NextMap;
	CalcEndScore(Score_MVP, Score_MostHeadshots, Score_MostKills);
	for (auto& PS : PlayerArray)
	{
		AHordePlayerState* PLY = Cast<AHordePlayerState>(PS);
		if (PLY)
		{
			PLY->ClientUpdateGameStatus(GameStatus);
		}
	}

	for (TActorIterator<AHordeBaseCharacter> ActorItr(GetWorld()); ActorItr; ++ActorItr)
	{
		AHordeBaseCharacter* PLYChar = *ActorItr;
		if (*ActorItr && PLYChar)
		{
			ABaseFirearm* CharFirearm = PLYChar->GetCurrentFirearm();
			if (CharFirearm)
			{
				CharFirearm->SetLifeSpan(0.1f);
			}
			PLYChar->SetLifeSpan(0.1f);
		}
		else {
			UE_LOG(LogTemp, Warning, TEXT("Could not Kill Player Characters. Not Valid."));
		}
	}
	if (!GetWorld()->GetTimerManager().IsTimerActive(EndGameTimer))
	{
		GetWorld()->GetTimerManager().SetTimer(EndGameTimer, this, &AHordeGameState::ProcessEndTime, 1.f, true);
	}
}

/**
 *	Processes Lobby Time. Starts Countdown and Starts Game if time is Over.
 *
 * @param
 * @return void
 */
void AHordeGameState::ProcessLobbyTime()
{
	if (LobbyTime > 0.f)
	{
		LobbyTime--;
		if (FMath::RoundToInt(LobbyTime) <= 10 && FMath::RoundToInt(LobbyTime) > 0)
		{
			PopMessage(FHordeChatMessage("Game starting in " + FString::FromInt(FMath::RoundToInt(LobbyTime)) + " seconds."));
			if (FMath::RoundToInt(LobbyTime) <= 5)
			{
				BlockDisconnect = true;
			}
		}
		else {
			if (FMath::RoundToInt(LobbyTime) == 0)
			{
				PopMessage(FHordeChatMessage("Game starting...."));
			}
		}
	}
	else 
	{
		if (GetWorld()->GetTimerManager().IsTimerActive(LobbyTimer))
		{
			GetWorld()->GetTimerManager().ClearTimer(LobbyTimer);
		}
		StartGame();
	}

}

/**
 *	Resets Lobby Time and Stops Timer.
 *
 * @param
 * @return void
 */
void AHordeGameState::ResetLobbyTime()
{
	// Fixed: Only send "interrupted" message if the timer was actually active (game was starting)
	const bool bWasTimerActive = GetWorld()->GetTimerManager().IsTimerActive(LobbyTimer);
	if (bWasTimerActive)
	{
		GetWorld()->GetTimerManager().ClearTimer(LobbyTimer);
		PopMessage(FHordeChatMessage("Game Start was interrupted."));
	}
	LobbyTime = LobbyInformation.DefaultLobbyTime;
	BlockDisconnect = false;
}

/**
 *	Switches all Players Ready Status to false
 *
 * @param
 * @return void
 */
void AHordeGameState::UnreadyAllPlayers()
{
	for (auto& PS : PlayerArray)
	{
		AHordePlayerState* PlayerState = Cast<AHordePlayerState>(PS);
		if (PlayerState)
		{
			PlayerState->SwitchReady(false);
		}
	}
}

/**
 * Aborts Character Trade and Starts Round Base Game or Non Linear Game.
 *
 * @param
 * @return void
 */
void AHordeGameState::StartGame()
{
	if (GameStatus != EGameStatus::EINGAME)
	{
		AbortLobbyTrade();
		GameStatus = EGameStatus::EINGAME;
		for (auto& Ply : PlayerArray)
		{
			AHordePlayerState* PS = Cast<AHordePlayerState>(Ply);
			if (PS)
			{
				PS->ClientUpdateGameStatus(GameStatus);
			}
		}
		AHordeGameMode* HGM = Cast<AHordeGameMode>(GetWorld()->GetAuthGameMode());
		if (HGM)
		{
			HGM->GameStart(LobbyPlayers);
		}

		AHordeWorldSettings* WS = Cast<AHordeWorldSettings>(GetWorld()->GetWorldSettings(true, true));
		if (WS)
		{
			if (WS->MatchMode == EMatchMode::EMatchModeNonLinear)
			{
				StartRoundBasedGame();
			}
		}
	}
}

/**
 *	Returns if Character is Taken by a Player or not.
 *
 * @param The Character ID
 * @return bool ( Availability )
 */
bool AHordeGameState::IsCharacterTaken(FName CharacterID)
{
	bool RetTaken = false;
	for (auto Char : LobbyInformation.AvailableCharacters)
	{
		if (Char.CharacterID == CharacterID && Char.PlayerID != "")
		{
			RetTaken = true;
		}
	}
	return RetTaken;
}

/**
 *	Gets the Character by PlayerID.
 *
 * @param The PlayerID
 * @output int32 Character Index of LobbyInformation.AvailableCharacters ( Array )
 * @return The Character ID
 */
FName AHordeGameState::GetCharacterByID(FString PlayerID, int32 &CharacterIndex)
{
	int32 TempIndex = -1;
	FName RetTemp = TEXT("None");
	for (auto PChar : LobbyInformation.AvailableCharacters)
	{
		TempIndex++;
		if (PChar.PlayerID == PlayerID)
		{
			RetTemp = PChar.CharacterID;
			break;
		}
	}
	CharacterIndex = TempIndex;
	return RetTemp;
}

/**
 *	Returns PlayerState by given PlayerID; Returns nullptr if doesn't exist;
 *
 * @param The Player ID we want the Player State of.
 * @return PlayerState (Object) from given Player ID
 */
AHordePlayerState* AHordeGameState::GetPlayerStateByID(FString PlayerID)
{
	AHordePlayerState* RetPS = nullptr;
	for (auto& PLY : PlayerArray)
	{
		AHordePlayerState* PS = Cast<AHordePlayerState>(PLY);
		// Fixed: Added null check for GetUniqueId()
		if (PS && PS->GetUniqueId().IsValid())
		{
			if (PS->GetUniqueId()->ToString() == PlayerID)
			{
				RetPS = PS;
				break;
			}
		}
	}
	return RetPS;
}

/**
 *	Searches for Characters that are not assigned to any player and/or if a player disconnects the character gets unassigned and the player id gets cleared.
 *
 * @param
 * @return void
 */
void AHordeGameState::FreeupUnassignedCharacters()
{
	TArray<FLobbyAvailableCharacters> LocalCharacters = LobbyInformation.AvailableCharacters;
	TArray<int32> PlayersToFreeup;
	
	int32 LocalCharCurrentIndex = -1;
	for (auto LChar : LocalCharacters)
	{
		LocalCharCurrentIndex += 1;
		bool PlayerFound = false;
		GetUsernameBySteamID(LChar.PlayerID, PlayerFound);
		if (!PlayerFound)
		{
			PlayersToFreeup.AddUnique(LocalCharCurrentIndex);
		}
	}
	for (int32 PlyID : PlayersToFreeup)
	{
		if (LocalCharacters[PlyID].PlayerID != "" && LocalCharacters[PlyID].PlayerUsername != "")
		{
			//Display Disconnect Message.
			PopMessage(FHordeChatMessage(LocalCharacters[PlyID].PlayerUsername + " has disconnected"));

			//Abort Character Trade if Player was involved.
			// Fixed: Added parentheses for correct operator precedence (&&  has higher precedence than ||)
			if (IsTradeInProgress && (TradeProgress.Instigator == LocalCharacters[PlyID].PlayerID || TradeProgress.Target == LocalCharacters[PlyID].PlayerID))
			{
				AbortLobbyTrade();
			}

			//Reset Character and set it to not taken.
			LocalCharacters[PlyID].PlayerUsername = "";
			LocalCharacters[PlyID].PlayerID = "";
			LocalCharacters[PlyID].CharacterTaken = false;
		}
		else 
		{
			LocalCharacters[PlyID].PlayerUsername = "";
			LocalCharacters[PlyID].PlayerID = "";
			LocalCharacters[PlyID].CharacterTaken = false;
		}
	}
	//Update Lobby Information with new Character List.
	LobbyInformation.AvailableCharacters = LocalCharacters;


}

/**
 *	Returns if Player was found and the Username of it.
 *
 * @param The Player ID
 * @output bool if Player was found.
 * @return The Player Username
 */
FString AHordeGameState::GetUsernameBySteamID(FString ID, bool &FoundPlayer)
{
	FString RetUserName = "None";
	bool TempFoundPlayer = false;
	for (auto& PS : PlayerArray)
	{
		AHordePlayerState* PlayerST = Cast<AHordePlayerState>(PS);
		// Fixed: Added null check for GetUniqueId()
		if (PlayerST && PlayerST->GetUniqueId().IsValid())
		{
			if (PlayerST->GetUniqueId()->ToString() == ID)
			{
				RetUserName = PlayerST->GetPlayerName();
				TempFoundPlayer = true;
			}
		}
	}
	FoundPlayer = TempFoundPlayer;
	return RetUserName;
}

/**
 *	Updates Player List in Lobby. Also Checks if All Players are ready. If yes cuts the lobby time and sets game starting to true.
 *
 * @param
 * @return void
 */
void AHordeGameState::UpdatePlayerLobby()
{
	if (HasAuthority())
	{
		LobbyPlayers.Empty();

		if (PlayerArray.Num() >= LobbyInformation.MinimumStartingPlayers)
		{
			StartLobbyTimer();
		}
		else {
			GameStarting = false;
			UnreadyAllPlayers();
			ResetLobbyTime();
		}
		for (auto& PS : PlayerArray)
		{
			AHordePlayerState* PlayerST = Cast<AHordePlayerState>(PS);
			if (PlayerST)
			{
				LobbyPlayers.Add(PlayerST->GetPlayerInfo());
			}
		}
		FreeupUnassignedCharacters();
		for (auto& PS : PlayerArray)
		{
			AHordePlayerState* PlayerST = Cast<AHordePlayerState>(PS);
			if (PlayerST)
			{
				PlayerST->UpdateLobbyPlayerList(LobbyPlayers);
			}
		}
		if (CheckPlayersReady())
		{
			GameStarting = true;
			LobbyTime = 10.f;
		}
	

	}
}

/**
 *	Returns if all players are ready.
 *
 * @param 
 * @return bool if all players are ready.
 */
bool AHordeGameState::CheckPlayersReady()
{
	bool Ready = true;
	for (auto& PLY : LobbyPlayers)
	{
		if (!PLY.PlayerReady)
		{
			Ready = false;
			break;
		}
	}
	return Ready;
}

/**
 *	Sends a chat message directly to a player by given player id ( steam id ).
 *
 * @param The Message and Player ID
 * @return void
 */
void AHordeGameState::MessagePlayer(FHordeChatMessage Message, FString PlayerID)
{
	for (auto PLY: PlayerArray)
	{
		AHordePlayerState* PS = Cast<AHordePlayerState>(PLY);
		if (PS && PS->GetPlayerInfo().PlayerID == PlayerID)
		{
			PS->OnMessageReceived(Message);
		}
	}
}

/**
 *	Sends Message to all Players.
 *
 * @param The Message
 * @return void
 */
void AHordeGameState::PopMessage(FHordeChatMessage Message)
{
	for (auto& PS : PlayerArray)
	{
		AHordePlayerState* PlayerST = Cast<AHordePlayerState>(PS);
		if (PlayerST)
		{
			PlayerST->OnMessageReceived(Message);
		}
	}
}

/**
 *	Returns ID of a Free Character.
 *
 * @param
 * @return FName - ID of free Character. None if none is available.
 */
FName AHordeGameState::GetFreeCharacter()
{
	FName TempChar = "None";
	for (auto& Char : LobbyInformation.AvailableCharacters)
	{
		if (!Char.CharacterTaken)
		{
			TempChar = Char.CharacterID;
			break;
		}
	}
	return TempChar;
}

/**
 *	Checks if all Players are dead. If so we want to End the Game with true so we Reset the Level.
 *
 * @param
 * @return void
 */
void AHordeGameState::AllPlayerDeadCheck()
{
	if (CountAlivePlayers() < 1 && GameStatus == EGameStatus::EINGAME)
	{
		EndGame(true);
	}
}

/**
 *	Ends the current game session, clears up all the game driven timers and sets the next level to be played.
 *
 * @param bool if it should reset the level.
 * @return void
 */
void AHordeGameState::EndGame(bool ResetLevel)
{

	//Reset Round Based Timers if active, so we do not have any problems running any round end logic twice.
	if (GetWorld()->GetTimerManager().IsTimerActive(PauseTimer))
	{
		GetWorld()->GetTimerManager().ClearTimer(PauseTimer);
	}
	if (GetWorld()->GetTimerManager().IsTimerActive(RoundTimer))
	{
		GetWorld()->GetTimerManager().ClearTimer(RoundTimer);
	}

	//If Force Reset Reset Game Instantly
	if (ResetLevel)
	{
		GameOver(*UGameplayStatics::GetCurrentLevelName(GetWorld(), true));
	}
	else
	{
		AHordeWorldSettings* WS = Cast<AHordeWorldSettings>(GetWorld()->GetWorldSettings(false, true));
		if (WS)
		{
			//If Linear Gameplay check if all Zombies are dead. If yes Reset Game with Next Map in Rotation.
			if (ZedsLeft > 0 && WS->MatchMode != EMatchMode::EMatchModeNonLinear)
			{
				PopMessage(FHordeChatMessage("All Zombies needs to be killed to end the Map."));
			}
			else {
				GameOver(GetNextLevelInRotation(ResetLevel));
			}
		}
	
	}

}

/**
 *	Removes Time from the End Screen Time. If over we want to update the client game status to EGameStatus::ServerTravel and ServerTravel to the next Map.
 *
 * @param
 * @return void
 */
void AHordeGameState::ProcessEndTime()
{
	if (EndTime <= 0.f)
	{
		if (GetWorld()->GetTimerManager().IsTimerActive(EndGameTimer))
		{
			GetWorld()->GetTimerManager().ClearTimer(EndGameTimer);
		}
		GameStatus = EGameStatus::ESERVERTRAVEL;
		for (auto& PS : PlayerArray)
		{
			AHordePlayerState* PLY = Cast<AHordePlayerState>(PS);
			if (PLY)
			{
				PLY->ClientUpdateGameStatus(GameStatus);
			}
		}
		UE_LOG(LogTemp, Log, TEXT("ServerTravel to: %s"), *NextLevel.ToString());

		//Delay ServerTravel so our Loading Screen plays the animation.
		FTimerHandle TravelTimer;
		FTimerDelegate TravelTimerDelegate;

		TravelTimerDelegate.BindLambda([this] {
			GetWorld()->ServerTravel(NextLevel.ToString() + "?listen", false, false);
		});

		GetWorld()->GetTimerManager().SetTimer(TravelTimer, TravelTimerDelegate, 1.5f, false);
	
		ResetLobby();
	}
	else 
	{
		EndTime--;
	}
}

/**
 *	Resets the Lobby Time, the bool values and the GameStatus to EGameStatus::ELOBBY
 *
 * @param
 * @return void
 */
void AHordeGameState::ResetLobby()
{
	AHordeWorldSettings* WS = Cast<AHordeWorldSettings>(GetWorld()->GetWorldSettings(false, true));
	if (WS)
	{
		LobbyTime = 300.f;
		GameStarting = false;
		BlockDisconnect = false;
		GameStatus = EGameStatus::ELOBBY;
	}
}

/**
 *	Returns score for the end of the game to display.
 *
 * @param
 * @output The MVP, The player with the most headshots and kills.
 * @return void
 */
void AHordeGameState::CalcEndScore(FPlayerScore& MVP, FPlayerScore& HS, FPlayerScore& KS)
{
	FPlayerScore LocalMVP("MVP");
	FPlayerScore LocalHS("Most Headshots");
	FPlayerScore LocalKS("Most Kills");

	for (auto& PS : PlayerArray)
	{
		AHordePlayerState * PLY = Cast<AHordePlayerState>(PS);
		if (PLY)
		{
			if (PLY->Points > LocalMVP.Score)
			{
				LocalMVP.Score = PLY->Points;
				LocalMVP.PlayerID = PLY->GetPlayerInfo().PlayerID;
				LocalMVP.Character = PLY->GetPlayerInfo().SelectedCharacter;
				LocalMVP.ScoreType = PLY->GetPlayerInfo().UserName;
			}
			if (PLY->HeadShots > LocalHS.Score)
			{
				LocalHS.Score = PLY->HeadShots;
				LocalHS.PlayerID = PLY->GetPlayerInfo().PlayerID;
				LocalHS.Character = PLY->GetPlayerInfo().SelectedCharacter;
				LocalHS.ScoreType = PLY->GetPlayerInfo().UserName;
			}
			if (PLY->ZedKills > LocalKS.Score)
			{
				LocalKS.Score = PLY->ZedKills;
				LocalKS.PlayerID = PLY->GetPlayerInfo().PlayerID;
				LocalKS.Character = PLY->GetPlayerInfo().SelectedCharacter;
				LocalKS.ScoreType = PLY->GetPlayerInfo().UserName;
			}
		}
	}

	MVP = LocalMVP;
	HS = LocalHS;
	KS = LocalKS;
	
}

/**
 *	Returns amount of alive players in game.
 *
 * @param
 * @return int32 Amount of Alive Players
 */
int32 AHordeGameState::CountAlivePlayers()
{
	int32 TempAliveCount = 0;
	for (TActorIterator<AHordeBaseCharacter> ActorItr(GetWorld()); ActorItr; ++ActorItr)
	{
		AHordeBaseCharacter* PLYChar = *ActorItr;
		if (PLYChar && !PLYChar->GetIsDead())
		{
			TempAliveCount++;
		}
	}
	return TempAliveCount;
}

/**
 *	Counts alive zombies on the map and returns it.
 *
 * @param
 * @return int32 Amount of alive zombies.
 */
int32 AHordeGameState::CountAliveZeds()
{
	int32 TempAliveCount = 0;
	for (TActorIterator<AZedPawn> ActorItr(GetWorld()); ActorItr; ++ActorItr)
	{
		AZedPawn* Zed = *ActorItr;
		if (Zed && !Zed->GetIsDead())
		{
			TempAliveCount++;
		}

	}
	return TempAliveCount;
}

/**
 *	Updates ZedsLeft (Int32; Exposed to BP) and runs Check for Game Over.
 *
 * @param
 * @return void
 */
void AHordeGameState::UpdateAliveZeds()
{
	ZedsLeft = CountAliveZeds();
	AHordeGameMode* GMO = Cast<AHordeGameMode>(GetWorld()->GetAuthGameMode());
	if (GMO)
	{
		GMO->CheckGameOver();
	}
}

/**
 *	Initiates Character Trade inside lobby with given Player IDs.
 *
 * @param The Instigator Player ID and the Target Player ID.
 * @return void
 */
void AHordeGameState::StartCharacterTrade(FString InstigatorPlayer, FString TargetPlayer)
{
	if (!IsTradeInProgress && InstigatorPlayer != "" && TargetPlayer != "" && !GetWorld()->GetTimerManager().IsTimerActive(LobbyTradeTimer))
	{
		TradeProgress.Instigator = InstigatorPlayer;
		TradeProgress.Target = TargetPlayer;
		TradeProgress.TimeLeft = 20.f;
		GetWorld()->GetTimerManager().SetTimer(LobbyTradeTimer, this, &AHordeGameState::ProcessCharacterTrade, 1.f, true);
		IsTradeInProgress = true;
	}
}

/**
 *	Calculates Character Trade Time left. When time is < 0 Character Trade gets canceled.
 *
 * @param
 * @return void
 */
void AHordeGameState::ProcessCharacterTrade()
{
	if (TradeProgress.TimeLeft > 0)
	{
		TradeProgress.TimeLeft--;
	}
	else {
		AbortLobbyTrade();
	}
}

/**
 *	Accepts Character Trade and Swaps Characters of Instigator and Target Player. After all that it updates the lobby.
 *	Fixed: Added bounds checking for character indices before array access.
 *
 * @param
 * @return void
 */
void AHordeGameState::AcceptCharacterTrade()
{

	int32 TargetCharacterIndex = -1;
	int32 InstigatorCharacterIndex = -1;
	FName TargetCharacter = GetCharacterByID(TradeProgress.Target, TargetCharacterIndex);
	FName InstigatorCharacter = GetCharacterByID(TradeProgress.Instigator, InstigatorCharacterIndex);

	// Fixed: Check that both indices are valid before proceeding with the trade
	if (!LobbyInformation.AvailableCharacters.IsValidIndex(InstigatorCharacterIndex) ||
		!LobbyInformation.AvailableCharacters.IsValidIndex(TargetCharacterIndex))
	{
		UE_LOG(LogTemp, Warning, TEXT("AcceptCharacterTrade: Invalid character indices (Instigator: %d, Target: %d). Aborting trade."), InstigatorCharacterIndex, TargetCharacterIndex);
		AbortLobbyTrade();
		return;
	}

	AHordePlayerState* TargetPlayerState = GetPlayerStateByID(TradeProgress.Target);
	if (TargetPlayerState)
	{
		LobbyInformation.AvailableCharacters[InstigatorCharacterIndex].PlayerID = TargetPlayerState->GetPlayerInfo().PlayerID;
		LobbyInformation.AvailableCharacters[InstigatorCharacterIndex].PlayerUsername = TargetPlayerState->GetPlayerInfo().UserName;
		TargetPlayerState->SwitchCharacter(InstigatorCharacter);
	}

	AHordePlayerState* InstigatorPlayerState = GetPlayerStateByID(TradeProgress.Instigator);
	if (InstigatorPlayerState)
	{
		LobbyInformation.AvailableCharacters[TargetCharacterIndex].PlayerID = InstigatorPlayerState->GetPlayerInfo().PlayerID;
		LobbyInformation.AvailableCharacters[TargetCharacterIndex].PlayerUsername = InstigatorPlayerState->GetPlayerInfo().UserName;
		InstigatorPlayerState->SwitchCharacter(TargetCharacter);
	}
	AbortLobbyTrade();
	UpdatePlayerLobby();

}

/**
 *	Stops Character Trade and resets Character Trade Structure.
 *
 * @param
 * @return void
 */
void AHordeGameState::AbortLobbyTrade()
{
	if (GetWorld()->GetTimerManager().IsTimerActive(LobbyTradeTimer))
	{
		GetWorld()->GetTimerManager().ClearTimer(LobbyTradeTimer);
	}
	TradeProgress.Instigator = "";
	TradeProgress.Target = "";
	TradeProgress.TimeLeft = 20.f;
	IsTradeInProgress = false;
}


