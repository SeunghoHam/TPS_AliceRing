// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/BossHPWidget.h"
#include "BaseEnemy.h"
#include "AnbyGameState.h"
#include "ZZZEnemy.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

void UBossHPWidget::Setup(class UMainMenu* _parent)
{
	//Parent = _parent;
}

bool UBossHPWidget::Initialize()
{
	bool init = Super::Initialize();
	if (!init) return false;
	//Setup();
	
	return true;
}

void UBossHPWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (GS == nullptr)
	{
	//	GS=GetWorld()->GetGameState<AAnbyGameState>();
		GS = Cast<AAnbyGameState>(GetWorld()->GetGameState());
	}
	GS->OnTargetBossChanged.AddDynamic(this, &UBossHPWidget::UpdateTargetBoss);
	UpdateTargetBoss(GS->CurrentTargetEnemy);
	
	GS->OnBossHPChanged.AddDynamic(this, &UBossHPWidget::UpdateBossHP);
	UpdateBossHP(GS->BossCurrnetHP, GS->BossMaxHP);
	

	
}

void UBossHPWidget::UpdateBossHP(float _current, float _max)
{
	if (BossHPBar)
	{
		BossHPBar->SetPercent(_current/_max);
	}
}
void UBossHPWidget::UpdateTargetBoss(AZZZEnemy* _enemy)
{
	if (BossName)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green,TEXT("WBP_BossHPWidget_ UpdateTargetBoss"));
		//BossName->SetText(_enemy->GetBossName());
	}
}

