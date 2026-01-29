// Copyright (c) 2018 - 2026 Marc Fraedrich
// Horde Template V2 (Native) - A cooperative zombie survival game framework
// Licensed under the MIT License

#include "EndGameTrigger.h"
#include "Character/HordeBaseCharacter.h"
#include "Gameplay/HordeGameState.h"

/**
 * @file EndGameTrigger.cpp
 * @brief Implementation of collision trigger that ends the game on player overlap
 * @author Marc Fraedrich
 */

/**
 * Constructor for AEndGameTrigger
 *
 * @param
 * @return
 */
AEndGameTrigger::AEndGameTrigger()
{
	SetReplicates(true);
	PrimaryActorTick.bCanEverTick = false;
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	BoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("Player Collision"));
	BoxComponent->SetupAttachment(RootComponent);
	BoxComponent->SetRelativeScale3D(FVector(2.f, 2.f, 2.f));

	BoxComponent->OnComponentBeginOverlap.AddDynamic(this, &AEndGameTrigger::OnColide);

}

/** ( Delegate Binding )
 * Triggers End Game inside the Game State if Player Enters.
 *
 * @param OverlappedComponent, OtherActor,  OtherComponent, OtherBodyIndex, bFromSweep, SweepResult
 * @return void
 */
void AEndGameTrigger::OnColide(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (HasAuthority())
	{
		AHordeBaseCharacter* PLY = Cast<AHordeBaseCharacter>(OtherActor);
		if(PLY)
		{
			AHordeGameState* GS = Cast<AHordeGameState>(GetWorld()->GetGameState());
			if (GS)
			{
				GS->EndGame(false);
			}
		}
	}
}

