// Copyright (c) 2018 - 2026 Marc Fraedrich
// Horde Template V2 (Native) - A cooperative zombie survival game framework
// Licensed under the MIT License

#pragma once

/**
 * @file Med_VAC.h
 * @brief Medical VAC healing device that restores player health with animation
 * @author Marc Fraedrich
 */

#include "CoreMinimal.h"
#include "Weapons/BaseFirearm.h"
#include "Components/StaticMeshComponent.h"
#include "Med_VAC.generated.h"

/**
 * 
 */
UCLASS()
class HORDETEMPLATEV2NATIVE_API AMed_VAC : public ABaseFirearm
{
	GENERATED_BODY()

public:
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "VAC")
		bool IsHealing = false;
	
protected:
	AMed_VAC();

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Components")
		class UStaticMeshComponent* VACMesh;

	virtual void FireFirearm() override;

	UFUNCTION(NetMulticast, WithValidation, Reliable, BlueprintCallable, Category = "VAC")
		void StartHealing();

	UFUNCTION(Server, WithValidation, Reliable, BlueprintCallable, Category = "VAC")
		void FinishHealing();
};
