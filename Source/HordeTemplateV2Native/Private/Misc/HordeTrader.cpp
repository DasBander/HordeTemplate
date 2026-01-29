// Copyright (c) 2018 - 2026 Marc Fraedrich
// Horde Template V2 (Native) - A cooperative zombie survival game framework
// Licensed under the MIT License

#include "HordeTrader.h"
#include "HordeTemplateV2Native.h"
#include "Components/SceneComponent.h"
#include "Gameplay/HordeGameState.h"
#include "Gameplay/HordeBaseController.h"
#include "Gameplay/HordeWorldSettings.h"
#include "Kismet/GameplayStatics.h"
#include "Animation/AnimBlueprintGeneratedClass.h"
#include "Animation/AnimInstance.h"
#include "ConstructorHelpers.h"

/**
 * @file HordeTrader.cpp
 * @brief Implementation of NPC trader for purchasing weapons and items
 * @author Marc Fraedrich
 */

/**
 * Constructor for AHordeTrader
 *
 * @param
 * @return
 */
AHordeTrader::AHordeTrader()
{
	PrimaryActorTick.bStartWithTickEnabled = false;
	PrimaryActorTick.bCanEverTick = false;
	SetReplicates(true);
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	TraderMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("TraderMesh"));
	TraderMeshComponent->SetupAttachment(RootComponent);
	const ConstructorHelpers::FObjectFinder<USkeletalMesh> TraderMeshAsset(TEXT("SkeletalMesh'/Game/HordeTemplateBP/Assets/Mannequin/Character/Mesh/SK_Mannequin.SK_Mannequin'"));
	if (TraderMeshAsset.Succeeded())
	{
		TraderMeshComponent->SetSkeletalMesh(TraderMeshAsset.Object);
		TraderMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		TraderMeshComponent->SetCollisionProfileName(TEXT("TraderCollision"));
	}

	// static ConstructorHelpers::FClassFinder<UAnimInstance> AnimBP(
	// 	  TEXT("/Game/HordeTemplateBP/Assets/Mannequin/Animations/ABP_ThirdPerson")
	//   );
	//
	// if (AnimBP.Succeeded() && TraderMeshComponent)
	// {
	// 	TraderMeshComponent->SetAnimInstanceClass(AnimBP.Class);
	// }
	
	
	TraderTextComponent = CreateDefaultSubobject<UTextRenderComponent>(TEXT("Trader Header Text"));
	TraderTextComponent->SetupAttachment(TraderMeshComponent);
	TraderTextComponent->SetRelativeLocation(FVector(-29.f, 0.f, 180.f));
	TraderTextComponent->SetRelativeRotation(FRotator(0.f, 90.f, 0.f).Quaternion());
	TraderTextComponent->SetText(FText::FromString("Trader"));

}

/** ( Multicast )
 * Plays Welcome Sound and Animation on all clients.
 *
 * @param
 * @return void
 */
void AHordeTrader::PlayWelcome_Implementation()
{
	USoundCue* WelcomeSound = ObjectFromPath<USoundCue>(TEXT("SoundCue'/Game/HordeTemplateBP/Assets/Sounds/Trader/A_Trader_Welcome.A_Trader_Welcome'"));
	UAnimMontage* WelcomeAnimation = ObjectFromPath<UAnimMontage>(TEXT("AnimMontage'/Game/HordeTemplateBP/Assets/Mannequin/Animations/A_Trader_Welcome_Montage.A_Trader_Welcome_Montage'"));
	if (WelcomeSound && WelcomeAnimation)
	{
		UGameplayStatics::SpawnSoundAtLocation(GetWorld(), WelcomeSound, TraderMeshComponent->GetComponentLocation());
		if (TraderMeshComponent->GetAnimInstance())
		{
			TraderMeshComponent->GetAnimInstance()->Montage_Play(WelcomeAnimation, 1.f);
		}
		
	}
}

bool AHordeTrader::PlayWelcome_Validate()
{
	return true;
}

/** ( Interface )
 * Opens Trader UI on Interacting Client.
 *
 * @param Interacting Character / Actor
 * @return void
 */
void AHordeTrader::Interact_Implementation(AActor* InteractingOwner)
{
	if (HasAuthority())
	{
		PlayWelcome();
	
		AHordeBaseController* PC = Cast<AHordeBaseController>(InteractingOwner->GetInstigatorController());
		if (PC)
		{
			PC->ClientOpenTraderUI();
		}
	}

}

/** ( Interface )
 * Returns Interaction Info
 *
 * @param
 * @return Interaction Information
 */
FInteractionInfo AHordeTrader::GetInteractionInfo_Implementation()
{
	FInteractionInfo InteractionInfo;
	InteractionInfo.InteractionText = FText::FromString("Talk to Trader");
	InteractionInfo.InteractionTime = 3.f;
	AHordeWorldSettings* WS = Cast<AHordeWorldSettings>(GetWorld()->GetWorldSettings());
 	if (WS)
 	{
 		AHordeGameState* GS = Cast<AHordeGameState>(GetWorld()->GetGameState());
 		if (GS)
 		{
 			InteractionInfo.AllowedToInteract = (WS->MatchMode == EMatchMode::EMatchModeNonLinear) ? GS->IsRoundPaused : true;
 		}
 	}
	
	return InteractionInfo;
}
