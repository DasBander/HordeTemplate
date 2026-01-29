// Copyright (c) 2018 - 2026 Marc Fraedrich
// Horde Template V2 (Native) - A cooperative zombie survival game framework
// Licensed under the MIT License

#include "PlayerHeadDisplay.h"

/**
 * @file PlayerHeadDisplay.cpp
 * @brief Implementation of overhead player name and health bar display
 * @author Marc Fraedrich
 */

/** ( Virtual; Overridden )
 * Binds Delegates to On Show Widget and On Hide Widget.
 *
 * @param
 * @return void
 */
void UPlayerHeadDisplay::NativeConstruct()
{
	Super::NativeConstruct();

	OnShowWidgetDelegate.AddDynamic(this, &UPlayerHeadDisplay::OnShowWidget);
	OnHideWidgetDelegate.AddDynamic(this, &UPlayerHeadDisplay::OnHideWidget);
}

/**
 * Returns Player Name
 *
 * @param
 * @return Returns Player name as FText.
 */
FText UPlayerHeadDisplay::GetPlayerName()
{
	return FText::FromString(PlayerName);
}

/**
 * Returns Interpolated Health 0-1 for Progress bar.
 *
 * @param
 * @return float Progress ( Health )
 */
float UPlayerHeadDisplay::GetPlayerHealth()
{
	InterpHealth = FMath::FInterpTo(InterpHealth, Health, GetWorld()->GetDeltaSeconds(), 2.f);
	return InterpHealth / 100.f;
}
