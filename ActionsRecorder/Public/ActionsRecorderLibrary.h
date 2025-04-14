// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FHandleInputEventThread.h"
#include "IInputDevice.h"

#include "ActionsRecorderLibrary.generated.h"

UENUM(BlueprintType)
enum class EPlayRecordMode
{
	PRM_Single             UMETA(DisplayName = "Play Single"), 
	PRM_SingleLoop         UMETA(DisplayName = "Single Loop"),
	PRM_Sequential         UMETA(DisplayName = "Sequential Play All Record"),
	PRM_Repeat             UMETA(DisplayName = "Loop All Record"),
};

/**
 * mouse button type
 */
UENUM()
enum class EButtonType
{
	Left    = 0,
	Middle,
	Right,
	Thumb01,
	Thumb02,
	MouseMove,
	MouseWheel,
	KeyChar,
	KeyDown,
	KeyUp,

	Invalid,
};

/**
 * an independent user input action
 */
USTRUCT()
struct FActionRecordData
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, Category = "Actions Record")
	double ActTime = 0.0f;

	UPROPERTY(VisibleAnywhere, Category = "Actions Record")
	FVector2D MouseLocation = {0, 0};

	UPROPERTY(VisibleAnywhere, Category = "Actions Record")
	FVector2D AbsoluteLocation = {0, 0};

	UPROPERTY(VisibleAnywhere, Category = "Actions Record")
	FVector2D ViewportSize = {0, 0};

	UPROPERTY(VisibleAnywhere, Category = "Actions Record")
	bool IsDown = false;

	// use for keyboard only
	UPROPERTY(VisibleAnywhere, Category = "Actions Record")
	bool IsRepeat = false;

	UPROPERTY(VisibleAnywhere, Category = "Actions Record")
	int32 KeyCode = -1;

	UPROPERTY(VisibleAnywhere, Category = "Actions Record")
	float WheelDelta=0;
	
	UPROPERTY(VisibleAnywhere, Category = "Actions Record")
	EButtonType ButtonType = EButtonType::Invalid;

	UPROPERTY(VisibleAnywhere, Category = "Actions Record")
	FVector ViewDirection = FVector::ZeroVector;
};

/**
 * a actions record has many independent actions
 */
USTRUCT()
struct FSingleRecord
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, Category = "Actions Record")
	FString Timestamp;
	
	UPROPERTY(VisibleAnywhere, Category = "Actions Record")
	FVector OriginalViewDirection = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, Category = "Actions Record")
	FVector OriginalLocation = FVector::ZeroVector;
	
	UPROPERTY(VisibleAnywhere, Category = "Actions Record")
	TArray<FActionRecordData> ActionRecordData;

	FSingleRecord()
	{}

	FSingleRecord(FString& InTimestamp, FVector& InViewDirection, FVector& InOriginalLoc)
		: Timestamp(InTimestamp)
		, OriginalViewDirection(InViewDirection)
		, OriginalLocation(InOriginalLoc)
	{
	}
};

class ACTIONSRECORDER_API FActionsRecordController : public IInputDevice
{
private:
	FString ActionsRecordFilename;

	APlayerController* PlayerController = nullptr;

	TSharedPtr<FGenericApplicationMessageHandler> MessageHandler;
	
	FActionsRecordController(const TSharedPtr<FGenericApplicationMessageHandler>& InTargetHandler);

	void CreateActionsRecordFile();
	
	static TSharedPtr<FActionsRecordController> Singleton;

	TSharedPtr<FJsonObject> ActionsRecordJsonObject = nullptr;

	// 单次do action所需的参数
	struct FActionParam
	{
		FActionRecordData ActionRecordData;
		TWeakPtr<SWindow> TargetWindow;
	};

	// 存储action的队列，线程安全的
	TQueue<FActionParam> ActionQueue;

	struct FActionView
	{
		FVector ViewOriginal;
		FVector ViewDirection;
	};
	TQueue<FActionView> ActionsView;

	TUniquePtr<class FHandleInputEventThread> PlayingThread;
	
public:
	static TSharedPtr<FActionsRecordController> GetInstance();

	TSharedPtr<FJsonObject> GetActionsRecord() {return ActionsRecordJsonObject;}
	
	void EnqueueAction(FActionRecordData& Action, TWeakPtr<SWindow>& TargetWindow);

	void EnqueueActionOriginalView(const FVector& ViewOriginal, const FVector& ViewDirection);

	// save one actions record
	void SaveSingleRecord(const FSingleRecord& SingleRecord);

	// read generated lcm file and initilize json object
	void ReadActionsRecord();

	// save actions record json object to file
	void SaveActionsRecord();

	// use generated lcm data to modify actor, !!!only one time in runtime
	void PlayActionsRecord(TWeakPtr<SWindow>TargetWindow, int32 RecordIndex, EPlayRecordMode PlayMode);



	void StopPlay();

	// setup location and viewdirectio of controller
	void SetupController(const FActionView& ActionView) const;

	void StopPlayingThread();

	void SetPlayerController(APlayerController* Controller) {PlayerController = Controller;}

	void DoAction(FActionRecordData& Action, TWeakPtr<SWindow>& TargetWindow) const;

	virtual ~FActionsRecordController();
	
	////////////// inputdevice interface implementation /////////////
	virtual void Tick(float DeltaTime) override;
	virtual void SendControllerEvents() override;
	virtual void SetMessageHandler(const TSharedRef<FGenericApplicationMessageHandler>& InMessageHandler) override;
	virtual bool Exec(UWorld* InWorld, const TCHAR* Cmd, FOutputDevice& Ar) override;
	virtual void SetChannelValues(int32 ControllerId, const FForceFeedbackValues& values) override;
	virtual void SetChannelValue(int32 ControllerId, FForceFeedbackChannelType ChannelType, float Value) override;

};



/**
 * 
 */
UCLASS()
class ACTIONSRECORDER_API UActionsRecorderLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	static TSharedPtr<FSingleRecord> SingleRecord;
	
public:
	// Record single action
	static void RecordAction(FActionRecordData ActionRecordData, APlayerController* Controller);

	// register message handler to listenning, and start to record
	UFUNCTION(BlueprintCallable, Category = "ActionsRecorder")
	static void StartRecord(APlayerController* PlayerController);

	// stop recording
	UFUNCTION(BlueprintCallable, Category = "ActionsRecorder")
	static void StopRecord();

	// playing record
	UFUNCTION(BlueprintCallable, Category = "ActionsRecorder")
	static void PlayRecord();

	// stop playing record
	UFUNCTION(BlueprintCallable, Category = "ActionsRecorder")
	static void StopPlaying();

	// set controller of action handler
	UFUNCTION(BlueprintCallable, Category = "ActionsRecorder")
	static void SetActionController(APlayerController* PlayerController);

	static FVector GetControllerRotation(APlayerController* PlayerController);

	FWidgetPath FindRoutingMessageWidget(const FVector2D& Location, TWeakPtr<SWindow>& TargetWindow,
	FSlateApplication& SlateApplication);
	
	static FKey TranslateMouseButtonToKey(EButtonType Button);
	

	UFUNCTION(BlueprintCallable, Category = "ActionsRecorder")
	static void StartLoopRecordMain(FString Name="Test.json");
	
	static void OnMouseUp(FPointerEvent& Event, FSlateApplication& SlateApplication);

	static void OnMouseDown(FPointerEvent& Event, FSlateApplication& SlateApplication);

	static bool IsHotKey(int32 KeyCode);

	inline static constexpr int32 MOUSE_CODE = -1;
};

