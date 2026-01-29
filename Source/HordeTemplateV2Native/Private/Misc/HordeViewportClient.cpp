// Copyright (c) 2018 - 2026 Marc Fraedrich
// Horde Template V2 (Native) - A cooperative zombie survival game framework
// Licensed under the MIT License

#include "HordeViewportClient.h"

/**
 * @file HordeViewportClient.cpp
 * @brief Implementation of custom viewport client with widget stack management
 * @author Marc Fraedrich
 */

/** ( Virtual; Overridden )
 * Adds Widgets to the Viewport Content Stack.
 *
 * @param Shared Ref of SWidget and the Z Order
 * @return void
 */
void UHordeViewportClient::AddViewportWidgetContent(TSharedRef<class SWidget> ViewportContent, const int32 ZOrder /*= 0*/)
{

	if (ViewportContentStack.Contains(ViewportContent))
	{
		return;
	}

	ViewportContentStack.AddUnique(ViewportContent);

	Super::AddViewportWidgetContent(ViewportContent, 0);
}

/** ( Virtual; Overridden )
 * Removes Widget Reference from Viewport Content Stack and Hidden Viewport Content Stack
 *
 * @param Shared Ref of SWidget
 * @return void
 */
void UHordeViewportClient::RemoveViewportWidgetContent(TSharedRef<class SWidget> ViewportContent)
{
	ViewportContentStack.Remove(ViewportContent);
	HiddenViewportContentStack.Remove(ViewportContent);

	Super::RemoveViewportWidgetContent(ViewportContent);
}

/** ( Virtual; Overridden )
 * Release Slate Resources when Viewport Client 
 *
 * @param
 * @return void
 */
void UHordeViewportClient::BeginDestroy()
{
	ReleaseSlateResources();

	Super::BeginDestroy();
}

/** ( Virtual; Overridden )
 * Release Slate Resources when Viewport Client Detaches.
 *
 * @param
 * @return void
 */
void UHordeViewportClient::DetachViewportClient()
{
	Super::DetachViewportClient();

	ReleaseSlateResources();
}

/**
 * Clears Viewport Content Stack and Hidden Viewport Content Stack.
 *
 * @param
 * @return void
 */
void UHordeViewportClient::ReleaseSlateResources()
{
	ViewportContentStack.Empty();
	HiddenViewportContentStack.Empty();
}
