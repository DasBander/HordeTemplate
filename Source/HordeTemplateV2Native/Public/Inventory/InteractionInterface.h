// Copyright (c) 2018 - 2026 Marc Fraedrich
// Horde Template V2 (Native) - A cooperative zombie survival game framework
// Licensed under the MIT License

#pragma once

/**
 * @file InteractionInterface.h
 * @brief Interface and data structures for interactive world objects
 * @author Marc Fraedrich
 */

#include "EngineMinimal.h"
#include "Sound/SoundCue.h"
#include "InteractionInterface.generated.h"

USTRUCT(BlueprintType)
struct FInteractionInfo {
	GENERATED_USTRUCT_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
		FText InteractionText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
		bool HideKeyInText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
		float InteractionTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
		USoundCue * InteractionSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
		bool AllowedToInteract;



	FInteractionInfo()
	{
		InteractionText = FText::FromString("None");
		InteractionTime = 0.f;
		InteractionSound = nullptr;
		AllowedToInteract = true;
		HideKeyInText = false;
	}

	FInteractionInfo(FString InInteractionText, float InInteractionTime, bool InAllowedToInteract)
	{
		InteractionText = FText::FromString(InInteractionText);
		InteractionTime = InInteractionTime;
		AllowedToInteract = InAllowedToInteract;
		HideKeyInText = false;
		InteractionSound = nullptr;
	}
};


/**
 * 
 */
UINTERFACE(BlueprintType)
class HORDETEMPLATEV2NATIVE_API UInteractionInterface : public UInterface
{
	GENERATED_UINTERFACE_BODY()
	
};

class HORDETEMPLATEV2NATIVE_API IInteractionInterface
{
	GENERATED_IINTERFACE_BODY()

public:

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
		void Interact(AActor* InteractingOwner);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
		FInteractionInfo GetInteractionInfo();
};
