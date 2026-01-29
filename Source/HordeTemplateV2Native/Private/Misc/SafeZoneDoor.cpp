// Copyright (c) 2018 - 2026 Marc Fraedrich
// Horde Template V2 (Native) - A cooperative zombie survival game framework
// Licensed under the MIT License

#include "SafeZoneDoor.h"
#include "UObject/ConstructorHelpers.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimBlueprintGeneratedClass.h"

/**
 * @file SafeZoneDoor.cpp
 * @brief Implementation of one-way animated safe zone door
 * @author Marc Fraedrich
 */

/**
 * Constructor for ASafeZoneDoor
 *
 * @param
 * @return
 */
ASafeZoneDoor::ASafeZoneDoor()
{
	SetReplicates(true);
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Scene Root"));
	DoorMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Door Mesh"));
	DoorMesh->SetupAttachment(RootComponent);
	DoorMesh->SetCollisionProfileName("BlockAll");
	const ConstructorHelpers::FObjectFinder<USkeletalMesh> DoorMeshAsset(TEXT("SkeletalMesh'/Game/HordeTemplateBP/Assets/Meshes/Misc/SK_SafeZoneDoor.SK_SafeZoneDoor'"));
	if (DoorMeshAsset.Succeeded())
	{
		DoorMesh->SetSkeletalMesh(DoorMeshAsset.Object);
	}

	const ConstructorHelpers::FObjectFinder<UAnimBlueprintGeneratedClass> AnimBlueprint(TEXT("AnimBlueprint'/Game/HordeTemplateBP/Assets/Meshes/Misc/ABP_SafeZoneDoor.ABP_SafeZoneDoor_C'"));
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
void ASafeZoneDoor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASafeZoneDoor, bIsOpen);
}

/** ( Interface )
 * Opens door.
 *
 * @param Interacting Actor
 * @return
 */
void ASafeZoneDoor::Interact_Implementation(AActor* InteractingOwner)
{
	if (!bIsOpen)
	{
		bIsOpen = true;
	}
}

/** ( Interface )
 * Returns Interaction Info.
 *
 * @param
 * @return Interaction Information.
 */
FInteractionInfo ASafeZoneDoor::GetInteractionInfo_Implementation()
{
	return FInteractionInfo((bIsOpen) ? "" : "Open Door", 2, true);
}



