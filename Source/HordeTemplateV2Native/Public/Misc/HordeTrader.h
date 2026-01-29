// Copyright (c) 2018 - 2026 Marc Fraedrich
// Horde Template V2 (Native) - A cooperative zombie survival game framework
// Licensed under the MIT License

#pragma once

/**
 * @file HordeTrader.h
 * @brief NPC trader actor for buying weapons and items between rounds
 * @author Marc Fraedrich
 */

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameFramework/Actor.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Inventory/InteractionInterface.h"
#include "HordeTrader.generated.h"

USTRUCT(BlueprintType)
struct FTraderSellItem : public FTableRowBase
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trading Item")
		FName ItemID = TEXT("None");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trading Item")
		FText Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trading Item")
		FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trading Item")
		int32 ItemPrice = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trading Item")
		UTexture2D* Icon = nullptr;

	FTraderSellItem() {}

};

UCLASS()
class HORDETEMPLATEV2NATIVE_API AHordeTrader : public AActor, public IInteractionInterface
{
	GENERATED_BODY()
	
public:	
	
	AHordeTrader();

	UFUNCTION(NetMulticast, WithValidation, Reliable, BlueprintCallable, Category = "Trader")
		void PlayWelcome();

protected:


	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, Category = "Trader")
		class USkeletalMeshComponent* TraderMeshComponent; 

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, Category = "Trader")
		class UTextRenderComponent* TraderTextComponent;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
		void Interact(AActor* InteractingOwner);
	virtual void Interact_Implementation(AActor* InteractingOwner) override;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
		FInteractionInfo GetInteractionInfo();
	virtual FInteractionInfo GetInteractionInfo_Implementation() override;



};
