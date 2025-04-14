// Fill out your copyright notice in the Description page of Project Settings.


#include "FHandleInputEventThread.h"

#include "JsonObjectConverter.h"

void FHandleInputEventThread::Start()
{
	Thread.Reset(FRunnableThread::Create(this, TEXT("ActionsRecord Playing Thread")));
}

void FHandleInputEventThread::EnsureCompletion()
{
	if (Thread != nullptr)
	{
		Thread->WaitForCompletion();
	}
}

FHandleInputEventThread::FHandleInputEventThread(FActionsRecordController* InController, TWeakPtr<SWindow> InTargetWindow)
	: bExit(false), Controller(InController), TargetWindow(InTargetWindow)
{
}

FHandleInputEventThread::~FHandleInputEventThread()
{
	if (Thread != nullptr)
	{
		Thread->Kill(true);
		Thread.Reset();
	}
}

bool FHandleInputEventThread::Init()
{
	return FRunnable::Init();
}

void FHandleInputEventThread::SendSingleActionData(TSharedPtr<FJsonValue> SingleAction)
{
	UE_LOG(LogTemp, Warning, TEXT("=============== Thread Enqueue Actions Start. ==============="));

	TSharedPtr<FJsonObject> Record = SingleAction->AsObject();

	// 设定视角
	FSingleRecord SingleRecord;
	FJsonObjectConverter::JsonObjectToUStruct(Record.ToSharedRef(), FSingleRecord::StaticStruct(), &SingleRecord, 0, 0);
	if (Controller)
	{
		//Controller->EnqueueActionOriginalView(SingleRecord.OriginalLocation, SingleRecord.OriginalViewDirection);
		if(GWorld&&GWorld->GetFirstPlayerController())
		{
			GWorld->GetFirstPlayerController()->SetInitialLocationAndRotation(SingleRecord.OriginalLocation, SingleRecord.OriginalViewDirection.Rotation());
		}
		
	}
	
	TArray<TSharedPtr<FJsonValue>> Actions = Record->GetArrayField("ActionRecordData");
	
	// simulate actions
	double LastActionsTime = 0, LastActTime = 0;
	int i = 0;
	while (i < Actions.Num())
	{
		// exit thread;
		if (bExit)
		{
			UE_LOG(LogTemp, Warning, TEXT("=============== Thread Enqueue Actions Exiting. ==============="));
			return;
		}
			
		FActionRecordData ActionRecordData;
		FJsonObjectConverter::JsonObjectToUStruct(Actions[i]->AsObject().ToSharedRef(),
			FActionRecordData::StaticStruct(), &ActionRecordData, 0, 0);
			
		if (LastActionsTime == 0 && LastActTime == 0)
		{
			LastActionsTime = ActionRecordData.ActTime;
			LastActTime = FPlatformTime::Seconds();
		}

		double Now = FPlatformTime::Seconds(); 
		if (Now - LastActTime >= ActionRecordData.ActTime - LastActionsTime)
		{
			if (Controller)
			{
				Controller->EnqueueAction(ActionRecordData, TargetWindow);
			}
				
			LastActionsTime = ActionRecordData.ActTime;
			LastActTime = Now;
			++i;
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("=============== Thread Enqueue Actions End. ==============="));
}

uint32 FHandleInputEventThread::Run()
{
	if (Controller == nullptr) return 0;
	
	// all records
	TArray<TSharedPtr<FJsonValue>> ActionsRecords = Controller->GetActionsRecord()->GetArrayField("ActionsRecord");

	for (TSharedPtr<FJsonValue> ActionRecord : ActionsRecords)
	{
		// exit thread;
		if (bExit)
		{
			UE_LOG(LogTemp, Warning, TEXT("=============== Thread Enqueue Actions Exiting. ==============="));
			return 0;
		}

		FPlatformProcess::Sleep(1);
		
		SendSingleActionData(ActionRecord);
	}
	
	return 0;
}

void FHandleInputEventThread::Stop()
{
	FRunnable::Stop();

	bExit = true;
}
