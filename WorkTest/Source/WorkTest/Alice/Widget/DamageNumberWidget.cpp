// Fill out your copyright notice in the Description page of Project Settings.


#include "Alice/Widget/DamageNumberWidget.h"

#include "Engine/Engine.h"
#include "TimerManager.h"

#include "Blueprint/UserWidget.h"    // UUserWidget

#include "Components/TextBlock.h"    // UTextBlock (SetIsVolatile, SetRender~는 UWidget에서 옴)
#include "Components/HorizontalBox.h"// UHorizontalBox
#include "Components/HorizontalBoxSlot.h"
#include "Curves/CurveFloat.h"       // UCurveFloat
#include "Fonts/SlateFontInfo.h"     // FSlateFontInfo (중요: 이거 없으면 UHT 에러)

#include "Engine/Font.h"
//#include "Kismet/GameplayStatics.h"

void UDamageNumberWidget::NativeConstruct()
{
	Super::NativeConstruct();
	//if (HB_Digits) HB_Digits->SetIsVolatile(true); // 매 프레임 변환 갱신 안전


}

void UDamageNumberWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	
	if (DigitWidgets.Num() == 0) return;

	const float Now = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;

	for (int32 i = 0; i < DigitWidgets.Num(); ++i)
	{
		UTextBlock* TB = DigitWidgets[i];
		if (!IsValid(TB)) continue;

		const float Elapsed = Now - DigitStartTimes[i];
		if (Elapsed < 0.f)
		{
			// 시작 전: 대기
			continue;
		}

		if (Elapsed <= PopDuration)
		{
			const float T01 = FMath::Clamp(Elapsed / PopDuration, 0.f, 1.f);
			const float Alpha = EvalPop01(T01);          // 0~1
			const float Y = -PopAmplitude * Alpha;       // 위로 팝
			TB->SetRenderTranslation(FVector2D(0.f, Y));
		}
		else
		{
			// 끝: 원위치
			TB->SetRenderTranslation(FVector2D::ZeroVector);
		}
	}
}



void UDamageNumberWidget::SetDamage(float _value, bool _bCrit, FLinearColor _color)
{
	if (!HB_Digits) return;

	// 상태 초기화
	HB_Digits->ClearChildren();
	DigitWidgets.Reset();
	DigitStartTimes.Reset();

	bCurrentCrit = _bCrit;
	CurrentColor = _bCrit ? FLinearColor(1.f, 0.85f, 0.2f) : _color;

	const FString Str = FString::FromInt(FMath::RoundToInt(_value));
	StartTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;

	// 글자별 TextBlock 동적 생성
	for (int32 i = 0; i < Str.Len(); ++i)
	{
		UTextBlock* TB = NewObject<UTextBlock>(this, UTextBlock::StaticClass());
		if (!TB) continue;

		//TB->SetIsVolatile(true);
		TB->SetText(FText::FromString(FString::Chr(Str[i])));
		TB->SetColorAndOpacity(CurrentColor);
		
		if (DigitFont.FontObject)
		{
		TB->SetFont(DigitFont);
		}
		else
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("[DamageNumberWidget] Font is null"));
		}
		
		// 크리티컬이면 살짝 확대
		TB->SetRenderTransformPivot(FVector2D(0.5f, 1.f));
		TB->SetRenderScale(bCurrentCrit ? FVector2D(1.15f, 1.15f) : FVector2D(1.f, 1.f));

		// HorizontalBox에 추가
		if (UHorizontalBoxSlot* part = HB_Digits->AddChildToHorizontalBox(TB))
		{
			//Slot->SetPadding((FMargin(0)));
			part->SetPadding(FMargin(0));
			part->SetHorizontalAlignment(HAlign_Center);
			part->SetVerticalAlignment(VAlign_Bottom);
		}

		DigitWidgets.Add(TB);
		DigitStartTimes.Add(StartTime + i * StaggerPerChar);
	}
}

float UDamageNumberWidget::EvalPop01(float T01) const
{
	if (PopCurve)
	{
		return PopCurve->GetFloatValue(T01); // (권장) 0~1→0~1 팝 곡선
	}
	// 곡선이 없으면 단순 반주기 사인
	return FMath::Sin(T01 * PI); // 0→1→0
}
