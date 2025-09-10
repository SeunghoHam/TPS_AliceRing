// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AlicePlayerWidget.generated.h"

/**
 * 
 */
class UWidgetSwitcher;
class AAlice;
class ARevenant;
class UProgressBar;
class UBossHPWidget;
class UTextBlock;
class UDialogueDataAsset;
class UCanvasPanel;
class UImage;
class UButton;
UCLASS()
class WORKTEST_API UAlicePlayerWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeOnInitialized() override;
	// 위젯이 사용할 데이터 (BP에서 인스턴스 편집 가능, 스폰 시 전달 가능)
	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue", meta=(ExposeOnSpawn=true))
	//TSubclassOf<UDialogueDataAsset> DialogueDataAsset;
	UDialogueDataAsset* DialogueData = nullptr;

	// 화면에 찍을 TextBlock (UMG에서 이름을 Dialogue로)
	UPROPERTY(meta=(BindWidget))
	UTextBlock* Dialogue = nullptr;

	// 순차 재생용 인덱스
	UPROPERTY(BlueprintReadOnly, Category="Dialogue")
	int32 CurrentIndex = 0;

	// --- API: DataAsset -> Widget ---
	UFUNCTION(BlueprintCallable, Category="Dialogue")
	bool ShowByKey(FName Key);          // DialogueDataAsset의 키로 출력

	UFUNCTION(BlueprintCallable, Category="Dialogue")
	bool ShowByIndex(int32 Index);      // DialogueDataAsset의 인덱스로 출력

	UFUNCTION(BlueprintCallable, Category="Dialogue")
	bool NextLine();                    // OrderedLines에서 다음 줄 출력

	UFUNCTION(BlueprintCallable, Category="Dialogue")
	void ClearDialogue();               // 텍스트 지우기
	
	
	
	
	//UPROPERTY(meta=(BindWidget))
	//UBossHPWidget* BossHPWidget;
	
	// UMG 디자이너에서 같은 이름으로 ProgressBar 배치
	UPROPERTY(meta=(BindWidget)) UProgressBar* HPBar = nullptr;
	UPROPERTY(meta=(BindWidget)) UProgressBar* MPBar = nullptr;
	
	UPROPERTY(meta=(BindWidget)) UTextBlock* HPText = nullptr;
	UPROPERTY(meta=(BindWidget)) UTextBlock* MPText = nullptr;


	UPROPERTY(meta=(BindWidget)) UProgressBar* BossHPBar = nullptr;
	UPROPERTY(meta=(BindWidget)) UProgressBar* BossMPBar = nullptr;
	UPROPERTY(meta=(BindWidget)) UTextBlock* BossHPText = nullptr;
	UPROPERTY(meta=(BindWidget)) UTextBlock* BossMPText = nullptr;
	
	UPROPERTY(meta=(BindWidget)) UTextBlock* BossName = nullptr;
	
	UPROPERTY(meta=(BindWidget)) UImage* AvoidPanel = nullptr;

	UPROPERTY(meta=(BindWidget)) UButton* RestartBtn = nullptr;
	UPROPERTY(meta=(BindWidget)) UTextBlock* DiedText = nullptr;
	
	
	UPROPERTY(BlueprintReadWrite,EditAnywhere, meta=(BindWidget)) UImage* FullImage = nullptr;

	void AvoidPanelActive(bool _active);

	void DiedPanelActive(bool _active);
	void RestartBossStage();

	void FullImageShow(bool _enable);
	//void FUllImageHide();
	UFUNCTION(BlueprintCallable,BlueprintImplementableEvent)
	void FullImageActive(bool _value);
	virtual void NativeConstruct() override;
	//virtual void NativeDestruct() override;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	
	bool bIsFullImageGradation = false;
	// 델리게이트 핸들(언바인딩용)
	FDelegateHandle MPHandle;
	FDelegateHandle HPHandle;
	FDelegateHandle BossHPHandle;
	FDelegateHandle BossMPHandle;
	
	// 콜백 (델리게이트에서 호출됨)
	void OnHPChanged(float Current, float Max);
	void OnMPChanged(float Current, float Max);
	void OnBossHPChanged(float Current, float Max);
	void OnBossMPChanged(float Current, float Max);

	void OnSetBossName(const FText& _name);
	// 초기 싱크
	void RefreshAllNow();
	void BindCharacter(AAlice* alice);
	AAlice* Alice = nullptr;

	void BindBoss(ARevenant* revenant);
	ARevenant* Revenant= nullptr;

	
	UFUNCTION(BlueprintCallable) void ShowPlayer();
	UFUNCTION(BlueprintCallable) void ShowVideo();
	UFUNCTION(BlueprintCallable) void ToggleView();
	UPROPERTY(meta=(BindWidget))
	UWidgetSwitcher* Switcher;
	UPROPERTY(meta=(BindWidgetOptional)) UCanvasPanel* PlayerPanel = nullptr;
	UPROPERTY(meta=(BindWidgetOptional)) UCanvasPanel* VideoPanel  = nullptr;
	UPROPERTY(meta=(BindWidgetOptional),BlueprintReadWrite,EditAnywhere) UTextBlock* QuestName  = nullptr; // 도로롱 처치
	UPROPERTY(meta=(BindWidgetOptional)) UTextBlock* QuestDirection  = nullptr; // 3 / 3

	
	void BossUIActive(bool _enable);
	UFUNCTION(BlueprintImplementableEvent)
	void ChangeQuestName();
	void ChangeQuestDireciton(int32 current, int32 max);
	
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	float CheckTime = 0.0f;
	bool IsChargeState =false;
	
	UFUNCTION(BlueprintCallable,BlueprintPure)
	float GetCheckTime() {return CheckTime;}
	UFUNCTION(BlueprintCallable,BlueprintPure)
	bool IsCharging(){ return IsChargeState;}	
	
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SetVideoOpacityEnable(bool _value);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SetBackGroundReset();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void Video_Ultimate();
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void Video_ChargeModeGo();
};
