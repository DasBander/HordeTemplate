// Copyright (c) 2018 - 2026 Marc Fraedrich
// Horde Template V2 (Native) - A cooperative zombie survival game framework
// Licensed under the MIT License

#include "PlayerHUDWidget.h"
#include "GameFramework/Character.h"
#include "Character/HordeBaseCharacter.h"
#include "Gameplay/HordeGameState.h"
#include "Gameplay/HordeWorldSettings.h"
#include "InventoryHelpers.h"
#include "HordeTemplateV2Native.h"

/**
 * @file PlayerHUDWidget.cpp
 * @brief Implementation of main gameplay HUD with health, weapon, and game state display
 * @author Marc Fraedrich
 */

/**
 * Calls delegate to hide the Interaction Text.
 *
 * @param
 * @return void
 */
void UPlayerHUDWidget::HideInteractionTxt()
{
	OnHideInteractionText.Broadcast();
}

/**
 * Returns Slate Visibility as Visible if Player Owns a Character.
 *
 * @param
 * @return Visibility if Owner has Character.
 */
ESlateVisibility UPlayerHUDWidget::IsOwningCharacter()
{
	ESlateVisibility DefaultVisibilty = ESlateVisibility::Hidden;
	ACharacter* PLY = Cast<ACharacter>(GetOwningPlayerPawn());
	if (PLY)
	{
		DefaultVisibilty = ESlateVisibility::Visible;
	}
	return DefaultVisibilty;
}

/**
 * Returns Interpolated Player Health for Progress bar ( 0.f - 1.f )
 *
 * @param
 * @return Player Health from 0.f - 1.f
 */
float UPlayerHUDWidget::GetPlayerHealth()
{
	AHordeBaseCharacter* PLY = Cast<AHordeBaseCharacter>(GetOwningPlayerPawn());
	if (PLY)
	{
		HealthInterpolate = FMath::FInterpTo(HealthInterpolate, PLY->GetHealth(), GetWorld()->GetDeltaSeconds(), 2.f);
		return HealthInterpolate / 100.f;
	}
	else {
		return 0.f;
	}
}

/**
 * Returns Interpolated Player Stamina for Progress bar ( 0.f - 1.f )
 *
 * @param
 * @return Player Stamina from 0.f - 1.f
 */
float UPlayerHUDWidget::GetPlayerStamina()
{
	AHordeBaseCharacter* PLY = Cast<AHordeBaseCharacter>(GetOwningPlayerPawn());
	if (PLY)
	{
		StaminaInterpolate = FMath::FInterpTo(StaminaInterpolate, PLY->GetStamina(), GetWorld()->GetDeltaSeconds(), 2.f);
		return StaminaInterpolate / 100.f;
	}
	else {
		return 0.f;
	}
}

/**
 * Returns Visibility depending on if Player is dead or not.
 *
 * @param
 * @return  Visible depending on if Player is dead or not.
 */
ESlateVisibility UPlayerHUDWidget::bGetIsDead()
{
	AHordeBaseCharacter* PLY = Cast<AHordeBaseCharacter>(GetOwningPlayerPawn());
	if (PLY)
	{
		return (PLY->GetIsDead()) ? ESlateVisibility::Collapsed : ESlateVisibility::Visible;
	}
	else
	{
		return ESlateVisibility::Collapsed;
	}
}

/**
 * Returns Weapon Text as ( Weapon Name - 12 / 350 )
 *
 * @param
 * @return Weapon Text ( Weapon Name - Loaded Ammo / Amount of Ammo in Inventory )
 */
FText UPlayerHUDWidget::GetWeaponText()
{
	FString ReturnString;
	AHordeBaseCharacter* PLY = Cast<AHordeBaseCharacter>(GetOwningPlayerPawn());
	ABaseFirearm* Firearm = PLY->GetCurrentFirearm();
	
	if (PLY && Firearm)
	{
		FItem Itm = UInventoryHelpers::FindItemByID(FName(*Firearm->WeaponID));
		ReturnString = (PLY->Inventory->AvailableAmmoForCurrentWeapon == 0) ? Itm.ItemName + " - " + FString::FromInt(Firearm->LoadedAmmo) : Firearm->WeaponID + " - " + FString::FromInt(Firearm->LoadedAmmo) + " / " + FString::FromInt(PLY->Inventory->AvailableAmmoForCurrentWeapon);
	}
	return FText::FromString(ReturnString);
}

/**
 * Returns Health as Text.
 *
 * @param
 * @return Health as Text.
 */
FText UPlayerHUDWidget::GetHealthText()
{
	return FText::FromString(FString::SanitizeFloat(HealthInterpolate, 0));
}

/**
 * Returns amount of Remaining Zombies.
 *
 * @param
 * @return Amount of Remaining Zeds as Text.
 */
FText UPlayerHUDWidget::GetZedsLeft()
{
	FString ReturnString = "None";
	AHordeGameState* GS = Cast<AHordeGameState>(GetWorld()->GetGameState());
	if (GS)
	{
		ReturnString = FString::FromInt(GS->ZedsLeft);
	}
	return FText::FromString(ReturnString);
}

/**
 * Returns Amount of Rounds as Text ( 1 / 10 )
 *
 * @param
 * @return Amount of Rounds ( Current Round / Max Rounds )
 */
FText UPlayerHUDWidget::GetRounds()
{
	FString ReturnString = "Unknown Rounds";
	AHordeGameState* GS = Cast<AHordeGameState>(GetWorld()->GetGameState());
	if (GS)
	{
		ReturnString = FString::FromInt(GS->GameRound) + " / " + FString::FromInt(GS->GetHordeWorldSettings()->MaxRounds) + " ROUNDS";
	}
	return FText::FromString(ReturnString);
}

