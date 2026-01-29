// Copyright (c) 2018 - 2026 Marc Fraedrich
// Horde Template V2 (Native) - A cooperative zombie survival game framework
// Licensed under the MIT License

#include "PlayerEndScreen.h"
#include "Gameplay/HordeGameState.h"
#include "HordeTemplateV2Native.h"

/**
 * @file PlayerEndScreen.cpp
 * @brief Implementation of end-game screen with MVP, kills, headshots, and next level info
 * @author Marc Fraedrich
 */

/**
 * Gets the username of the current MVP.
 *
 * @param
 * @return The Player Name of the MVP
 */
FText UPlayerEndScreen::GetMVPName()
{
	FText RetText = FText::FromString("None");
	AHordeGameState* GS = Cast<AHordeGameState>(GetWorld()->GetGameState());
	if (GS)
	{
		RetText = FText::FromString(*GS->Score_MVP.ScoreType);
	}
	return RetText;
}

/**
 * Returns the username of the player with the most head shots.
 *
 * @param
 * @return Player Username of the player with the most head shots.
 */
FText UPlayerEndScreen::GetMHSName()
{
	FText RetText = FText::FromString("None");
	AHordeGameState* GS = Cast<AHordeGameState>(GetWorld()->GetGameState());
	if (GS)
	{
		RetText = FText::FromString(*GS->Score_MostHeadshots.ScoreType);
	}
	return RetText;
}

/**
 * Returns the username of the player with the most kills.
 *
 * @param
 * @return Player Username of the player with the most kills.
 */
FText UPlayerEndScreen::GetMKSName()
{
	FText RetText = FText::FromString("None");
	AHordeGameState* GS = Cast<AHordeGameState>(GetWorld()->GetGameState());
	if (GS)
	{
		RetText = FText::FromString(*GS->Score_MostKills.ScoreType);
	}
	return RetText;
}

/**
 * Returns the points from the MVP.
 *
 * @param
 * @return Points of the MVP
 */
FText UPlayerEndScreen::GetMVPPoints()
{
	FText RetText = FText::FromString("0");
	AHordeGameState* GS = Cast<AHordeGameState>(GetWorld()->GetGameState());
	if (GS)
	{
		RetText = FText::FromString(*FString::FromInt(GS->Score_MVP.Score));
	}
	return RetText;
}

/**
 * Returns the amount of the head shots from the player that has the most.
 *
 * @param
 * @return Most Head shots.
 */
FText UPlayerEndScreen::GetMHSPoints()
{
	FText RetText = FText::FromString("0");
	AHordeGameState* GS = Cast<AHordeGameState>(GetWorld()->GetGameState());
	if (GS)
	{
		RetText = FText::FromString(*FString::FromInt(GS->Score_MostHeadshots.Score));
	}
	return RetText;
}

/**
 * Returns the amount of the kills from the player that has the most.
 *
 * @param
 * @return Most Kills.
 */
FText UPlayerEndScreen::GetMKSPoints()
{
	FText RetText = FText::FromString("0");
	AHordeGameState* GS = Cast<AHordeGameState>(GetWorld()->GetGameState());
	if (GS)
	{
		RetText = FText::FromString(*FString::FromInt(GS->Score_MostKills.Score));
	}
	return RetText;
}

/**
 * Returns the End Time of the End Game Timer.
 *
 * @param
 * @return Returns the End Time in Minutes:Seconds format.
 */
FText UPlayerEndScreen::GetEndTime()
{
	FFormatNamedArguments Args;
	AHordeGameState* GS = Cast<AHordeGameState>(GetWorld()->GetGameState());
	if (GS)
	{
		float LobbyTime = GS->EndTime;

		int32 Minutes = FMath::FloorToInt(LobbyTime / 60.f);
		int32 Seconds = FMath::TruncToInt(LobbyTime - (Minutes * 60.f));

		FString TimeStr = FString::Printf(TEXT("%s%s : %s%s"), (Minutes < 10) ? TEXT("0") : TEXT(""), *FString::FromInt(Minutes), (Seconds < 10) ? TEXT("0") : TEXT(""), *FString::FromInt(Seconds));

		return FText::FromString(TimeStr);
	}
	else {
		return FText::FromString("nA / nA");
	}
}

/**
 * Returns the name of the Next Level.
 *
 * @param
 * @return Name of Next Level
 */
FText UPlayerEndScreen::GetNextLevel()
{
	FString RetText = "NEXT: Not found!";
	AHordeGameState* GS = Cast<AHordeGameState>(GetWorld()->GetGameState());
	if (GS)
	{
		FPlayableLevel Ply = FindLevelByID(GS->NextLevel);
		if (Ply.RawLevelName != NAME_None)
		{
			RetText = "NEXT: " + Ply.LevelName.ToString();
		}
	}

	return FText::FromString(RetText);
}

/**
 * Returns Level Structure by given Level ID
 *
 * @param
 * @return FPlayableLevel by given ID.
 */
FPlayableLevel UPlayerEndScreen::FindLevelByID(FName LevelID)
{
	UDataTable* InventoryData = Cast<UDataTable>(StaticLoadObject(UDataTable::StaticClass(), nullptr, MAPS_DATATABLE_PATH));
	FPlayableLevel TempItem;

	if (InventoryData) {
		FPlayableLevel* ItemFromRow = InventoryData->FindRow<FPlayableLevel>(LevelID, "PlayerEndScreen Widget - Find Level By ID", false);
		if (ItemFromRow)
		{
			TempItem = *ItemFromRow;
		}
	}
	else {
		GLog->Log("Player Maps Data not valid.");
	}

	return TempItem;
}