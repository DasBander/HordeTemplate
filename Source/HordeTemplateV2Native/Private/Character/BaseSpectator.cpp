

#include "BaseSpectator.h"
#include "EngineUtils.h"

/**
 * Constructor for ABaseSpectator
 *
 * @param
 * @return
 */
ABaseSpectator::ABaseSpectator() { }


/** ( Virtual; Overridden )
 *	Sets up Key Bindings for Player.
 *
 * @param Player Input Component
 * @return void
 */
void ABaseSpectator::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &ABaseSpectator::ServerFocusPlayer);

}

/** ( Client )
 *	Focuses Player by Player Object.
 *
 * @param Player to Focus
 * @return void
 */
void ABaseSpectator::ClientFocusPlayer_Implementation(AHordeBaseCharacter* Player)
{
	APlayerController* PC = Cast<APlayerController>(GetController());
	// Fixed: Added null check for Player parameter
	if (PC && Player)
	{
		PC->SetViewTargetWithBlend(Player, 1.f, VTBlend_EaseIn, 0.5f, true);
	}
}


/**
 *	Gets random alive player from the World.
 *
 * @param
 * @return Random Alive Player
 */
AHordeBaseCharacter* ABaseSpectator::GetRandomAlivePlayer()
{
	TArray<AHordeBaseCharacter*> AliveCharacter;

	// Fixed: Use TActorIterator instead of TObjectIterator to only iterate actors in current world
	// TObjectIterator iterates ALL objects in memory which could include objects from other worlds/PIE instances
	for (TActorIterator<AHordeBaseCharacter> Itr(GetWorld()); Itr; ++Itr)
	{
		AHordeBaseCharacter* PLY = *Itr;
		if (PLY && !PLY->GetIsDead())
		{
			AliveCharacter.Add(PLY);
		}
	}
	return (AliveCharacter.Num() > 0) ? AliveCharacter[FMath::RandRange(0, AliveCharacter.Num() - 1)] : nullptr;
}


/**	( Server )
 *	Gets Random Alive Player and focuses him on client.
 *
 * @param
 * @return void
 */
void ABaseSpectator::ServerFocusPlayer_Implementation()
{
	AHordeBaseCharacter* PLY = GetRandomAlivePlayer();
	if (PLY)
	{
		ClientFocusPlayer(PLY);
	}
}

bool ABaseSpectator::ServerFocusPlayer_Validate()
{
	return true;
}
