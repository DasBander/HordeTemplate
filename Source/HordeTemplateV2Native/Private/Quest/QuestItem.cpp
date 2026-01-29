// Copyright (c) 2018 - 2026 Marc Fraedrich
// Horde Template V2 (Native) - A cooperative zombie survival game framework
// Licensed under the MIT License

#include "Quest/QuestItem.h"
#include "Quest/QuestManager.h"
#include "Character/HordeBaseCharacter.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Net/UnrealNetwork.h"

/**
 * @file QuestItem.cpp
 * @brief Implementation of collectible quest items with visual effects and pickup logic
 * @author Marc Fraedrich
 */

AQuestItem::AQuestItem()
{
	PrimaryActorTick.bCanEverTick = true;
	SetReplicates(true);

	// Create pickup collision
	PickupCollision = CreateDefaultSubobject<USphereComponent>(TEXT("PickupCollision"));
	PickupCollision->SetSphereRadius(50.f);
	PickupCollision->SetCollisionProfileName(TEXT("Trigger"));
	PickupCollision->SetGenerateOverlapEvents(true);
	RootComponent = PickupCollision;

	PickupCollision->OnComponentBeginOverlap.AddDynamic(this, &AQuestItem::OnPickupOverlap);

	// Create mesh component
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetupAttachment(RootComponent);
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Set default mesh
	static ConstructorHelpers::FObjectFinder<UStaticMesh> DefaultMesh(TEXT("StaticMesh'/Engine/BasicShapes/Sphere.Sphere'"));
	if (DefaultMesh.Succeeded())
	{
		MeshComponent->SetStaticMesh(DefaultMesh.Object);
		MeshComponent->SetRelativeScale3D(FVector(0.25f));
	}

	// Set default interaction info
	InteractionInfo.InteractionText = FText::FromString("Pick Up");
	InteractionInfo.InteractionTime = 0.f;
	InteractionInfo.AllowedToInteract = true;
}


void AQuestItem::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AQuestItem, bIsPickedUp);
}


void AQuestItem::BeginPlay()
{
	Super::BeginPlay();

	InitialLocation = GetActorLocation();

	// Apply custom mesh if set
	if (ItemMesh)
	{
		MeshComponent->SetStaticMesh(ItemMesh);
	}

	// Apply custom material if set
	if (ItemMaterial)
	{
		MeshComponent->SetMaterial(0, ItemMaterial);
	}
}


void AQuestItem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsPickedUp)
	{
		return;
	}

	// Rotation effect
	if (RotationSpeed != 0.f)
	{
		FRotator NewRotation = MeshComponent->GetRelativeRotation();
		NewRotation.Yaw += RotationSpeed * DeltaTime;
		MeshComponent->SetRelativeRotation(NewRotation);
	}

	// Bobbing effect
	if (BobAmplitude > 0.f)
	{
		BobTimer += DeltaTime * BobSpeed;
		float BobOffset = FMath::Sin(BobTimer) * BobAmplitude;
		FVector NewLocation = InitialLocation;
		NewLocation.Z += BobOffset;
		SetActorLocation(NewLocation);
	}
}


void AQuestItem::OnPickupOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (bIsPickedUp || bRequireInteraction)
	{
		return;
	}

	AHordeBaseCharacter* Player = Cast<AHordeBaseCharacter>(OtherActor);
	if (Player && !Player->GetIsDead())
	{
		TryPickup(Player);
	}
}


bool AQuestItem::CanBePickedUp(AHordeBaseCharacter* Player) const
{
	if (bIsPickedUp)
	{
		return false;
	}

	if (!Player || Player->GetIsDead())
	{
		return false;
	}

	// Check if required quest is active
	if (RequiredActiveQuest != NAME_None)
	{
		UQuestManager* QuestManager = UQuestManager::GetQuestManager(this);
		if (QuestManager && !QuestManager->IsQuestActive(RequiredActiveQuest))
		{
			return false;
		}
	}

	return true;
}


bool AQuestItem::TryPickup(AHordeBaseCharacter* Player)
{
	if (!HasAuthority())
	{
		return false;
	}

	if (!CanBePickedUp(Player))
	{
		return false;
	}

	ExecutePickup(Player);
	return true;
}


void AQuestItem::Interact_Implementation(AActor* InteractingOwner)
{
	if (!bRequireInteraction)
	{
		return;
	}

	AHordeBaseCharacter* Player = Cast<AHordeBaseCharacter>(InteractingOwner);
	if (Player)
	{
		TryPickup(Player);
	}
}


FInteractionInfo AQuestItem::GetInteractionInfo_Implementation()
{
	// Update interaction info based on current state
	FInteractionInfo Info = InteractionInfo;

	// If already picked up, don't allow interaction
	if (bIsPickedUp)
	{
		Info.AllowedToInteract = false;
	}

	// Use display name if set
	if (!DisplayName.IsEmpty())
	{
		Info.InteractionText = FText::Format(NSLOCTEXT("QuestItem", "PickupFormat", "Pick Up {0}"), DisplayName);
	}

	return Info;
}


void AQuestItem::ExecutePickup(AHordeBaseCharacter* Player)
{
	bIsPickedUp = true;

	// Report to quest manager
	UQuestManager* QuestManager = UQuestManager::GetQuestManager(this);
	if (QuestManager)
	{
		QuestManager->ReportItemCollected(ItemID, ItemCount);
	}

	// Play effects
	Multicast_OnPickedUp();

	// Fire blueprint event
	OnItemPickedUp(Player);

	// Destroy after a short delay (allows effects to play)
	SetLifeSpan(0.5f);
}


void AQuestItem::Multicast_OnPickedUp_Implementation()
{
	// Play pickup sound
	if (PickupSound)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), PickupSound, GetActorLocation());
	}

	// Hide mesh
	MeshComponent->SetVisibility(false);
	PickupCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}
