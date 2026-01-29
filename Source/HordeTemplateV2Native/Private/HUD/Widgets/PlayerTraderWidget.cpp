// Copyright (c) 2018 - 2026 Marc Fraedrich
// Horde Template V2 (Native) - A cooperative zombie survival game framework
// Licensed under the MIT License

#include "PlayerTraderWidget.h"
#include "Gameplay/HordePlayerState.h"

/**
 * @file PlayerTraderWidget.cpp
 * @brief Implementation of trader shop interface with player money display
 * @author Marc Fraedrich
 */

/** ( Virtual; Overridden )
 * Sets widget bIsFocusable to true.
 *
 * @param
 * @return
 */
void UPlayerTraderWidget::NativeConstruct()
{
	Super::NativeConstruct();

	SetIsFocusable(true);
}

/**
 * Returns Players Money as Text.
 *
 * @param
 * @return Players Money as Text.
 */
FText UPlayerTraderWidget::GetPlayerMoney()
{
	FText PlayerMoney;
	AHordePlayerState* PS = Cast<AHordePlayerState>(GetOwningPlayer()->PlayerState);
	if (PS)
	{
		PlayerMoney = FText::FromString(FString::FromInt(PS->PlayerMoney) + " $");
	}
	return PlayerMoney;
}
