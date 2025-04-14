// Fill out your copyright notice in the Description page of Project Settings.

#include "ActionsRecorderLibrary.h"

#include "JsonObjectConverter.h"
#include "FActionsMessageHandler.h"
#include "Input/HittestGrid.h"
#include "FHandleInputEventThread.h"
#include "Kismet/GameplayStatics.h"

TSharedPtr<FActionsRecordController> FActionsRecordController::Singleton = nullptr;

FActionsRecordController::FActionsRecordController(const TSharedPtr<FGenericApplicationMessageHandler>& InTargetHandler)
	:MessageHandler(InTargetHandler)
{
	
	ActionsRecordFilename = FPaths::ProjectConfigDir() + "/ActionsRecord.json";
	if (!FPaths::FileExists(FPaths::ConvertRelativePathToFull(ActionsRecordFilename)))
	{
		CreateActionsRecordFile();
	}

	ReadActionsRecord();
}

void FActionsRecordController::CreateActionsRecordFile()
{
	TArray<TSharedPtr<FJsonValue>> EmptyArray = {};

	ActionsRecordJsonObject = MakeShareable(new FJsonObject);
	ActionsRecordJsonObject->SetArrayField("ActionsRecord", EmptyArray);

	SaveActionsRecord();
}

TSharedPtr<FActionsRecordController> FActionsRecordController::GetInstance()
{
	if (!Singleton.IsValid())
	{
		// get base message handler
		TSharedPtr<FGenericApplicationMessageHandler> BaseHandler = nullptr;
		if (FSlateApplication::IsInitialized() && FSlateApplication::Get().GetPlatformApplication().IsValid())
		{
			BaseHandler = FSlateApplication::Get().GetPlatformApplication()->GetMessageHandler();
		}
		
		Singleton = TSharedPtr<FActionsRecordController>(new FActionsRecordController(BaseHandler));
	}
	
	return Singleton;
}

void FActionsRecordController::EnqueueAction(FActionRecordData& Action, TWeakPtr<SWindow>& TargetWindow)
{
	ActionQueue.Enqueue({Action, TargetWindow});
}

void FActionsRecordController::EnqueueActionOriginalView(const FVector& ViewOriginal, const FVector& ViewDirection)
{
	ActionsView.Enqueue({ViewOriginal, ViewDirection});
}

void FActionsRecordController::SaveSingleRecord(const FSingleRecord& SingleRecord)
{
	if (!ActionsRecordJsonObject.IsValid())
	{
		ReadActionsRecord();
	}
	
	TArray<TSharedPtr<FJsonValue>> ActionsRecords = ActionsRecordJsonObject->GetArrayField("ActionsRecord");

	TSharedPtr<FJsonObject> SingleRecordJson = MakeShareable(new FJsonObject());
	 FJsonObjectConverter::UStructToJsonObject(FSingleRecord::StaticStruct(), &SingleRecord,
		SingleRecordJson.ToSharedRef(), 0, 0);
	
	ActionsRecords.Add(MakeShareable(new FJsonValueObject(SingleRecordJson)));

	ActionsRecordJsonObject->SetArrayField("ActionsRecord", ActionsRecords);
	SaveActionsRecord();
}

void FActionsRecordController::ReadActionsRecord()
{
	FString JsonContent;
	const FString FullPath = FPaths::ConvertRelativePathToFull(ActionsRecordFilename);

	if (!FFileHelper::LoadFileToString(JsonContent, *FullPath))
	{
		return ;
	}

	TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(JsonContent);
	if (!FJsonSerializer::Deserialize(JsonReader, ActionsRecordJsonObject))
	{
		UE_LOG(LogTemp, Warning, TEXT("ReadLCMFile: Deserialize LCM object failed."));
		return ;
	}
	
}

void FActionsRecordController::SaveActionsRecord()
{
	FString JsonContent;
	const FString FullPath = FPaths::ConvertRelativePathToFull(ActionsRecordFilename);

	TSharedPtr<TJsonWriter<>> JsonWriter = TJsonWriterFactory<>::Create(&JsonContent);

	if (!FJsonSerializer::Serialize(ActionsRecordJsonObject.ToSharedRef(), *JsonWriter))
	{
		UE_LOG(LogTemp, Warning, TEXT("WriteLCMFile: Serialize LCM string failed."));
		return ;
	}

	if (!FFileHelper::SaveStringToFile(JsonContent, *FullPath, FFileHelper::EEncodingOptions::ForceUTF8))
	{
		return ;
	}
	
}

