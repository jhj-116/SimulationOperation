// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Windows/WindowsApplication.h"
#include "ActionsRecorderLibrary.h"

/**
 * 
 */
class ACTIONSRECORDER_API FActionsMessageHandler : public IWindowsMessageHandler
{
	
	static TSharedPtr<FActionsMessageHandler> ActionsMessageHandler;
	
	TWeakPtr<SWindow> TargetWindow;

	APlayerController* Controller;

	FVector2D LastMousePosition = {-1, -1};

	struct EModifierKey
	{
		enum Type
		{
			LeftShift,		// VK_LSHIFT
			RightShift,		// VK_RSHIFT
			LeftControl,	// VK_LCONTROL
			RightControl,	// VK_RCONTROL
			LeftAlt,		// VK_LMENU
			RightAlt,		// VK_RMENU
			CapsLock,		// VK_CAPITAL
			Count,
		};
	};
	
public:
	bool ModifierKeyState[EModifierKey::Count];
	
	// return false means don't handle this message, whitch gets passed on to other handlers
	virtual bool ProcessMessage(HWND hwnd, uint32 msg, WPARAM wParam, LPARAM lParam, int32& OutResult) override;
	
	static void RegisterActionMessageHandler(TSharedPtr<SWindow> TargetWindow, APlayerController* PlayerController);
	
	static void UnregisterActionMessageHandler();

	FIntPoint ConvertFullScreenPositionToViewport(const FVector2D& ScreenLocation, FVector2D& OutViewportSize);

	virtual ~FActionsMessageHandler() {};

	bool ReceivedPythonCommand=false;
};
