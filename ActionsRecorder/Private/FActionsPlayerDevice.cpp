// Fill out your copyright notice in the Description page of Project Settings.


#include "FActionsPlayerDevice.h"

TSharedPtr<FActionsPlayerDevice> FActionsPlayerDevice::Device;

FActionsPlayerDevice::FActionsPlayerDevice()
{
}

TSharedPtr<FActionsPlayerDevice> FActionsPlayerDevice::GetActionsPlayerDevice()
{
	if (!Device.IsValid())
	{
		Device = TSharedPtr<FActionsPlayerDevice>(new FActionsPlayerDevice());
	}

	return Device;
}

void FActionsPlayerDevice::Tick(float DeltaTime)
{
	for (TWeakPtr<FActionsRecordController> WeakController : ActionsController)
	{
		if (TSharedPtr<FActionsRecordController> Controller = WeakController.Pin())
		{
			Controller->Tick(DeltaTime);
		}
	}
}

void FActionsPlayerDevice::SendControllerEvents()
{
}

void FActionsPlayerDevice::SetMessageHandler(const TSharedRef<FGenericApplicationMessageHandler>& InMessageHandler)
{
	for (TWeakPtr<FActionsRecordController> WeakController : ActionsController)
	{
		if (TSharedPtr<FActionsRecordController> Controller = WeakController.Pin())
		{
			Controller->SetMessageHandler(InMessageHandler);
		}
	}
}

bool FActionsPlayerDevice::Exec(UWorld* InWorld, const TCHAR* Cmd, FOutputDevice& Ar)
{
	bool bRetVal = true;
	for (TWeakPtr<FActionsRecordController> WeakController : ActionsController)
	{
		if (TSharedPtr<FActionsRecordController> Controller = WeakController.Pin())
		{
			bRetVal &= Controller->Exec(InWorld, Cmd, Ar);
		}
	}

	return bRetVal;
}

void FActionsPlayerDevice::SetChannelValues(int32 ControllerId, const FForceFeedbackValues& values)
{
	for (TWeakPtr<FActionsRecordController> WeakController : ActionsController)
	{
		if (TSharedPtr<FActionsRecordController> Controller = WeakController.Pin())
		{
			Controller->SetChannelValues(ControllerId, values);
		}
	}
}

void FActionsPlayerDevice::SetChannelValue(int32 ControllerId, FForceFeedbackChannelType ChannelType, float Value)
{
	for (TWeakPtr<FActionsRecordController> WeakController : ActionsController)
	{
		if (TSharedPtr<FActionsRecordController> Controller = WeakController.Pin())
		{
			Controller->SetChannelValue(ControllerId, ChannelType, Value);
		}
	}
}

void FActionsPlayerDevice::AddActionController(TSharedPtr<FActionsRecordController> ActionsRecordController)
{
	ActionsController.Add(ActionsRecordController);
}

