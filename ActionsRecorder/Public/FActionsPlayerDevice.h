// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ActionsRecorderLibrary.h"
#include "IInputDevice.h"

/**
 * 
 */
class ACTIONSRECORDER_API FActionsPlayerDevice : public IInputDevice
{
	TArray<TWeakPtr<FActionsRecordController>> ActionsController;

	static TSharedPtr<FActionsPlayerDevice> Device;
	
	FActionsPlayerDevice();
public:

	static TSharedPtr<FActionsPlayerDevice> GetActionsPlayerDevice();
	
	virtual void Tick(float DeltaTime) override;
	virtual void SendControllerEvents() override;
	virtual void SetMessageHandler(const TSharedRef<FGenericApplicationMessageHandler>& InMessageHandler) override;
	virtual bool Exec(UWorld* InWorld, const TCHAR* Cmd, FOutputDevice& Ar) override;
	virtual void SetChannelValues(int32 ControllerId, const FForceFeedbackValues& values) override;
	virtual void SetChannelValue(int32 ControllerId, FForceFeedbackChannelType ChannelType, float Value) override;

	void AddActionController(TSharedPtr<FActionsRecordController> ActionsRecordController);
};
