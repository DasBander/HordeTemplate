#include "HordeBaseHUD.h"
#include "Engine/Canvas.h"
#include "TextureResource.h"
#include "CanvasItem.h"
#include "Engine/Texture2D.h"
#include "Gameplay/HordeGameState.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Character/HordeBaseCharacter.h"

/**
 * Swaps widgets out and switches input mode depending on the game status. 
 *
 * @param Current Game Status.
 * @return void
 */
void AHordeBaseHUD::GameStatusChanged(uint8 GameStatus)
{
	EGameStatus GS = (EGameStatus)GameStatus;
	CurrentGameStatus = GS;
	FirstTimeGameStatusChange = true;

	// Remove All Widgets from Viewport.
	UGameViewportClient* ViewportClient = GetWorld()->GetGameViewport();
	if (ViewportClient)
	{
		ViewportClient->RemoveAllViewportWidgets();
	}
	
	// Add Widgets to Viewport depending on Game Status.
	switch (GS)
	{
		case EGameStatus::EINGAME:
		{
			if (PlayerHUDWidget)
			{
				PlayerHUDWidget->AddToViewport();
				GetOwningPlayerController()->SetInputMode(FInputModeGameOnly());
				GetOwningPlayerController()->bShowMouseCursor = false;
			}
			break;
		}
		case EGameStatus::ELOBBY:
		{
			if (PlayerLobbyWidget)
			{
				PlayerLobbyWidget->AddToViewport();
				// Fixed: Use stack allocation instead of heap to avoid memory leak
				FInputModeGameAndUI InputMode;
				InputMode.SetWidgetToFocus(PlayerLobbyWidget->TakeWidget());
				GetOwningPlayerController()->SetInputMode(InputMode);
				GetOwningPlayerController()->bShowMouseCursor = true;
			}
			break;
		}
		case EGameStatus::EGAMEOVER:
		{
			if (PlayerEndScreenWidget)
			{
				PlayerEndScreenWidget->AddToViewport();
				GetOwningPlayerController()->SetInputMode(FInputModeGameOnly());
				GetOwningPlayerController()->bShowMouseCursor = false;
			}
			break;
		}
		case EGameStatus::ESERVERTRAVEL:
		{
			// Fixed: Added null check for PlayerTravelWidget
			if (PlayerTravelWidget)
			{
				PlayerTravelWidget->AddToViewport();
				GetOwningPlayerController()->SetInputMode(FInputModeGameOnly());
				GetOwningPlayerController()->bShowMouseCursor = false;
			}
			break;
		}
		default:
			break;
	}
}

/**
 * On Player Points Received Binding.
 *
 * @param The Ponts Type and Points.
 * @return void
 */
void AHordeBaseHUD::OnPlayerPointsReceived(EPointType PointType, int32 Points)
{
	if (PlayerHUDWidget)
	{
		PlayerHUDWidget->OnPointsReceived(PointType, Points);
	}
}

/**
 * Constructor for AHordeBaseHUD
 *
 * @param
 * @return
 */
