// Fill out your copyright notice in the Description page of Project Settings.


#include "FWindowOutDeviceLogError.h"

#include "ActionsRecorderLibrary.h"


extern CORE_API bool GIsGPUCrashed;

FWindowOutDeviceLogError::FWindowOutDeviceLogError(FOutputDeviceError*InputDevice)
{
	OldDevice=InputDevice;
}

void FWindowOutDeviceLogError::Serialize( const TCHAR* Msg, ELogVerbosity::Type Verbosity, const class FName& Category )
{
	OldDevice->Serialize(Msg,Verbosity,Category);
	if(!GIsGuarded)
	{
		HandleError();
	}
}

void FWindowOutDeviceLogError::HandleError()
{
	// make sure we don't report errors twice
	static int32 CallCount = 0;
	int32 NewCallCount = FPlatformAtomics::InterlockedIncrement(&CallCount);
	if (NewCallCount != 1)
	{
		UE_LOG(LogWindows, Error, TEXT("HandleError re-entered.") );
		return;
	}
	
	GIsGuarded				= 0;
	GIsRunning				= 0;
	GIsCriticalError		= 1;
	GLogConsole				= NULL;
	GErrorHist[UE_ARRAY_COUNT(GErrorHist) - 1] = TCHAR('\0');

	UActionsRecorderLibrary::StopRecord();

	
	
	// Trigger the OnSystemFailure hook if it exists
	// make sure it happens after GIsGuarded is set to 0 in case this hook crashes
	FCoreDelegates::OnHandleSystemError.Broadcast();

	// Dump the error and flush the log. If you change behavior here, you should probably update RenderingThread's __except block in FRenderingThread::Run
	#if !NO_LOGGING
	FDebug::LogFormattedMessageWithCallstack(LogWindows.GetCategoryName(), __FILE__, __LINE__, TEXT("=== Critical error: ==="), GErrorHist, ELogVerbosity::Error);
#endif
	GLog->Panic();

	HandleErrorRestoreUI();

	FPlatformMisc::SubmitErrorReport( GErrorHist, EErrorReportMode::Interactive );

	FCoreDelegates::OnShutdownAfterError.Broadcast();
}

void FWindowOutDeviceLogError::HandleErrorRestoreUI()
{
}