/**
 * Returns Visibility depending on if owner is character or spectator.
 *
 * @param
 * @return Visibility if owner is spectator.
 */
ESlateVisibility UPlayerHUDWidget::bIsSpectator()
{
	AHordeBaseCharacter* PLY = Cast<AHordeBaseCharacter>(GetOwningPlayerPawn());
	return (PLY) ? ESlateVisibility::Collapsed : ESlateVisibility::Visible;
}

/**
 * Returns Visibility depending on if owner is interacting.
 *
 * @param
 * @return Visibility if owner is interacting.
 */
ESlateVisibility UPlayerHUDWidget::bIsInteracting()
{
	return (IsInteracting) ? ESlateVisibility::Visible : ESlateVisibility::Hidden;
}

/**
 * Returns current Weapons FireMode as Text.
 *
 * @param
 * @return Returns Weapons FireMode as Text.
 */
FText UPlayerHUDWidget::GetWeaponFireMode()
{
	FString ReturnString;
	AHordeBaseCharacter* PLY = Cast<AHordeBaseCharacter>(GetOwningPlayerPawn());
	ABaseFirearm* Firearm = PLY->GetCurrentFirearm();
	if (PLY && Firearm)
	{
		switch ((EFireMode)Firearm->FireMode) {
			case EFireMode::EFireModeSingle:
				ReturnString = "Single";
			break;

			case EFireMode::EFireModeBurst:
				ReturnString = "Burst";
			break;

			case EFireMode::EFireModeFull:
				ReturnString = "Full Automatic";
			break;

			default:
			break;
		}
			
	}
	return FText::FromString(ReturnString);
}

/**
 * Returns Visibility depending on game type. ( Visible if NonLinear / Collapsed if Linear )
 *
 * @param
 * @return ( Visible if NonLinear / Collapsed if Linear )
 */
ESlateVisibility UPlayerHUDWidget::GetGameType()
{
	AHordeGameState* GS = Cast<AHordeGameState>(GetWorld()->GetGameState());
	if (GS)
	{
		return (GS->MatchMode == EMatchMode::EMatchModeNonLinear) ? ESlateVisibility::Visible : ESlateVisibility::Collapsed;
	}
	else {
		return ESlateVisibility::Collapsed;
	}
}

/**
 * Returns Round Time as Text in the Minutes:Seconds format.
 *
 * @param
 * @return Round Time as Minutes:Seconds
 */
FText UPlayerHUDWidget::GetRoundTime()
{
	FFormatNamedArguments Args;
	AHordeGameState* GS = Cast<AHordeGameState>(GetWorld()->GetGameState());
	if (GS)
	{
		float LobbyTime = (GS->IsRoundPaused) ? GS->PauseTime : GS->RoundTime;

		int32 Minutes = FMath::FloorToInt(LobbyTime / 60.f);
		int32 Seconds = FMath::TruncToInt(LobbyTime - (Minutes * 60.f));

		FString TimeStr = FString::Printf(TEXT("%s%s : %s%s"), (Minutes < 10) ? TEXT("0") : TEXT(""), *FString::FromInt(Minutes), (Seconds < 10) ? TEXT("0") : TEXT(""), *FString::FromInt(Seconds));

		return FText::FromString(TimeStr + ((GS->IsRoundPaused) ? " Pause" : " Game Time"));
	}
	else {
		return FText::FromString("nA / nA");
	}
}

/** ( Virtual; Overridden )
 *	Creates Circle Progress Bar Material Instance and binds Delegates.
 *
 * @param
 * @return void
 */
void UPlayerHUDWidget::NativeConstruct()
{
	
	/*
	Creates Dynamic Object for Circle Material so we can adjust the parameters for it.
	*/

	CircleMaterialInstance = ObjectFromPath<UMaterialInstanceConstant>(FName(TEXT("MaterialInstanceConstant'/Game/HordeTemplateBP/Assets/Materials/UI/MI_ProgressCircle.MI_ProgressCircle'")));

	if (CircleMaterialInstance)
	{
		ProgressCircleDynamic = UKismetMaterialLibrary::CreateDynamicMaterialInstance(GetWorld(), CircleMaterialInstance);
	}

	OnSetInteractionText.AddDynamic(this, &UPlayerHUDWidget::SetInteractionText);
	OnHideInteractionText.AddDynamic(this, &UPlayerHUDWidget::HideInteractionText);
	OnBindingVariables.AddDynamic(this, &UPlayerHUDWidget::VariablesBound);

	OnBindingVariables.Broadcast();

	Super::NativeConstruct();
}

/** ( Virtual; Overridden )
 * Updates the Circle Progress Bar Material with the Interaction Progress Value.
 *
 * @param The Geometry and DeltaTime
 * @return void
 */
void UPlayerHUDWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (IsInteracting && ProgressCircleDynamic)
	{

		AHordeBaseCharacter* PLY = Cast<AHordeBaseCharacter>(GetOwningPlayerPawn());
		if (PLY)
		{
			ProgressCircleDynamic->SetScalarParameterValue(FName(TEXT("Progress")), FMath::GetMappedRangeValueClamped(FVector2D(0.f, 100.f), FVector2D(0.f, 1.f), FMath::FInterpTo(PLY->InteractionProgress, InteractionValueToInterpolate, InDeltaTime, 0.1f)));
		}
	}
}

