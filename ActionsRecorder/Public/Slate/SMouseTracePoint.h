// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/AudioComponent.h"
#include "Widgets/SCompoundWidget.h"

/**
 * 
 */
class ACTIONSRECORDER_API SMouseTracePoint : public SCompoundWidget
{
	TSharedPtr<SImage> MouseTraceImage;

	TAttribute<APlayerController*> ControllerAttr;
	TAttribute<UWorld*> WorldAttr;

	const float TracePointWidth = 100;
	const float TracePointLength = 100;
	
public:
	SLATE_BEGIN_ARGS(SMouseTracePoint)
		:_ControllerAttr(nullptr), _WorldAttr(nullptr)
		{
		}
	SLATE_ATTRIBUTE(APlayerController*, ControllerAttr)
	SLATE_ATTRIBUTE(UWorld*, WorldAttr);
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);

	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;
	
};