void FActionsRecordController::PlayActionsRecord(TWeakPtr<SWindow> TargetWindow, int32 RecordIndex, EPlayRecordMode PlayMode)
{
	if (!TargetWindow.IsValid())
	{
		return;
	}
	
	if (!ActionsRecordJsonObject.IsValid())
	{
		ReadActionsRecord();
	}
	
	
	// stop playing before
	StopPlayingThread();

	// start new thread to send action record
	PlayingThread = MakeUnique<FHandleInputEventThread>(this, TargetWindow);
	PlayingThread->Start();
}

void FActionsRecordController::SetupController(const FActionView& ActionView) const
{
	if (PlayerController == nullptr) return;

	PlayerController->SetInitialLocationAndRotation(ActionView.ViewOriginal, ActionView.ViewDirection.Rotation());

	
}

void FActionsRecordController::StopPlayingThread()
{
	if (PlayingThread.IsValid())
	{
		PlayingThread->Stop();
		PlayingThread.Release();
	}
}

void FActionsRecordController::DoAction(FActionRecordData& Action,TWeakPtr<SWindow>& TargetWindow) const
{
	TSharedPtr<ISlateViewport> Viewport = TargetWindow.Pin()->GetViewport();
	float XRatio = 1.0f, YRatio = 1.0f;
	if (Viewport.IsValid())
	{
		XRatio = Viewport->GetSize().X * 1.0f / Action.ViewportSize.X;
		YRatio = Viewport->GetSize().Y * 1.0f / Action.ViewportSize.Y;
	}
	
	FSlateApplication& SlateApplication = FSlateApplication::Get();

	// 鼠标位置有效时，才设置鼠标位置
	if (Action.AbsoluteLocation.X >= 0 && Action.AbsoluteLocation.Y >= 0 &&
		SlateApplication.Get().GetPlatformApplication().IsValid() &&
		SlateApplication.Get().GetPlatformApplication()->Cursor.IsValid())
	{
		SlateApplication.Get().OnCursorSet();
		SlateApplication.Get().GetPlatformApplication()->Cursor->SetPosition(Action.AbsoluteLocation.X * XRatio,
			Action.AbsoluteLocation.Y * YRatio);

		//UE_LOG(LogTemp, Warning, TEXT("Set Mouse at: %s"), *Action.AbsoluteLocation.ToString());
	}
	
	FKey Key = UActionsRecorderLibrary::TranslateMouseButtonToKey(Action.ButtonType);
	
	switch (Action.ButtonType)
	{
	case EButtonType::Left:
	case EButtonType::Right:
		{
			if (Action.IsDown)
			{
				FPointerEvent MouseEvent(
					SlateApplication.GetUserIndexForMouse(),
					FSlateApplicationBase::CursorPointerIndex,
					Action.AbsoluteLocation,
					SlateApplication.GetLastCursorPos(),
					SlateApplication.GetPressedMouseButtons(),
					Key,
					0,
					SlateApplication.GetPlatformApplication()->GetModifierKeys());
				UActionsRecorderLibrary::OnMouseDown(MouseEvent, SlateApplication);
			}
			else
			{
				FPointerEvent MouseEvent(
					SlateApplication.GetUserIndexForMouse(),
					FSlateApplicationBase::CursorPointerIndex,
					SlateApplication.GetCursorPos(),
					SlateApplication.GetLastCursorPos(),
					SlateApplication.GetPressedMouseButtons(),
					Key,
					0,
					SlateApplication.GetPlatformApplication()->GetModifierKeys());
				UActionsRecorderLibrary::OnMouseUp(MouseEvent, SlateApplication);
			}

		}
		break;
	case EButtonType::MouseMove:
		{

			
			double DeltaX = Action.AbsoluteLocation.X - SlateApplication.GetLastCursorPos().X;
			double DeltaY = Action.AbsoluteLocation.Y - SlateApplication.GetLastCursorPos().Y;
			DeltaX = FMath::Clamp(DeltaX, -1, 1);
			DeltaY = FMath::Clamp(DeltaY, -1, 1);

			FPointerEvent MouseMoveEvent(
				SlateApplication.GetUserIndexForMouse(),
				FSlateApplication::CursorPointerIndex,
				Action.AbsoluteLocation,
				SlateApplication.GetLastCursorPos(),
				FVector2D(DeltaX, DeltaY), 
				SlateApplication.GetPressedMouseButtons(),
				SlateApplication.GetPlatformApplication()->GetModifierKeys()
			);
			SlateApplication.ProcessMouseMoveEvent(MouseMoveEvent);
			// 解决移动中视角不一致的问题暂定办法
			if (PlayerController)
			{
				PlayerController->SetControlRotation(Action.ViewDirection.Rotation());
			}
		}
		break;

	case EButtonType::MouseWheel:
		{
			MessageHandler->OnMouseWheel(Action.WheelDelta);//,Action.AbsoluteLocation
		}
		break;
	case EButtonType::KeyChar:
		{
			const TCHAR Character = IntCastChecked<TCHAR>(Action.KeyCode);
			MessageHandler->OnKeyChar(Character, Action.IsRepeat);
		}
		break;
	case EButtonType::KeyDown:
		{
			MessageHandler->OnKeyDown(Action.KeyCode, Action.KeyCode, Action.IsRepeat);
		}
		break;
	case EButtonType::KeyUp:
		{
			MessageHandler->OnKeyUp(Action.KeyCode, Action.KeyCode, Action.IsRepeat);
		}
		break;
	default:
		{};break;
	}
}

