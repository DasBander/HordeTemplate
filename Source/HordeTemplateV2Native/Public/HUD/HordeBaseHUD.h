// Copyright (c) 2018 - 2026 Marc Fraedrich
// Horde Template V2 (Native) - A cooperative zombie survival game framework
// Licensed under the MIT License

#pragma once

/**
 * @file HordeBaseHUD.h
 * @brief Main HUD class managing widget creation, display, and game state transitions
 * @author Marc Fraedrich
 */

#include "CoreMinimal.h"
#include "LobbyStructures.h"
#include "GameFramework/HUD.h"

#include "Widgets/PlayerHUDWidget.h"
#include "Widgets/PlayerLobbyWidget.h"
#include "Widgets/PlayerTraderWidget.h"
#include "Widgets/PlayerEndScreen.h"
#include "Widgets/PlayerTravelWidget.h"
#include "Widgets/PlayerEscapeMenu.h"
#include "Widgets/PlayerScoreboardWidget.h"

#include "HordeBaseHUD.generated.h"

// forward declares for your widgets (adjust to your project)
class UPlayerHUDWidget;
class UPlayerLobbyWidget;
class UPlayerTraderWidget;
class UPlayerEndScreen;
class UPlayerTravelWidget;
class UPlayerEscapeMenu;
class UPlayerScoreboardWidget;
class UFont;
class UTexture2D;



DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGameStatusChanged, uint8, GameStatus);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPlayerPointsReceived, EPointType, PointType, int32, Points);

UCLASS()
class HORDETEMPLATEV2NATIVE_API AHordeBaseHUD : public AHUD
{
	GENERATED_BODY()

public:
	// IMPORTANT: explicitly declare the ctor with default object initializer
	AHordeBaseHUD();


	
	UFUNCTION() void GameStatusChanged(uint8 GameStatus);
	UFUNCTION() void OnPlayerPointsReceived(EPointType PointType, int32 Points);

	UFUNCTION(BlueprintCallable) UPlayerHUDWidget* GetHUDWidget();
	UFUNCTION(BlueprintCallable) UPlayerLobbyWidget* GetLobbyWidget();
	UFUNCTION(BlueprintCallable) void CloseEscapeMenu();
	UFUNCTION(BlueprintCallable) void ToggleScoreboard();
	UFUNCTION(BlueprintCallable) void OpenEscapeMenu();
	UFUNCTION(BlueprintCallable) void OpenTraderUI();
	UFUNCTION(BlueprintCallable) void CloseTraderUI();
	UPROPERTY(VisibleAnywhere, Category="UI|State") bool       IsInChat          = false;
	UPROPERTY(BlueprintAssignable) FOnGameStatusChanged OnGameStatusChanged;
	UPROPERTY(BlueprintAssignable) FOnPlayerPointsReceived OnPlayerPointsReceivedDelegate;

	// AHUD
	virtual void BeginPlay() override;
	virtual void DrawHUD() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

protected:
	// widget classes (edit paths or set in BP)
	UPROPERTY(EditDefaultsOnly, Category="UI|Classes") TSubclassOf<UPlayerHUDWidget>        PlayerHUDWidgetClass;
	UPROPERTY(EditDefaultsOnly, Category="UI|Classes") TSubclassOf<UPlayerLobbyWidget>      PlayerLobbyWidgetClass;
	UPROPERTY(EditDefaultsOnly, Category="UI|Classes") TSubclassOf<UPlayerTraderWidget>     PlayerTraderWidgetClass;
	UPROPERTY(EditDefaultsOnly, Category="UI|Classes") TSubclassOf<UPlayerEndScreen>        PlayerEndScreenClass;
	UPROPERTY(EditDefaultsOnly, Category="UI|Classes") TSubclassOf<UPlayerTravelWidget>     PlayerTravelWidgetClass;
	UPROPERTY(EditDefaultsOnly, Category="UI|Classes") TSubclassOf<UPlayerEscapeMenu>       PlayerEscapeWidgetClass;
	UPROPERTY(EditDefaultsOnly, Category="UI|Classes") TSubclassOf<UPlayerScoreboardWidget> PlayerScoreboardWidgetClass;

	// widget instances (UPROPERTY so GC can see them)
	UPROPERTY() UPlayerHUDWidget*        PlayerHUDWidget        = nullptr;
	UPROPERTY() UPlayerLobbyWidget*      PlayerLobbyWidget      = nullptr;
	UPROPERTY() UPlayerTraderWidget*     PlayerTraderWidget     = nullptr;
	UPROPERTY() UPlayerEndScreen*        PlayerEndScreenWidget  = nullptr;
	UPROPERTY() UPlayerTravelWidget*     PlayerTravelWidget     = nullptr;
	UPROPERTY() UPlayerEscapeMenu*       PlayerEscapeWidget     = nullptr;
	UPROPERTY() UPlayerScoreboardWidget* PlayerScoreboardWidget = nullptr;

	// assets
	UPROPERTY(EditDefaultsOnly, Category="UI|Assets") UTexture2D* CrosshairTex = nullptr;
	UPROPERTY(EditDefaultsOnly, Category="UI|Assets") UFont*      WaitingFont  = nullptr;

	// state
	UPROPERTY(VisibleAnywhere, Category="UI|State") bool       FirstTimeGameStatusChange = false;
	UPROPERTY(VisibleAnywhere, Category="UI|State") EGameStatus CurrentGameStatus = EGameStatus::ELOBBY;
	UPROPERTY(VisibleAnywhere, Category="UI|State") bool       bIsScoreboardOpen = false;

	UPROPERTY(VisibleAnywhere, Category="UI|State") bool       bIsTraderUIOpen   = false;

private:
	void RemoveMyWidgetsFromViewport();
};