AHordeBaseHUD::AHordeBaseHUD()
{
	const ConstructorHelpers::FClassFinder<UPlayerHUDWidget> PlayerHUDAsset(WIDGET_HUD_MAIN_UI_PATH);
	if (PlayerHUDAsset.Class)
	{
		PlayerHUDWidgetClass = PlayerHUDAsset.Class;
	}

	const ConstructorHelpers::FClassFinder<UPlayerLobbyWidget> PlayerLobbyAsset(WIDGET_LOBBY_UI_PATH);
	if (PlayerLobbyAsset.Class)
	{
		PlayerLobbyWidgetClass = PlayerLobbyAsset.Class;
	}

	const ConstructorHelpers::FClassFinder<UPlayerTraderWidget> PlayerTraderAsset(WIDGET_TRADER_UI_PATH);
	if (PlayerTraderAsset.Class)
	{
		PlayerTraderWidgetClass = PlayerTraderAsset.Class;
	}

	const ConstructorHelpers::FClassFinder<UPlayerEndScreen> PlayerEndScreenAsset(WIDGET_ENDSCREEN_UI_PATH);
	if (PlayerEndScreenAsset.Class)
	{
		PlayerEndScreenClass = PlayerEndScreenAsset.Class;
	}

	const ConstructorHelpers::FClassFinder<UPlayerTravelWidget> PlayerTravelWidgetAsset(WIDGET_SERVERTRAVEL_UI_PATH);
	if (PlayerTravelWidgetAsset.Class)
	{
		PlayerTravelWidgetClass = PlayerTravelWidgetAsset.Class;
	}

	const ConstructorHelpers::FClassFinder<UPlayerEscapeMenu> PlayerEscapeWidgetAsset(WIDGET_ESCAPEMENU_UI_PATH);
	if (PlayerEscapeWidgetAsset.Class)
	{
		PlayerEscapeWidgetClass = PlayerEscapeWidgetAsset.Class;
	}

	static ConstructorHelpers::FObjectFinder<UTexture2D> CrosshairTexAsset(CROSSHAIR_TEXTURE_PATH);
	if (CrosshairTexAsset.Succeeded())
	{
		CrosshairTex = CrosshairTexAsset.Object;
	}

	static ConstructorHelpers::FClassFinder<UPlayerScoreboardWidget> PlayerScoreboardAsset(WIDGET_SCOREBOARD_PATH);
	if (PlayerScoreboardAsset.Succeeded())
	{
		PlayerScoreboardWidgetClass = PlayerScoreboardAsset.Class;
	}

	static ConstructorHelpers::FObjectFinder<UFont> DF(TEXT("/Engine/EngineFonts/RobotoDistanceField.RobotoDistanceField"));
	if (DF.Succeeded())
	{
		WaitingFont = DF.Object;
	}

	OnGameStatusChanged.AddDynamic(this, &AHordeBaseHUD::GameStatusChanged);
	OnPlayerPointsReceivedDelegate.AddDynamic(this, &AHordeBaseHUD::OnPlayerPointsReceived);
}

/**
 * Returns HUD Widget Object
 *
 * @param
 * @return HUDWidget Object
 */
UPlayerHUDWidget* AHordeBaseHUD::GetHUDWidget()
{
	return (PlayerHUDWidget) ? PlayerHUDWidget : nullptr;
}

/**
 * Returns Lobby Widget Object.
 *
 * @param
 * @return Lobby Widget Object.
 */
UPlayerLobbyWidget* AHordeBaseHUD::GetLobbyWidget()
{
	return (PlayerLobbyWidget) ? PlayerLobbyWidget : nullptr;
}

/**
 * Closes Escape Menu and Resets input mode to game only.
 *
 * @param
 * @return void
 */
void AHordeBaseHUD::CloseEscapeMenu()
{
	if (PlayerEscapeWidget)
	{
		PlayerEscapeWidget->RemoveFromParent();
		GetOwningPlayerController()->SetInputMode(FInputModeGameOnly());
		GetOwningPlayerController()->bShowMouseCursor = false;
	}
}

/**
 * Opens or closes Scoreboard.
 *
 * @param
 * @return void
 */
void AHordeBaseHUD::ToggleScoreboard()
{
	// Fixed: Added null check for PlayerScoreboardWidget
	if (!IsInChat && !bIsTraderUIOpen && CurrentGameStatus == EGameStatus::EINGAME && PlayerScoreboardWidget)
	{
		if (!bIsScoreboardOpen)
		{
			AHordeGameState* GS = Cast<AHordeGameState>(GetWorld()->GetGameState());
			if (GS)
			{
				PlayerScoreboardWidget->UpdatePlayerList(GS->PlayerArray);
			}
			bIsScoreboardOpen = true;
			PlayerScoreboardWidget->AddToViewport(9999);
		}
		else
		{
			PlayerScoreboardWidget->RemoveFromParent();
			bIsScoreboardOpen = false;
		}
	}
}

/**
 * Opens Escape Menu or closes chat or trader ui.
 *
 * @param
 * @return void
 */
