// Fill out your copyright notice in the Description page of Project Settings.


#include "Slate/SMouseTracePoint.h"

#include "SlateOptMacros.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Widgets/Layout/SConstraintCanvas.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION


void SMouseTracePoint::Construct(const FArguments& InArgs)
{
	ControllerAttr = InArgs._ControllerAttr;
	WorldAttr = InArgs._WorldAttr;

	SetVisibility(EVisibility::SelfHitTestInvisible);
	
	ChildSlot
	[
		// Populate the widget
		SNew(SConstraintCanvas)
		+ SConstraintCanvas::Slot()
		.Alignment(FVector2D(0.5, 0.5))
		.Offset(FMargin(0, 0, TracePointWidth, TracePointLength))
		[
			SAssignNew(MouseTraceImage, SImage)
			.Image(FAppStyle::GetBrush(TEXT("Icons.PlusCircle")))
			.ColorAndOpacity(FSlateColor(FLinearColor(1, 1, 1, 0.3)))
			.Visibility(EVisibility::SelfHitTestInvisible)
		]
	];
}

void SMouseTracePoint::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

	FVector2D MousePos;
	if (ControllerAttr.IsSet() && WorldAttr.IsSet())
	{
		MousePos = UWidgetLayoutLibrary::GetMousePositionOnViewport(WorldAttr.Get());
	}

	if (MouseTraceImage.IsValid())
	{
		MouseTraceImage->SetRenderTransform(FSlateRenderTransform(MousePos));
	}
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION
