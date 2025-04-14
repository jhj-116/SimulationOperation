// Fill out your copyright notice in the Description page of Project Settings.


#include "FActionsMessageHandler.h"
#include "Widgets/SViewport.h"
#include <windowsx.h>

TSharedPtr<FActionsMessageHandler> FActionsMessageHandler::ActionsMessageHandler = nullptr;

bool FActionsMessageHandler::ProcessMessage(HWND hwnd, uint32 msg, WPARAM wParam, LPARAM lParam, int32& OutResult)
{
	// mouse position
	POINT CursorPoint;
	CursorPoint.x = GET_X_LPARAM(lParam);
	CursorPoint.y = GET_Y_LPARAM(lParam);
	ClientToScreen(hwnd, &CursorPoint);
	const FVector2D CursorPos(CursorPoint.x, CursorPoint.y);
	FVector2D ViewportSize;
	DWORD pid;
	GetWindowThreadProcessId(hwnd,&pid);
	const FVector2D ScreenLocation = ConvertFullScreenPositionToViewport(CursorPos, ViewportSize);
	UE_LOG(LogTemp,Warning,TEXT("HWD :0x%p,%u,%u,%u"),hwnd,pid,msg,WM_CHAR);
	// do not record hot keys
	if (UActionsRecorderLibrary::IsHotKey(static_cast<int32>(wParam)))
	{
		return false;
	}
	
	switch (msg)
	{
	// 符号键
	case WM_CHAR:
		{
			// Character code is stored in WPARAM
			const TCHAR Character = IntCastChecked<TCHAR>(wParam);

			// LPARAM bit 30 will be ZERO for new presses, or ONE if this is a repeat
			const bool bIsRepeat = ( lParam & 0x40000000 ) != 0;

			bool bIsDown = true;
			const int32 Win32Key = IntCastChecked<int32>(wParam);
			
			FActionRecordData ActionRecordData = {FPlatformTime::Seconds(), ScreenLocation, CursorPos,
				ViewportSize, bIsDown, bIsRepeat, Win32Key, 0,EButtonType::KeyChar};
			UActionsRecorderLibrary::RecordAction(ActionRecordData, Controller);
			GEngine->AddOnScreenDebugMessage(-1,5,FColor::Red,TEXT("Start player"));
			//UE_LOG(LogTemp, Warning, TEXT("[WM_CHAR]: %c, Repeat: %d, Code:%d"), Character, bIsRepeat, Win32Key);
		}
		break;
	// Key down
	case WM_SYSKEYDOWN:
	case WM_KEYDOWN:
		{
			// Character code is stored in WPARAM
			const int32 Win32Key = IntCastChecked<int32>(wParam);

			// The actual key to use.  Some keys will be translated into other keys. 
			// I.E VK_CONTROL will be translated to either VK_LCONTROL or VK_RCONTROL as these
			// keys are never sent on their own
			int32 ActualKey = Win32Key;

			// LPARAM bit 30 will be ZERO for new presses, or ONE if this is a repeat
			bool bIsRepeat = ( lParam & 0x40000000 ) != 0;

			switch( Win32Key )
			{
			case VK_MENU:
				// Differentiate between left and right alt
				if( (lParam & 0x1000000) == 0 )
				{
					ActualKey = VK_LMENU;
					bIsRepeat = ModifierKeyState[EModifierKey::LeftAlt];
					ModifierKeyState[EModifierKey::LeftAlt] = true;
				}
				else
				{
					ActualKey = VK_RMENU;
					bIsRepeat = ModifierKeyState[EModifierKey::RightAlt];
					ModifierKeyState[EModifierKey::RightAlt] = true;
				}
				break;
			case VK_CONTROL:
				// Differentiate between left and right control
				if( (lParam & 0x1000000) == 0 )
				{
					ActualKey = VK_LCONTROL;
					bIsRepeat = ModifierKeyState[EModifierKey::LeftControl];
					ModifierKeyState[EModifierKey::LeftControl] = true;
				}
				else
				{
					ActualKey = VK_RCONTROL;
					bIsRepeat = ModifierKeyState[EModifierKey::RightControl];
					ModifierKeyState[EModifierKey::RightControl] = true;
				}
				break;
			case VK_SHIFT:
				// Differentiate between left and right shift
				ActualKey = MapVirtualKey( (lParam & 0x00ff0000) >> 16, MAPVK_VSC_TO_VK_EX);
				if (ActualKey == VK_LSHIFT)
				{
					bIsRepeat = ModifierKeyState[EModifierKey::LeftShift];
					ModifierKeyState[EModifierKey::LeftShift] = true;
				}
				else
				{
					bIsRepeat = ModifierKeyState[EModifierKey::RightShift];
					ModifierKeyState[EModifierKey::RightShift] = true;
				}
				break;
			case VK_CAPITAL:
				ModifierKeyState[EModifierKey::CapsLock] = (::GetKeyState(VK_CAPITAL) & 0x0001) != 0;
				break;
			default:
				// No translation needed
				break;
			}

			// Get the character code from the virtual key pressed.  If 0, no translation from virtual key to character exists
			uint32 CharCode = ::MapVirtualKey( Win32Key, MAPVK_VK_TO_CHAR );

			// 记录键盘非符号键
			bool bIsDown = true;
			FActionRecordData ActionRecordData = {FPlatformTime::Seconds(), ScreenLocation, CursorPos,ViewportSize, bIsDown, bIsRepeat, ActualKey, 0,EButtonType::KeyDown};
			UActionsRecorderLibrary::RecordAction(ActionRecordData, Controller);

			UE_LOG(LogTemp, Warning, TEXT("[WM_KEYDOWN]: Key:%d, CharCode:%d Repeat:%d"), ActualKey, CharCode, bIsRepeat);
		}
		break;

	// Key up
	case WM_SYSKEYUP:
	case WM_KEYUP:
		{
			// Character code is stored in WPARAM
			int32 Win32Key = IntCastChecked<int32>(wParam);

			// The actual key to use.  Some keys will be translated into other keys. 
			// I.E VK_CONTROL will be translated to either VK_LCONTROL or VK_RCONTROL as these
			// keys are never sent on their own
			int32 ActualKey = Win32Key;

			bool bModifierKeyReleased = false;
			switch( Win32Key )
			{
			case VK_MENU:
				// Differentiate between left and right alt
				if( (lParam & 0x1000000) == 0 )
				{
					ActualKey = VK_LMENU;
					ModifierKeyState[EModifierKey::LeftAlt] = false;
				}
				else
				{
					ActualKey = VK_RMENU;
					ModifierKeyState[EModifierKey::RightAlt] = false;
				}
				break;
			case VK_CONTROL:
				// Differentiate between left and right control
				if( (lParam & 0x1000000) == 0 )
				{
					ActualKey = VK_LCONTROL;
					ModifierKeyState[EModifierKey::LeftControl] = false;
				}
				else
				{
					ActualKey = VK_RCONTROL;
					ModifierKeyState[EModifierKey::RightControl] = false;
				}
				break;
			case VK_SHIFT:
				// Differentiate between left and right shift
				ActualKey = MapVirtualKey( (lParam & 0x00ff0000) >> 16, MAPVK_VSC_TO_VK_EX);
				if (ActualKey == VK_LSHIFT)
				{
					ModifierKeyState[EModifierKey::LeftShift] = false;
				}
				else
				{
					ModifierKeyState[EModifierKey::RightShift] = false;
				}
				break;
			case VK_CAPITAL:
				ModifierKeyState[EModifierKey::CapsLock] = (::GetKeyState(VK_CAPITAL) & 0x0001) != 0;
				break;
			default:
				// No translation needed
				break;
			}

			// Get the character code from the virtual key pressed.  If 0, no translation from virtual key to character exists
			uint32 CharCode = ::MapVirtualKey( Win32Key, MAPVK_VK_TO_CHAR );

			// Key up events are never repeats
			const bool bIsRepeat = false;
			const bool bIsDown = false;
			FActionRecordData ActionRecordData = {FPlatformTime::Seconds(), ScreenLocation, CursorPos,
				ViewportSize, bIsDown, bIsRepeat, ActualKey, 0,EButtonType::KeyUp};
			UActionsRecorderLibrary::RecordAction(ActionRecordData, Controller);
		//	UE_LOG(LogTemp, Warning, TEXT("[WM_KEYUP]: Key:%d, CharCode:%d Repeat:%d"), ActualKey, CharCode, bIsRepeat);
		}
		break;

	// Mouse Button Down
	case WM_LBUTTONDBLCLK:
	case WM_LBUTTONDOWN:
	case WM_MBUTTONDBLCLK:
	case WM_MBUTTONDOWN:
	case WM_RBUTTONDBLCLK:
	case WM_RBUTTONDOWN:
	case WM_XBUTTONDBLCLK:
	case WM_XBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_MBUTTONUP:
	case WM_RBUTTONUP:
	case WM_XBUTTONUP:
		{
			EButtonType MouseButton = EButtonType::Invalid;
			FString ButtonName = "None";
			bool bDoubleClick = false;
			bool bMouseUp = false;
			switch (msg)
			{
			case WM_LBUTTONDBLCLK:
				bDoubleClick = true;
				MouseButton = EButtonType::Left;
				ButtonName = "Left";
				break;
			case WM_LBUTTONUP:
				bMouseUp = true;
				MouseButton = EButtonType::Left;
				ButtonName = "Left";
				break;
			case WM_LBUTTONDOWN:
				MouseButton = EButtonType::Left;
				ButtonName = "Left";
				break;
			case WM_MBUTTONDBLCLK:
				bDoubleClick = true;
				MouseButton = EButtonType::Middle;
				ButtonName = "Middle";
				break;
			case WM_MBUTTONUP:
				bMouseUp = true;
				MouseButton = EButtonType::Middle;
				ButtonName = "Middle";
				break;
			case WM_MBUTTONDOWN:
				MouseButton = EButtonType::Middle;
				ButtonName = "Middle";
				break;
			case WM_RBUTTONDBLCLK:
				bDoubleClick = true;
				MouseButton = EButtonType::Right;
				ButtonName = "Right";
				break;
			case WM_RBUTTONUP:
				bMouseUp = true;
				MouseButton = EButtonType::Right;
				ButtonName = "Right";
				break;
			case WM_RBUTTONDOWN:
				MouseButton = EButtonType::Right;
				ButtonName = "Right";
				break;
			case WM_XBUTTONDBLCLK:
				bDoubleClick = true;
				MouseButton = (HIWORD(wParam) & XBUTTON1) ? EButtonType::Thumb01 : EButtonType::Thumb02;
				break;
			case WM_XBUTTONUP:
				bMouseUp = true;
				MouseButton = (HIWORD(wParam) & XBUTTON1) ? EButtonType::Thumb01 : EButtonType::Thumb02;
				break;
			case WM_XBUTTONDOWN:
				MouseButton = (HIWORD(wParam) & XBUTTON1) ? EButtonType::Thumb01 : EButtonType::Thumb02;
				break;
			default:
				check(0);
			}

			// 记录鼠标点击数据
			FActionRecordData ActionRecordData = {FPlatformTime::Seconds(), ScreenLocation, CursorPos,
				ViewportSize, !bMouseUp, false, UActionsRecorderLibrary::MOUSE_CODE, 0,MouseButton};
			UActionsRecorderLibrary::RecordAction(ActionRecordData, Controller);
			UE_LOG(LogTemp, Warning, TEXT("[Mouse Button Down] Screen: (%f,%f) -> Full: (%f,%f), Down: %d, double:%d"),
				ScreenLocation.X, ScreenLocation.Y,
				CursorPos.X, CursorPos.Y, !bMouseUp, bDoubleClick);
			
		}
		break;
	case WM_MOUSEMOVE:
		{
			// don't move
			if (LastMousePosition.Equals(ScreenLocation))
			{
				break;
			}

			// first move
			if (LastMousePosition.X < 0 || LastMousePosition.Y < 0)
			{
				LastMousePosition = ScreenLocation;	
			}

			bool bIsDown = false, bIsRepeat = false;
			FActionRecordData ActionRecordData = {FPlatformTime::Seconds(), ScreenLocation, CursorPos, ViewportSize,

			bIsDown, bIsRepeat, UActionsRecorderLibrary::MOUSE_CODE, 0,EButtonType::MouseMove,
				UActionsRecorderLibrary::GetControllerRotation(Controller)};
			UActionsRecorderLibrary::RecordAction(ActionRecordData, Controller);
			LastMousePosition = ScreenLocation;

			UE_LOG(LogTemp, Warning, TEXT("[WM_MOUSEMOVE] screen:(%f,%f)->full:(%f,%f)"),
			ScreenLocation.X, ScreenLocation.Y,
			CursorPos.X, CursorPos.Y);
		}
		break;
	// Mouse Wheel
	case WM_MOUSEWHEEL:
		{
			const float SpinFactor = 1 / 120.0f;
			const SHORT WheelDelta = GET_WHEEL_DELTA_WPARAM(wParam);

			bool bIsDown = false, bIsRepeat = false;
			FActionRecordData ActionRecordData = {FPlatformTime::Seconds(), ScreenLocation, CursorPos, ViewportSize,
			bIsDown, bIsRepeat, UActionsRecorderLibrary::MOUSE_CODE, static_cast<float>( WheelDelta )*SpinFactor,EButtonType::MouseWheel};
			UActionsRecorderLibrary::RecordAction(ActionRecordData,Controller);

			LastMousePosition = ScreenLocation;
		}
		break;
	case WM_USER:
		{
			//GEngine->AddOnScreenDebugMessage(-1,5,FColor::Red,TEXT("Start player"));
			if(!ReceivedPythonCommand)
			{
				ReceivedPythonCommand=true;
				GEngine->AddOnScreenDebugMessage(-1,5,FColor::Red,TEXT("Start player"));
				UActionsRecorderLibrary::PlayRecord();
			}
			
		}
	break;
	default:
		break;
	}

     /*if(msg==WM_COPYDATA)
     {
	     COPYDATASTRUCT*Copydata=(COPYDATASTRUCT*)lParam;
     	UE_LOG(LogTemp,Warning,TEXT("Data %s"),(char*)Copydata->lpData);
     }*/
	
	return false;
}

