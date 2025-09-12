// Fill out your copyright notice in the Description page of Project Settings.


#include "Alice/Widget/AlicePlayerWidget.h"
#include "Alice/Alice.h"
#include "Components/ProgressBar.h"
#include "Kismet/GameplayStatics.h"
#include "BossHPWidget.h"
#include "Components/TextBlock.h"
#include "Alice/DialogueDataAsset.h"
#include "Components/WidgetSwitcher.h"
#include "Components/CanvasPanel.h"
#include "Components/Image.h"
#include "Revenant/Revenant.h"
#include "Components/Button.h"
#include "ProfilingDebugging/BootProfiling.h"
#define LOCTEXT_NAMESPACE "HPText"

void UAlicePlayerWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	if (Dialogue) Dialogue->SetText(FText::GetEmpty());

	RestartBtn->OnClicked.AddDynamic(this, &UAlicePlayerWidget::RestartBossStage);

	
}
bool UAlicePlayerWidget::ShowByKey(FName Key)
{
	if (!DialogueData || !Dialogue)
	{
		if (!DialogueData) GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::White,TEXT("DialogueData"));
		if (!Dialogue)GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::White,TEXT("Dialogue"));
		return false;
	}

	FText Line;
	if (DialogueData->GetLineByKey(Key, Line))
	{
		Dialogue->SetText(Line);
		return true;
	}
	return false;
}

bool UAlicePlayerWidget::ShowByIndex(int32 Index)
{
	if (!DialogueData || !Dialogue)
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::White,TEXT("ShowByIndex"));
		
		return false;
	}

	FText Line;
	if (DialogueData->GetLineByIndex(Index, Line))
	{
		Dialogue->SetText(Line);
		CurrentIndex = Index; // 진행 위치 갱신
		return true;
	}
	return false;
}

bool UAlicePlayerWidget::NextLine()
{
	if (!DialogueData || !Dialogue) return false;

	const int32 Next = CurrentIndex + 1;
	FText Line;
	if (DialogueData->GetLineByIndex(Next, Line))
	{
		Dialogue->SetText(Line);
		CurrentIndex = Next;
		return true;
	}
	return false;
}

void UAlicePlayerWidget::ClearDialogue()
{
	if (Dialogue) Dialogue->SetText(FText::GetEmpty());
}

void UAlicePlayerWidget::AvoidPanelActive(bool _active)
{
	FLinearColor color = FLinearColor(0.5,0.8,1,0.7);
	if (_active)
		color = FLinearColor(0,0,0,0);
	else
	AvoidPanel->SetColorAndOpacity(color);
}

