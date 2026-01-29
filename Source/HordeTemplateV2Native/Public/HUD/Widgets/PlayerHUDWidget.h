// Copyright (c) 2018 - 2026 Marc Fraedrich
// Horde Template V2 (Native) - A cooperative zombie survival game framework
// Licensed under the MIT License

#pragma once

/**
 * @file PlayerHUDWidget.h
 * @brief Main in-game HUD showing health, stamina, ammo, rounds, and interaction prompts
 * @author Marc Fraedrich
 */

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/KismetMaterialLibrary.h"
#include "Materials/MaterialInstanceConstant.h"
#include "PlayerHUDWidget.generated.h"
UCLASS()
class HORDETEMPLATEV2NATIVE_API UPlayerHUDWidget : public UUserWidget
{
	GENERATED_BODY()

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSetInteractionText, const FText&, InteractionText);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnHideInteractionText);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBindingVariables);

protected:

	UPROPERTY()
		UMaterialInstance* CircleMaterialInstance;

public:

	UPROPERTY(BlueprintAssignable, Category = "Delegates")
		FOnSetInteractionText OnSetInteractionText;

	UPROPERTY(BlueprintAssignable, Category = "Delegates")
		FOnHideInteractionText OnHideInteractionText;

	UPROPERTY(BlueprintAssignable, Category = "Delegates")
		FOnBindingVariables OnBindingVariables;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Interaction")
		bool IsInteracting = false;

	UPROPERTY()
		float InteractionValueToInterpolate = 0.f;

	UFUNCTION(BlueprintImplementableEvent, Category = "Interaction")
		void SetInteractionText(const FText &InteractionText);

	UFUNCTION(BlueprintImplementableEvent, Category = "Interaction")
		void HideInteractionText();

	UFUNCTION(BlueprintImplementableEvent, Category = "Score")
		void OnPointsReceived(EPointType PType, int32 Points);

	UFUNCTION(BlueprintImplementableEvent, Category = "Interaction")
		void VariablesBound();

	UFUNCTION(BlueprintCallable, Category = "Interaction")
		void HideInteractionTxt();

	UFUNCTION(BlueprintPure, Category = "Visibility")
		ESlateVisibility IsOwningCharacter();

	UPROPERTY(BlueprintReadOnly, Category = "Interaction")
		UMaterialInstanceDynamic* ProgressCircleDynamic;

	UPROPERTY()
		float HealthInterpolate = 100.f;

	UPROPERTY()
		float StaminaInterpolate = 100.f;

	UFUNCTION(BlueprintPure, Category = "HUD")
		float GetPlayerHealth();

	UFUNCTION(BlueprintPure, Category = "HUD")
		float GetPlayerStamina();

	UFUNCTION(BlueprintPure, Category = "HUD")
		ESlateVisibility bGetIsDead();

	UFUNCTION(BlueprintPure, Category = "HUD")
		FText GetWeaponText();

	UFUNCTION(BlueprintPure, Category = "HUD")
		FText GetHealthText();

	UFUNCTION(BlueprintPure, Category = "HUD")
		FText GetZedsLeft();

	UFUNCTION(BlueprintPure, Category = "HUD")
		FText GetRounds();

	UFUNCTION(BlueprintPure, Category = "HUD")
		ESlateVisibility bIsSpectator();

	UFUNCTION(BlueprintPure, Category = "HUD")
		ESlateVisibility bIsInteracting();

	UFUNCTION(BlueprintPure, Category = "HUD")
		FText GetWeaponFireMode();

	UFUNCTION(BlueprintPure, Category = "HUD")
		ESlateVisibility GetGameType();

	UFUNCTION(BlueprintPure, Category = "HUD")
		FText GetRoundTime();


	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	
};