void FActionsMessageHandler::RegisterActionMessageHandler(TSharedPtr<SWindow> TargetWindow, APlayerController* PlayerController)
{
	if (ActionsMessageHandler == nullptr)
	{
		ActionsMessageHandler = MakeShareable(new FActionsMessageHandler());
		ActionsMessageHandler->TargetWindow = TargetWindow;
		ActionsMessageHandler->Controller = PlayerController;
		FMemory::Memzero(ActionsMessageHandler->ModifierKeyState, EModifierKey::Count);
	}
	
	if (FSlateApplication::Get().GetPlatformApplication().IsValid())
	{
		if (FWindowsApplication* Application =
			static_cast<FWindowsApplication*>(FSlateApplication::Get().GetPlatformApplication().Get()))
		{
			Application->AddMessageHandler(*ActionsMessageHandler);
		}
	}
}

void FActionsMessageHandler::UnregisterActionMessageHandler()
{
	if (!ActionsMessageHandler.IsValid()) return;
	
	if (FSlateApplication::Get().GetPlatformApplication().IsValid())
	{
		if (FWindowsApplication* Application =
			static_cast<FWindowsApplication*>(FSlateApplication::Get().GetPlatformApplication().Get()))
		{
			Application->RemoveMessageHandler(*ActionsMessageHandler);
		}
	}
}

FIntPoint FActionsMessageHandler::ConvertFullScreenPositionToViewport(const FVector2D& ScreenLocation,
	FVector2D& OutViewportSize)
{
	FIntPoint OutVector((int32)ScreenLocation.X, (int32)ScreenLocation.Y);
	
	if (TSharedPtr<SWindow> ApplicationWindow = TargetWindow.Pin())
	{
		TSharedPtr<ISlateViewport> Viewport = ApplicationWindow->GetViewport();
		
		if (Viewport.IsValid() && Viewport->GetWidget().IsValid())
		{
			TSharedPtr<SWidget> ViewportWidget = Viewport->GetWidget().Pin();
			FVector2D ViewportLocation(ViewportWidget->GetCachedGeometry().AbsolutePosition.X,
				ViewportWidget->GetCachedGeometry().AbsolutePosition.Y);

			OutViewportSize = Viewport->GetSize();
			
			FVector2D OutTemp = ScreenLocation - ViewportLocation;
			OutVector = FIntPoint((int32)OutTemp.X, (int32)OutTemp.Y);
		}
	}

	return OutVector;
}