// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 * 
 */
class OUTDEVICEERRORLOG_API FWindowOutDeviceLogError:public FOutputDeviceError
{
public:
	/** Constructor, initializing member variables */
	FWindowOutDeviceLogError(FOutputDeviceError*InputDevice);

	/**
	 * Serializes the passed in data unless the current event is suppressed.
	 *
	 * @param	Data	Text to log
	 * @param	Event	Event name used for suppression purposes
	 */
	virtual void Serialize( const TCHAR* Msg, ELogVerbosity::Type Verbosity, const class FName& Category ) override;

	/**
	 * Error handling function that is being called from within the system wide global
	 * error handler, e.g. using structured exception handling on the PC.
	 */
	virtual void HandleError() override;
	FOutputDeviceError*OldDevice;

protected:
	/**
	 * Callback to allow FWindowsApplicationErrorOutputDevice to restore the UI.
	 */
	virtual void HandleErrorRestoreUI();
};
