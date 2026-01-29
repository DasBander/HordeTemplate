// Copyright (c) 2018 - 2026 Marc Fraedrich
// Horde Template V2 (Native) - A cooperative zombie survival game framework
// Licensed under the MIT License

#pragma once

/**
 * @file Door.h
 * @brief Replicated interactable door actor with animated open/close states
 * @author Marc Fraedrich
 */

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Inventory/InteractionInterface.h"
#include "HordeTemplateV2Native.h"
#include "Door.generated.h"

UCLASS()
class HORDETEMPLATEV2NATIVE_API ADoor : public AActor, public IInteractionInterface
{
	GENERATED_BODY()
	
public:	
	ADoor();

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Components")
		class USkeletalMeshComponent* DoorMesh;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
		void Interact(AActor* InteractingOwner);
		virtual void Interact_Implementation(AActor* InteractingOwner) override;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
		FInteractionInfo GetInteractionInfo();
		virtual FInteractionInfo GetInteractionInfo_Implementation() override;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Door")
		bool bIsOpen = false;
};
