#include "ActionsRecorder.h"
#include "ActionsRecorderLibrary.h"
#include "FActionsMessageHandler.h"

#define LOCTEXT_NAMESPACE "FActionsRecorderModule"



void FActionsRecorderModule::StartupModule()
{
	InputDevice = FActionsPlayerDevice::GetActionsPlayerDevice();
	CreateInputHandler();

	IModularFeatures::Get().RegisterModularFeature(GetModularFeatureName(), this);

	TestActionsMessageHandler = MakeShareable(new FActionsMessageHandler());

	if (FSlateApplication::Get().GetPlatformApplication().IsValid())
	{
		if (FWindowsApplication* Application =
			static_cast<FWindowsApplication*>(FSlateApplication::Get().GetPlatformApplication().Get()))
		{
			Application->AddMessageHandler(*TestActionsMessageHandler);
			
		}
	}
}

void FActionsRecorderModule::ShutdownModule()
{
}

TSharedPtr<IInputDevice> FActionsRecorderModule::CreateInputDevice(
	const TSharedRef<FGenericApplicationMessageHandler>& InMessageHandler)
{
	return InputDevice;
}

void FActionsRecorderModule::CreateInputHandler()
{
	InputDevice->AddActionController(FActionsRecordController::GetInstance());
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FActionsRecorderModule, ActionsRecorder)
