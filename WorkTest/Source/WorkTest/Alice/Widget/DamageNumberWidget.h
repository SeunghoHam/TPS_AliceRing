// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Engine/Font.h"
#include "DamageNumberWidget.generated.h"

/**
 */

class UHorizontalBox;
class UTextBlock;
class UFont;
UCLASS()
class WORKTEST_API UDamageNumberWidget : public UUserWidget
{
	GENERATED_BODY()

	protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
public:


	
	//UPROPERTY(meta=(BindWidget)) UTextBlock* TB_Digit;
	
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void PlayPopAnim();

	// 디자이너에서 바인딩
	UPROPERTY(meta=(BindWidget)) UHorizontalBox* HB_Digits;

	// 옵션 파라미터
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Damage|Anim")
	float PopDuration = 0.30f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Damage|Anim")
	float PopAmplitude = 20.f; // 8

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Damage|Anim")
	float StaggerPerChar = 0.02f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Damage|Font")
	FSlateFontInfo DigitFont;

	// 곡선(0~1 입력 → 0~1 출력). 없으면 Sine 사용
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Damage|Anim")
	UCurveFloat* PopCurve = nullptr;
	
	UFUNCTION()
	void SetDamage(float _value, bool _bCrit, FLinearColor _color);
	
private:
	// 동적으로 만든 글자들과 각 글자의 시작시간
	UPROPERTY() TArray<UTextBlock*> DigitWidgets;
	TArray<float> DigitStartTimes;

	bool bCurrentCrit = false;
	FLinearColor CurrentColor = FLinearColor::White;
	float StartTime = 0.f;

	float EvalPop01(float T01) const; // 0~1 → 0~1

};