void UAlicePlayerWidget::DiedPanelActive(bool _active)
{
	if (_active == true)
	{
		RestartBtn->SetVisibility(ESlateVisibility::Visible);
		DiedText->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		RestartBtn->SetVisibility(ESlateVisibility::Hidden);
		DiedText->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UAlicePlayerWidget::RestartBossStage()
{
	//GEngine	->AddOnScreenDebugMessage(-1, 1.0f, FColor::White, TEXT("RestartButton"));
	// 1. 일단 화면을 어둡게 해
	// 2. 위치를 Emote로 설정
	// 3. 화면을 밝게 만드렁
	// 4. 0.5초뒤에 인풋 가능 및 BT 동작

	DiedPanelActive(false);
	
	RestartBtn->SetVisibility(ESlateVisibility::Hidden);
	DiedText->SetVisibility(ESlateVisibility::Hidden);
	
	Alice->BossStageInit();
	Revenant->BossStateInit();
	FullImageShow(false);
}

// true : 페널 Video로 변경하고 화면 어둡게 만들기
// false  : 화면 alpha 0으로(게임화면 보이게) 만들고 패널 Play로

void UAlicePlayerWidget::FullImageShow(bool _enable)
{
	if (_enable)
	{
		FullImageActive(true); // alpha 0 -> 1
		ToggleView();
		bIsFullImageGradation = true; // false는 BTTask 에서 호출하는 함수랑 같이
	}
	else
	{
		FullImageActive(false);
		bIsFullImageGradation = false;
		ToggleView();
	}
}

void UAlicePlayerWidget::NativeConstruct()
{
	Super::NativeConstruct();

	Switcher->SetActiveWidgetIndex(0);
}

void UAlicePlayerWidget::OnHPChanged(float Current, float Max)
{
	if (!HPBar)return;
	//GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Yellow,TEXT("[AlicePlayerWidget] BroadCast In Widget"));
	
	//const float Pct = (Max <= 0.f) ? 0.f :  FMath::Clamp(Current / Max, 0.f, 1.0f);
	const float Pct = Current / Max;
	HPBar->SetPercent(Pct);
	const FText Format = FText::Format(
		LOCTEXT("HPText", "{0} / {1}"),
	FText::AsNumber(Current),
	FText::AsNumber(Max));
	
	HPText->SetText(Format);
}

void UAlicePlayerWidget::OnMPChanged(float Current, float Max)
{
	if (!MPBar)return;
	//const float Pct = (Max <= 0.f) ? 0.f : FMath::Clamp(Current / Max, 0.f, 1.f);
	const float Pct = Current / Max;
	MPBar->SetPercent(Pct);
	const FText Format = FText::Format(
		LOCTEXT("MPText", "{0} / {1}"),
	FText::AsNumber(Current),
	FText::AsNumber(Max));
	MPText->SetText(Format);
}

void UAlicePlayerWidget::OnBossHPChanged(float Current, float Max)
{
	if (!BossHPBar)
	{
	GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, TEXT("OnBossHPChanged - failed"));
		
		return;
	}
	//GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, TEXT("OnBossHPChanged"));

	
	//const float Pct = (Max <= 0.f) ? 0.f : FMath::Clamp(Current / Max, 0.f, 1.f);
	const float Pct = Current / Max;
	BossHPBar->SetPercent(Pct);
	
	//GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow, FString::Printf(TEXT("pct : %f"),Current / Max));
	
	const FText Format = FText::Format(
		LOCTEXT("BossHPText", "{0} / {1}"),
	FText::AsNumber(Current),
	FText::AsNumber(Max));
	BossHPText->SetText(Format);
}

void UAlicePlayerWidget::OnBossMPChanged(float Current, float Max)
{
	if (!BossMPBar) return;
	
	const float Pct = Current / Max;
	BossMPBar->SetPercent(Pct);
	
	const FText Format = FText::Format(
		LOCTEXT("BossMPText", "{0} / {1}"),
	FText::AsNumber(Current),
	FText::AsNumber(Max));
	
	BossMPText->SetText(Format);
}

void UAlicePlayerWidget::OnSetBossName(const FText& _name)
{
	BossName->SetText(_name);
}

void UAlicePlayerWidget::RefreshAllNow()
{
	if (Alice)
	{
		OnHPChanged(Alice->GetCurrentHP(), Alice->GetMaxHP());
		OnMPChanged(Alice->GetCurrentMP(), Alice->GetMaxMP());
	//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, FString::Printf(TEXT("[ Alice in Widget ] : %s"),*Alice->GetName()));
		
	}
	if (Revenant)
	{
		OnBossHPChanged(Revenant->GetCurrentHP(), Revenant->GetMaxHP());
		OnBossMPChanged(Revenant->GetCurrentMP(), Revenant->GetMaxMP());
	//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, FString::Printf(TEXT("[ Revenant in Widget ] : %s"),*Revenant->GetName()));
		
	}



}

void UAlicePlayerWidget::BindCharacter(AAlice* alice)
{
	Alice = alice;
	
	HPHandle = Alice->OnHPChanged.AddUObject(this, &UAlicePlayerWidget::OnHPChanged);
	MPHandle = Alice->OnMPChanged.AddUObject(this, &UAlicePlayerWidget::OnMPChanged);

	// 초기값 즉시 싱크
	RefreshAllNow();
}

void UAlicePlayerWidget::BindBoss(ARevenant* revenant)
{
	Revenant = revenant;
	if (!Revenant) return;
	
	BossHPHandle = Revenant->OnBossHPChanged.AddUObject(this, &UAlicePlayerWidget::OnBossHPChanged);
	BossMPHandle = Revenant->OnBossMPChanged.AddUObject(this, &UAlicePlayerWidget::OnBossMPChanged);
	
	RefreshAllNow();
	//BossHPHandle
}

void UAlicePlayerWidget::ShowPlayer()
{
	Switcher->SetActiveWidget(PlayerPanel);
}

void UAlicePlayerWidget::ShowVideo()
{
	Switcher->SetActiveWidget(VideoPanel);
}

void UAlicePlayerWidget::ToggleView()
{
	if (!Switcher) return;

	UWidget* Cur = Switcher->GetActiveWidget();
	if (Cur == PlayerPanel)
	{
		Switcher->SetActiveWidget(VideoPanel);
	}
	else if (VideoPanel)
	{
		Switcher->SetActiveWidget(PlayerPanel);
	}
}


void UAlicePlayerWidget::BossUIActive(bool _enable)
{
	if (_enable)
	{
		BossName->SetVisibility(ESlateVisibility::Visible);
		BossHPBar->SetVisibility(ESlateVisibility::Visible);
		BossMPBar->SetVisibility(ESlateVisibility::Visible);
		BossHPText->SetVisibility(ESlateVisibility::Visible);
		BossMPText->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		BossName->SetVisibility(ESlateVisibility::Hidden);
		BossHPBar->SetVisibility(ESlateVisibility::Hidden);
		BossMPBar->SetVisibility(ESlateVisibility::Hidden);
		BossHPText->SetVisibility(ESlateVisibility::Hidden);
		BossMPText->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UAlicePlayerWidget::ChangeQuestDireciton(int32 current, int32 max)
{
	if (!QuestDirection) return;
	//const float Pct = Current / Max;
	//BossMPBar->SetPercent(Pct);
	const FText Format = FText::Format(
		LOCTEXT("HPText", "{0} / {1}"),
	FText::AsNumber(current),
	FText::AsNumber(max));
	
	QuestDirection->SetText(Format);
}

#undef LOCTEXT_NAMESPACE
