// Copyright (c) 2018 - 2026 Marc Fraedrich
// Horde Template V2 (Native) - A cooperative zombie survival game framework
// Licensed under the MIT License

#include "TraderItemWidget.h"
#include "Gameplay/HordePlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "HordeTemplateV2Native.h"

/**
 * @file TraderItemWidget.cpp
 * @brief Implementation of trader item purchase functionality with price checking
 * @author Marc Fraedrich
 */

/**
 * Buys Item and plays buy sound on owning player.
 *
 * @param
 * @return void
 */
void UTraderItemWidget::BuyItem()
{
	AHordePlayerState* PS = Cast<AHordePlayerState>(GetOwningPlayer()->PlayerState);
	if (PS)
	{
		if ((PS->GetPlayerMoney() - TraderItem.ItemPrice) >= 0) 
		{
			PS->BuyItem(TraderItem.ItemID);
			USoundCue* BuySound = ObjectFromPath<USoundCue>(TRADER_BUY_SOUND);
			if (BuySound)
			{
				UGameplayStatics::PlaySound2D(GetWorld(), BuySound);
			}
			
		}
	}
}

/**
 * Returns the current item price as text.
 *
 * @param
 * @return Current Item Price as Text with Currency Prefix.
 */
FText UTraderItemWidget::GetPriceText()
{
	return FText::FromString(FString::FromInt(TraderItem.ItemPrice) + CURRENCY_PREFIX);
}

/**
 * Returns true if Player has enough money to afford item. 
 *
 * @param
 * @return true if Player can afford else false.
 */
bool UTraderItemWidget::HasEnoughMoney()
{
	AHordePlayerState* PS = Cast<AHordePlayerState>(GetOwningPlayer()->PlayerState);
	if (PS)
	{
		return (PS->GetPlayerMoney() - TraderItem.ItemPrice) >= 0;
	}
	else
	{
		return false;
	}
}