void AHordeBaseHUD::OpenEscapeMenu()
{
	// Fixed: Added null check for PlayerEscapeWidget
	if (!bIsScoreboardOpen && CurrentGameStatus != EGameStatus::ELOBBY && PlayerEscapeWidget)
	{
		if (IsInChat)
		{
			GetOwningPlayerController()->SetInputMode(FInputModeGameOnly());
			GetOwningPlayerController()->bShowMouseCursor = false;
			IsInChat = false;
		}

		if (bIsTraderUIOpen)
		{
			CloseTraderUI();
		}

		PlayerEscapeWidget->AddToViewport(9999);
		// Fixed: Use stack allocation instead of heap to avoid memory leak
		FInputModeUIOnly InputMode;
		InputMode.SetWidgetToFocus(PlayerEscapeWidget->TakeWidget());
		GetOwningPlayerController()->SetInputMode(InputMode);
		GetOwningPlayerController()->bShowMouseCursor = true;
	}
}

/**
 * Opens Trader UI
 *
 * @param
 * @return void
 */
void AHordeBaseHUD::OpenTraderUI()
{
	// Fixed: Added null check for PlayerTraderWidget
	if (!IsInChat && !bIsScoreboardOpen && CurrentGameStatus == EGameStatus::EINGAME && !bIsTraderUIOpen && PlayerTraderWidget)
	{
		PlayerTraderWidget->AddToViewport(9999);
		// Fixed: Use stack allocation instead of heap to avoid memory leak
		FInputModeUIOnly InputMode;
		InputMode.SetWidgetToFocus(PlayerTraderWidget->TakeWidget());
		GetOwningPlayerController()->SetInputMode(InputMode);
		GetOwningPlayerController()->bShowMouseCursor = true;
		bIsTraderUIOpen = true;
	}
}

/**
 * Closes Trader UI
 *
 * @param
 * @return void
 */
void AHordeBaseHUD::CloseTraderUI()
{
	if (bIsTraderUIOpen)
	{
		// Fixed: Added null check for PlayerTraderWidget before RemoveFromParent
		if (PlayerTraderWidget)
		{
			PlayerTraderWidget->RemoveFromParent();
		}
		GetOwningPlayerController()->SetInputMode(FInputModeGameOnly());
		GetOwningPlayerController()->bShowMouseCursor = false;
		bIsTraderUIOpen = false;
	}
}

/** ( Virtual; Overridden )
 * Tick
 *
 * @param
 * @return
 */
void AHordeBaseHUD::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}

/** ( Virtual; Overridden )
 * Creates all Widgets on Begin Play and makes Objects Valid.
 *
 * @param
 * @return void
 */
void AHordeBaseHUD::BeginPlay()
{
	Super::BeginPlay();

	PlayerHUDWidget       = CreateWidget<UPlayerHUDWidget>(GetOwningPlayerController(), PlayerHUDWidgetClass);
	PlayerLobbyWidget     = CreateWidget<UPlayerLobbyWidget>(GetOwningPlayerController(), PlayerLobbyWidgetClass);
	PlayerTraderWidget    = CreateWidget<UPlayerTraderWidget>(GetOwningPlayerController(), PlayerTraderWidgetClass);
	PlayerEndScreenWidget = CreateWidget<UPlayerEndScreen>(GetOwningPlayerController(), PlayerEndScreenClass);
	PlayerTravelWidget    = CreateWidget<UPlayerTravelWidget>(GetOwningPlayerController(), PlayerTravelWidgetClass);
	PlayerEscapeWidget    = CreateWidget<UPlayerEscapeMenu>(GetOwningPlayerController(), PlayerEscapeWidgetClass);
	PlayerScoreboardWidget= CreateWidget<UPlayerScoreboardWidget>(GetOwningPlayerController(), PlayerScoreboardWidgetClass);
}

/** ( Virtual; Overridden )
 * Draws Waiting for Server if Player state is not valid or simply client isn't ready.
 * Draws Center Dot.
 * @param
 * @return void
 */