FActionsRecordController::~FActionsRecordController()
{
}

void FActionsRecordController::Tick(float DeltaTime)
{
	/*FActionView ActionView;
	while (ActionsView.Dequeue(ActionView))
	{
		SetupController(ActionView);
	}*/
	
	// 轮询动作消息队列
	FActionParam ActionParam;
	while (ActionQueue.Dequeue(ActionParam))
	{
		DoAction(ActionParam.ActionRecordData, ActionParam.TargetWindow);
	}
}

void FActionsRecordController::SendControllerEvents()
{
	// pass	
}

void FActionsRecordController::SetMessageHandler(const TSharedRef<FGenericApplicationMessageHandler>& InMessageHandler)
{
	MessageHandler = InMessageHandler;
}

bool FActionsRecordController::Exec(UWorld* InWorld, const TCHAR* Cmd, FOutputDevice& Ar)
{
	return GEngine->Exec(InWorld, Cmd, Ar);
}

void FActionsRecordController::SetChannelValues(int32 ControllerId, const FForceFeedbackValues& values)
{
	// pass
}

void FActionsRecordController::SetChannelValue(int32 ControllerId, FForceFeedbackChannelType ChannelType, float Value)
{
	// pass
}

//////////////////////// function library ////////////////////
TSharedPtr<FSingleRecord> UActionsRecorderLibrary::SingleRecord = nullptr;

void UActionsRecorderLibrary::RecordAction(FActionRecordData ActionRecordData, APlayerController* Controller)
{
	if (!SingleRecord.IsValid())
	{
		FString Timestamp = FGenericPlatformTime::StrTimestamp();
		FVector Location = FVector::ZeroVector, ViewDirection = FVector::ZeroVector;
		
		if (Controller != nullptr && Controller->PlayerCameraManager)
		{
			Location=Controller->PlayerCameraManager->GetCameraLocation();
			ViewDirection = GetControllerRotation(Controller);
		}
		SingleRecord = MakeShareable(new FSingleRecord(Timestamp, ViewDirection, Location));
	}
	
	SingleRecord->ActionRecordData.Add(ActionRecordData);
}

void UActionsRecorderLibrary::StartRecord(APlayerController* PlayerController)
{
	if (GEngine && GEngine->GameViewport)
	{
		// listen message handler
		FActionsMessageHandler::RegisterActionMessageHandler(GEngine->GameViewport->GetWindow(),
			PlayerController);
	}
}


void UActionsRecorderLibrary::StopRecord()
{
	// unregister message handler
	FActionsMessageHandler::UnregisterActionMessageHandler();
	
	if (SingleRecord.IsValid())
	{
		FActionsRecordController::GetInstance()->SaveSingleRecord(*SingleRecord);
	}

	SingleRecord.Reset();
}

void UActionsRecorderLibrary::PlayRecord()
{
	int32 RecordIndex = 0;EPlayRecordMode PlayMode = EPlayRecordMode::PRM_Single;
	if (GEngine && GEngine->GameViewport)
	{
		FActionsRecordController::GetInstance()->PlayActionsRecord(GEngine->GameViewport->GetWindow(), RecordIndex, PlayMode);
	}
}

