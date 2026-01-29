// Copyright (c) 2018 - 2026 Marc Fraedrich
// Horde Template V2 (Native) - A cooperative zombie survival game framework
// Licensed under the MIT License

#include "Door.h"
#include "UObject/ConstructorHelpers.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimBlueprintGeneratedClass.h"

/**
 * @file Door.cpp
 * @brief Implementation of replicated interactable door with animation support
 * @author Marc Fraedrich
 */

/**
 * Constructor for ADoor
 *
 * @param
 * @return
 */
ADoor::ADoor()
{
	SetReplicates(true);
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Scene Root"));
	DoorMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Door Mesh"));
	DoorMesh->SetupAttachment(RootComponent);
	DoorMesh->SetCollisionProfileName("BlockAll");
	const ConstructorHelpers::FObjectFinder<USkeletalMesh> DoorMeshAsset(TEXT("SkeletalMesh'/Game/HordeTemplateBP/Assets/Meshes/Misc/SK_BaseDoor_Blue.SK_BaseDoor_Blue'"));
	if (DoorMeshAsset.Succeeded())
	{
		DoorMesh->SetSkeletalMesh(DoorMeshAsset.Object);
	}

	const ConstructorHelpers::FObjectFinder<UAnimBlueprintGeneratedClass> AnimBlueprint(TEXT("AnimBlueprint'/Game/HordeTemplateBP/Assets/Meshes/Misc/ABP_BaseDoor.ABP_BaseDoor_C'"));
	if (AnimBlueprint.Succeeded())
	{
		DoorMesh->AnimClass = AnimBlueprint.Object;
	}

}

/** ( Overridden )
 * Defines Replicated Props
 *
 * @param
 * @output Lifetime Props as Array.
 * @return void
 */
void ADoor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ADoor, bIsOpen);
}

/** ( Interface )
 * Toggles Open Status by Interaction
 *
 * @param Interacting Character / Actor 
 * @return void
 */
void ADoor::Interact_Implementation(AActor* InteractingOwner)
{
	bIsOpen = !bIsOpen;
}

/** ( Interface )
 * Returns Interaction Info depending if door is closed or open.
 *
 * @param
 * @return Interaction Information
 */
FInteractionInfo ADoor::GetInteractionInfo_Implementation()
{
	return FInteractionInfo((bIsOpen) ? "Close Door" : "Open Door", 2, true);
}