void AHordeBaseHUD::DrawHUD()
{
	Super::DrawHUD();
	if (!Canvas) return;

	const bool bWaiting =
		(!GetOwningPlayerController() ||
		 !GetOwningPlayerController()->PlayerState ||
		 !FirstTimeGameStatusChange);

	// Only draw crosshair when not waiting
	if (!bWaiting && CrosshairTex)
	{
		const FVector2D Center(Canvas->ClipX * 0.5f, Canvas->ClipY * 0.5f);
		const FVector2D CrosshairDrawPosition(
			Center.X - (CrosshairTex->GetSurfaceWidth() * 0.5f),
			Center.Y - (CrosshairTex->GetSurfaceHeight() * 0.5f));

		FCanvasTileItem TileItem(CrosshairDrawPosition, CrosshairTex->GetResource(), FLinearColor::White);
		TileItem.BlendMode = SE_BLEND_Translucent;
		Canvas->DrawItem(TileItem);
	}

	if (bWaiting)
	{
		// 1) Fullscreen black background
		{
			const FVector2D FullSize(Canvas->ClipX, Canvas->ClipY);
			FCanvasTileItem FullscreenItem(FVector2D::ZeroVector, FullSize, FLinearColor(0, 0, 0, 1)); // solid black
			FullscreenItem.BlendMode = SE_BLEND_Opaque; // ensures completely black
			Canvas->DrawItem(FullscreenItem);
		}

		// 2) Centered, crisp text on top
		const float t = GetWorld()->GetRealTimeSeconds(); // or TimeSeconds; RealTimeSeconds ignores pause
		const int32 dots = 1 + (static_cast<int32>(t / 0.5f) % 4);

		static const FString Base = TEXT("Waiting for Server");
		const FString MsgStr = Base + FString::ChrN(dots, '.');
		const FText   Msg    = FText::FromString(MsgStr);
		
		UFont* Font = WaitingFont ? WaitingFont : LoadObject<UFont>(nullptr, TEXT("/Engine/EngineFonts/RobotoDistanceField.RobotoDistanceField"));

		FCanvasTextItem TextItem(FVector2D::ZeroVector, Msg, Font, FLinearColor::White);
		TextItem.bCentreX = true;
		TextItem.bCentreY = true;
		TextItem.Scale    = FVector2D(2.2f, 2.2f);
		TextItem.EnableShadow(FLinearColor::Black);

		const FVector2D Center(Canvas->ClipX * 0.5f, Canvas->ClipY * 0.5f);
		Canvas->DrawItem(TextItem, Center);
	}
}

/** ( Virtual; Overridden )
 * Releases Slate Resources on HUDClass getting destroyed.
 *
 * @param End Play Reason
 * @return void
 */
void AHordeBaseHUD::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Fixed: Added null checks before calling ReleaseSlateResources to prevent crashes
	if (PlayerHUDWidget)
	{
		PlayerHUDWidget->ReleaseSlateResources(true);
		PlayerHUDWidget = nullptr;
	}
	if (PlayerLobbyWidget)
	{
		PlayerLobbyWidget->ReleaseSlateResources(true);
		PlayerLobbyWidget = nullptr;
	}
	if (PlayerTraderWidget)
	{
		PlayerTraderWidget->ReleaseSlateResources(true);
		PlayerTraderWidget = nullptr;
	}
	if (PlayerEndScreenWidget)
	{
		PlayerEndScreenWidget->ReleaseSlateResources(true);
		PlayerEndScreenWidget = nullptr;
	}
	if (PlayerTravelWidget)
	{
		PlayerTravelWidget->ReleaseSlateResources(true);
		PlayerTravelWidget = nullptr;
	}
	if (PlayerScoreboardWidget)
	{
		PlayerScoreboardWidget->ReleaseSlateResources(true);
		PlayerScoreboardWidget = nullptr;
	}
	if (PlayerEscapeWidget)
	{
		PlayerEscapeWidget->ReleaseSlateResources(true);
		PlayerEscapeWidget = nullptr;
	}

	Super::EndPlay(EndPlayReason);
}