void UActionsRecorderLibrary::StopPlaying()
{
	FActionsRecordController::GetInstance()->StopPlayingThread();
}

void UActionsRecorderLibrary::SetActionController(APlayerController* PlayerController)
{
	FActionsRecordController::GetInstance()->SetPlayerController(PlayerController);
}

FVector UActionsRecorderLibrary::GetControllerRotation(APlayerController* PlayerController)
{
	if (PlayerController == nullptr) return FVector::ZeroVector;
	
	return PlayerController->GetControlRotation().Vector();
}

FWidgetPath UActionsRecorderLibrary::FindRoutingMessageWidget(const FVector2D& Location, TWeakPtr<SWindow>& TargetWindow,
                                                              FSlateApplication& SlateApplication)
{
	if (TSharedPtr<SWindow> PlaybackWindowPinned = TargetWindow.Pin())
	{
		if (PlaybackWindowPinned->AcceptsInput())
		{
			bool bIgnoreEnabledStatus = false;
			TArray<FWidgetAndPointer> WidgetsAndCursors = PlaybackWindowPinned->GetHittestGrid().GetBubblePath(
				Location, SlateApplication.GetCursorRadius(), bIgnoreEnabledStatus);
			return FWidgetPath(MoveTemp(WidgetsAndCursors));
		}
	}
	return FWidgetPath();
}

FKey UActionsRecorderLibrary::TranslateMouseButtonToKey(const EButtonType Button)
{
	FKey Key = EKeys::Invalid;

	switch (Button)
	{
	case EButtonType::Left:
		Key = EKeys::LeftMouseButton;
		break;
	case EButtonType::Middle:
		Key = EKeys::MiddleMouseButton;
		break;
	case EButtonType::Right:
		Key = EKeys::RightMouseButton;
		break;
	case EButtonType::Thumb01:
		Key = EKeys::ThumbMouseButton;
		break;
	case EButtonType::Thumb02:
		Key = EKeys::ThumbMouseButton2;
		break;
	case EButtonType::KeyChar:
	case EButtonType::KeyDown:
	case EButtonType::KeyUp:
		// 键盘事件
		Key = EKeys::AnyKey;
		break;
    //鼠标滚轮
	case EButtonType::MouseWheel:
		Key = EKeys::MouseWheelAxis;
		break;
		
	case EButtonType::MouseMove:
		// 鼠标移动事件
		Key = EKeys::Invalid;
		break;
	
	default:
		{
			check(0);
		};
	}


	return Key;
}

void UActionsRecorderLibrary::StartLoopRecordMain(FString Name)
{
	TSharedPtr<FJsonObject>RecordJsonObject=MakeShareable(new FJsonObject);
	const FString FilePath=FPaths::ProjectConfigDir()+Name;
	FString JsonContent;
	const FString FullPath = FPaths::ConvertRelativePathToFull(FilePath);

	if (!FFileHelper::LoadFileToString(JsonContent, *FullPath))
	{
		return ;
	}

	TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(JsonContent);
	if (!FJsonSerializer::Deserialize(JsonReader, RecordJsonObject))
	{
		UE_LOG(LogTemp, Warning, TEXT("ReadLCMFile: Deserialize LCM object failed."));
		return ;
	}

	if(RecordJsonObject)
	{
		int32 LoopCount=RecordJsonObject->GetIntegerField("ExecutionCounts");
		TArray<TSharedPtr<FJsonValue>>JsonValues=RecordJsonObject->GetArrayField("ActionFiles");
		for(int i=0;i<LoopCount;++i)
		{
			PlayRecord();
		}
	}
}

void UActionsRecorderLibrary::OnMouseUp(FPointerEvent& Event, FSlateApplication& SlateApplication)
{
	SlateApplication.ProcessMouseButtonUpEvent(Event);
}

void UActionsRecorderLibrary::OnMouseDown( FPointerEvent& Event, FSlateApplication& SlateApplication)
{
	SlateApplication.ProcessMouseButtonDownEvent(nullptr,Event);
}

bool UActionsRecorderLibrary::IsHotKey(int32 KeyCode)
{
	static const TArray<int32> HotKeys = {
		112, 80,  // p, P
		13,       // enter
	};

	return HotKeys.Contains(KeyCode);
}


