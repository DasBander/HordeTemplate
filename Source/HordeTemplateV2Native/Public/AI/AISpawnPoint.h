// Copyright (c) 2018 - 2026 Marc Fraedrich
// Horde Template V2 (Native) - A cooperative zombie survival game framework
// Licensed under the MIT License

#pragma once

/**
 * @file AISpawnPoint.h
 * @brief Spawn point actor with collision detection to prevent spawning on occupied locations
 * @author Marc Fraedrich
 */

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BillboardComponent.h"
#include "AISpawnPoint.generated.h"

UCLASS(ClassGroup="Horde AI")
class HORDETEMPLATEV2NATIVE_API AAISpawnPoint : public AActor
{
	GENERATED_BODY()
	
public:	
	AAISpawnPoint();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
		FName PatrolTag;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "AI")
		bool SpawnNotFree = false;


protected:
	// Track number of characters overlapping to properly handle multiple characters
	UPROPERTY()
		int32 CharactersInSpawnCount = 0;
	UFUNCTION()
		void CharacterOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
		void CharacterEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UPROPERTY(VisibleDefaultsOnly, Category = "Components")
		class UBillboardComponent* ActorIcon;

};
