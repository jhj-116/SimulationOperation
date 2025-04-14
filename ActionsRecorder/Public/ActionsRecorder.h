#pragma once

#include "CoreMinimal.h"
#include "FActionsPlayerDevice.h"
#include "IInputDeviceModule.h"
#include "Modules/ModuleManager.h"

class FActionsMessageHandler;

class FActionsRecorderModule : public IInputDeviceModule
{
    TSharedPtr<FActionsPlayerDevice> InputDevice;
    
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;

    virtual TSharedPtr<IInputDevice> CreateInputDevice(const TSharedRef<FGenericApplicationMessageHandler>& InMessageHandler) override;

    void CreateInputHandler();
    TSharedPtr<FActionsMessageHandler> TestActionsMessageHandler;
};

