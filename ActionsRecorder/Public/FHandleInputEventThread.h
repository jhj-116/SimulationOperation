// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ActionsRecorderLibrary.h"

class FActionsRecordController;

/**
 * 
 */
class ACTIONSRECORDER_API FHandleInputEventThread: public FRunnable
{
private:
	TUniquePtr<FRunnableThread> Thread;

	TAtomic<bool> bExit;
	
	FActionsRecordController* Controller;

	TWeakPtr<SWindow> TargetWindow;

	void SendSingleActionData(TSharedPtr<FJsonValue> SingleAction);

public:

	void Start();

	void EnsureCompletion();
	
	FHandleInputEventThread(FActionsRecordController* InController, TWeakPtr<SWindow> InTargetWindow);
	virtual ~FHandleInputEventThread();

	//~ FRunnable Interface
	virtual bool Init() override;
	virtual uint32 Run() override;
	virtual void Stop() override;
	// End FRunnable Interface
};
