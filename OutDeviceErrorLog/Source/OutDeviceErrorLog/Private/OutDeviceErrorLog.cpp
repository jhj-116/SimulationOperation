// Copyright Epic Games, Inc. All Rights Reserved.

#include "OutDeviceErrorLog.h"

#include "ActionsRecorderLibrary.h"
#include "FWindowOutDeviceLogError.h"
#include "Misc/MessageDialog.h"

#define LOCTEXT_NAMESPACE "FOutDeviceErrorLogModule"

void FOutDeviceErrorLogModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module

	FCoreUObjectDelegates::PostLoadMapWithWorld.AddRaw(this,&FOutDeviceErrorLogModule::RegisterRecordToPostWorld);
	
}

void FOutDeviceErrorLogModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

void FOutDeviceErrorLogModule::RegisterRecordToPostWorld(UWorld* World)
{
	FString ConfigFilePath=FPaths::ProjectConfigDir()+"DefaultUserSetting.ini";
	FString Result;
	if(GConfig&&World)
	{
		bool IsStartRecord=false;
		/*if(FFileHelper::LoadFileToString(Result,*ConfigFilePath))
		{
			
		}else
		{
			FMessageDialog::Open(EAppMsgType::YesNo,FText::FromString(FPaths::ConvertRelativePathToFull(ConfigFilePath)));
			TArray<FString>Lines;
			if(Result.Contains("/Script/UserSetting.RecordSettings")&&Result.Contains("StartRecord"))
			{
				int Index=Result.Find("StartRecord");
			}
		}*/
		FString info;
		GConfig->GetString(TEXT("UserSeetingConfig"),TEXT("Test"),info,GGameIni);
		FMessageDialog::Open(EAppMsgType::YesNo,FText::FromString(info));
		
		if(IsStartRecord)
		{
			/*static FWindowOutDeviceLogError CrashToolOutPutDevice(GError);
			GError=&CrashToolOutPutDevice;
			UActionsRecorderLibrary::StartRecord(World->GetFirstPlayerController());*/
			FMessageDialog::Open(EAppMsgType::YesNo,FText::FromString(FString::FromInt(IsStartRecord)));
		}else
		{
			FMessageDialog::Open(EAppMsgType::YesNo,FText::FromString(FString::FromInt(IsStartRecord)));
		}
	}else
	{
		FMessageDialog::Open(EAppMsgType::YesNo,FText::FromString("Not GConfig"));
	}
	
	
	
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FOutDeviceErrorLogModule, OutDeviceErrorLog)