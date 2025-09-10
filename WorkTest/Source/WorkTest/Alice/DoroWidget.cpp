// Fill out your copyright notice in the Description page of Project Settings.


#include "Alice/DoroWidget.h"
#include "Components/ProgressBar.h"

void UDoroWidget::HPChange(float Current, float Max)
{
	//const float Pct = (Max <= 0.f) ? 0.f : FMath::Clamp(Current / Max, 0.f, 1.f);
	HpBar->SetPercent(Current / Max);
}
