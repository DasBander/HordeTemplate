

#include "HordeBaseController.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "HordePlayerState.h"
#include "Character/HordeBaseCharacter.h"
#include "HUD/HordeBaseHUD.h"

/** ( Client )
 *	Runs Close Trader UI in HUD Class
 *
 * @param
 * @return void
 */
void AHordeBaseController::ClientCloseTraderUI_Implementation()
{
	AHordeBaseHUD* HUD = Cast<AHordeBaseHUD>(GetHUD());
	if (HUD)
	{
		HUD->CloseTraderUI();
	}
}

/** ( Client )
 *	Plays sound on Owning Client.
 *
 * @param The Sound to play in 2D.
 * @return void
 */
void AHordeBaseController::ClientPlay2DSound_Implementation(USoundCue* Sound)
{
	if (Sound)
	{
		UGameplayStatics::PlaySound2D(GetWorld(), Sound);
	}
}

/**
 * Constructor
 *
 * @param
 * @return
 */
AHordeBaseController::AHordeBaseController()
{
	bAttachToPawn = true;
}

/** ( Client )
 *	Runs OpenTraderUI in HUD Class.
 *
 * @param
 * @return void
 */
void AHordeBaseController::ClientOpenTraderUI_Implementation()
{
	AHordeBaseHUD* HUD = Cast<AHordeBaseHUD>(GetHUD());
	if (HUD)
	{
		HUD->OpenTraderUI();
	}
}

/**
 *	Runs OpenEscapeMenu in HUD Class.
 *
 * @param
 * @return void
 */
void AHordeBaseController::OpenEscapeMenu()
{
	AHordeBaseHUD* HUD = Cast<AHordeBaseHUD>(GetHUD());
	if (HUD)
	{
		HUD->OpenEscapeMenu();
	}
}

/**
 *	Runs ToggleScoreboard in HUD Class.
 *
 * @param
 * @return void
 */
void AHordeBaseController::ToggleScoreboard()
{
	AHordeBaseHUD* HUD = Cast<AHordeBaseHUD>(GetHUD());
	if (HUD)
	{
		HUD->ToggleScoreboard();
	}
}

/** ( Virtual; Overridden )
 *	Sets up the key bindings that should be permanent.
 *
 * @param
 * @return void
 */
void AHordeBaseController::SetupInputComponent()
{
	Super::SetupInputComponent();
	if (InputComponent)
	{
		InputComponent->BindAction("Toggle Chat", IE_Pressed,this, &AHordeBaseController::ToggleChat);
		InputComponent->BindAction("EscapeMenu", IE_Pressed, this, &AHordeBaseController::OpenEscapeMenu);

		InputComponent->BindAction("Toggle Scoreboard", IE_Pressed, this, &AHordeBaseController::ToggleScoreboard);
		InputComponent->BindAction("Toggle Scoreboard", IE_Released, this, &AHordeBaseController::ToggleScoreboard);
	}
}

/**
 *	Drops Current Firearm and Disconnects from Server.
 *
 * @param
 * @return void
 */
void AHordeBaseController::DisconnectFromServer()
{
	AHordeBaseCharacter* PLY = Cast<AHordeBaseCharacter>(GetPawn());
	if (PLY && PLY->GetCurrentFirearm())
	{
		PLY->Inventory->ServerDropItem(PLY->GetCurrentFirearm());

	}
	// Fixed: GettingKicked is a Client RPC which cannot be called from client
	// Instead, just disconnect directly using the engine's built-in function
	UWorld* World = GetWorld();
	if (World)
	{
		UKismetSystemLibrary::QuitGame(World, this, EQuitPreference::Quit, false);
	}
}

/**
 *	Toggles Chat in-game in HUD Class.
 *
 * @param
 * @return void
 */
void AHordeBaseController::ToggleChat()
{
	AHordeBaseHUD* HUD = Cast<AHordeBaseHUD>(GetHUD());
	if (HUD)
	{
		if (!HUD->IsInChat)
		{
			HUD->IsInChat = true;
			bShowMouseCursor = true;
			OnFocusGameChat.Broadcast();
		}
	}
}

/**
 *	Closes Chat and sets input to Game Only.
 *
 * @param
 * @return void
 */
void AHordeBaseController::CloseChat()
{
	AHordeBaseHUD* HUD = Cast<AHordeBaseHUD>(GetHUD());
	if (HUD)
	{
		HUD->IsInChat = false;
		SetInputMode(FInputModeGameOnly());
		bShowMouseCursor = false;
	}
}
