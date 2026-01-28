

#include "AISpawnPoint.h"
#include "Components/CapsuleComponent.h"
#include "ConstructorHelpers.h"
#include "GameFramework/Character.h"


/**
 *	Constructor
 *
 * @param
 * @return
 */
AAISpawnPoint::AAISpawnPoint()
{
	PrimaryActorTick.bStartWithTickEnabled = false;
	PrimaryActorTick.bCanEverTick = false;

	RootComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Root"));
	RootComponent->SetRelativeScale3D(FVector(2.f, 2.f, 2.f));

	UCapsuleComponent* Capsule = Cast<UCapsuleComponent>(RootComponent);
	if (Capsule)
	{
		Capsule->SetCapsuleHalfHeight(44.f, true);
		Capsule->SetCapsuleRadius(22.f);
		Capsule->OnComponentBeginOverlap.AddDynamic(this, &AAISpawnPoint::CharacterOverlap);
		Capsule->OnComponentEndOverlap.AddDynamic(this, &AAISpawnPoint::CharacterEndOverlap);
		Capsule->SetCollisionProfileName(FName("SpawnerCollision"));
	}


	ActorIcon = CreateDefaultSubobject<UBillboardComponent>(TEXT("Icon"));
	ActorIcon->SetupAttachment(RootComponent);
	ActorIcon->SetRelativeLocation(FVector(0.f, 0.f, 53.f));

	const ConstructorHelpers::FObjectFinder<UTexture2D> IconAsset(TEXT("Texture2D'/Engine/EditorResources/S_Actor.S_Actor'"));
	if (IconAsset.Succeeded())
	{
		ActorIcon->SetSprite(IconAsset.Object);
	}


}


/**
 *	Overlap with Spawn Collision.
 *	If Character is inside this Collision Spawn wont be free.
 *	Fixed: Use counter to track multiple characters overlapping.
 *
 * @param The Overlapping Component, Actor, Other Component, Other Body Index, FromSweep and the HitResult.
 * @return void
 */
void AAISpawnPoint::CharacterOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor)
	{
		ACharacter* PLY = Cast<ACharacter>(OtherActor);
		if (PLY)
		{
			CharactersInSpawnCount++;
			SpawnNotFree = true;
		}
	}

}


/**
 * If Character leaves free up the Spawn.
 * Fixed: Use counter to properly handle multiple characters - only free spawn when all leave.
 *
 * @param The Overlapping Component, Other Actor, Other Component and Other Body Index
 * @return void
 */
void AAISpawnPoint::CharacterEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor)
	{
		ACharacter* PLY = Cast<ACharacter>(OtherActor);
		if (PLY)
		{
			CharactersInSpawnCount = FMath::Max(0, CharactersInSpawnCount - 1);
			if (CharactersInSpawnCount == 0)
			{
				SpawnNotFree = false;
			}
		}
	}
}

