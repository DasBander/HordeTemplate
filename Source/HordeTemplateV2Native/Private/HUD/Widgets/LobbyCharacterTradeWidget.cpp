// Copyright (c) 2018 - 2026 Marc Fraedrich
// Horde Template V2 (Native) - A cooperative zombie survival game framework
// Licensed under the MIT License

#include "LobbyCharacterTradeWidget.h"
#include "Gameplay/HordeGameState.h"
#include "Gameplay/HordePlayerState.h"
#include "HordeTemplateV2Native.h"
#include "Blueprint/WidgetBlueprintLibrary.h"

/**
 * @file LobbyCharacterTradeWidget.cpp
 * @brief Implementation of character trading UI displaying trade progress and character info
 * @author Marc Fraedrich
 */

/**
 * Returns the Trade Time for the Trade Widget.
 *
 * @param
 * @return Trade Time in Minutes:Seconds format.
 */
FText ULobbyCharacterTradeWidget::GetTradeTime()
{
	AHordeGameState* GS = Cast<AHordeGameState>(GetWorld()->GetGameState());
	if (GS)
	{
		float LobbyTime = GS->TradeProgress.TimeLeft;

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
 * Returns Returns the Character Name of the Target Player.
 *
 * @param
 * @return Character Name of Target Player.
 */
FText ULobbyCharacterTradeWidget::GetTargetCharacterName()
{
	FString RetStr = "None";
	AHordeGameState* GS = Cast<AHordeGameState>(GetWorld()->GetGameState());
	if (GS)
	{
		int32 TempIndex;
		FPlayableCharacter Char = FromDatatable<FPlayableCharacter>(CHARACTER_DATATABLE_PATH, GS->GetCharacterByID(GS->TradeProgress.Target, TempIndex));
		RetStr = Char.CharacterTitle;
	}

	return FText::FromString(RetStr);
}

/**
 * Returns Image of Target Players Character.
 *
 * @param
 * @return Image of Target Players Character.
 */
FSlateBrush ULobbyCharacterTradeWidget::GetTargetCharacterImage()
{
	FSlateBrush RetBrush;
	AHordeGameState* GS = Cast<AHordeGameState>(GetWorld()->GetGameState());
	if (GS)
	{
		int32 TempIndex;
		FPlayableCharacter Char = FromDatatable<FPlayableCharacter>(CHARACTER_DATATABLE_PATH, GS->GetCharacterByID(GS->TradeProgress.Target, TempIndex));
		RetBrush = UWidgetBlueprintLibrary::MakeBrushFromTexture(Char.CharacterImage, 64, 64);
	}
	return RetBrush;
}

/**
 * Returns Own Character Name
 *
 * @param
 * @return Own Character Name
 */
FText ULobbyCharacterTradeWidget::GetOwnCharacterName()
{
	FString RetStr = "None";
	AHordeGameState* GS = Cast<AHordeGameState>(GetWorld()->GetGameState());
	if (GS)
	{
		int32 TempIndex;
		FPlayableCharacter Char = FromDatatable<FPlayableCharacter>(CHARACTER_DATATABLE_PATH, GS->GetCharacterByID(GS->TradeProgress.Instigator, TempIndex));
		RetStr = Char.CharacterTitle;
	}

	return FText::FromString(RetStr);
}

/**
 * Returns own characters image.
 *
 * @param
 * @return Own Characters Image
 */
FSlateBrush ULobbyCharacterTradeWidget::GetOwnCharacterImage()
{
	FSlateBrush RetBrush;
	AHordeGameState* GS = Cast<AHordeGameState>(GetWorld()->GetGameState());
	if (GS)
	{
		int32 TempIndex;
		FPlayableCharacter Char = FromDatatable<FPlayableCharacter>(CHARACTER_DATATABLE_PATH, GS->GetCharacterByID(GS->TradeProgress.Instigator, TempIndex));
		RetBrush = UWidgetBlueprintLibrary::MakeBrushFromTexture(Char.CharacterImage, 64, 64);
	}
	return RetBrush;
}

/**
 * Returns if you are in a character trade involved.
 *
 * @param
 * @return Visible if involved in a character trade or collapsed.
 */
ESlateVisibility ULobbyCharacterTradeWidget::IsInCharacterTrade()
{
	ESlateVisibility RetVis = ESlateVisibility::Collapsed;
	AHordeGameState* GS = Cast<AHordeGameState>(GetWorld()->GetGameState());
	if (GS)
	{
		AHordePlayerState* PS = Cast<AHordePlayerState>(GetOwningPlayer()->PlayerState);
		if (PS)
		{
			if (GS->TradeProgress.Target == PS->GetPlayerInfo().PlayerID)
			{
				RetVis = ESlateVisibility::Visible;
			}
		}
	}
	return RetVis;
}
