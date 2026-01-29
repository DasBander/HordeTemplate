// Copyright (c) 2018 - 2026 Marc Fraedrich
// Horde Template V2 (Native) - A cooperative zombie survival game framework
// Licensed under the MIT License

#pragma once

/**
 * @file GameChat.h
 * @brief In-game chat widget for sending and receiving messages during gameplay
 * @author Marc Fraedrich
 */

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Gameplay/HordeBaseController.h"
#include "GameChat.generated.h"


UCLASS()
class HORDETEMPLATEV2NATIVE_API UGameChat : public UUserWidget
{
	GENERATED_BODY()


public:
	virtual void NativeConstruct() override;
	
	UFUNCTION(BlueprintImplementableEvent, Category = "Chat")
		void OnMessageReceived(const FHordeChatMessage& Message);

	UFUNCTION(BlueprintImplementableEvent, Category = "Chat")
		void OnGameFocusChat();

	UFUNCTION(BlueprintCallable, Category = "Chat")
		void SubmitChatMessage(const FText& Message);

	virtual void NativeDestruct() override;

};
