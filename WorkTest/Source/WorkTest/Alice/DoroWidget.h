// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DoroWidget.generated.h"

/**
 * 
 */
 class UProgressBar;
UCLASS()
class WORKTEST_API UDoroWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(meta=(BindWidget))
	UProgressBar* HpBar;

	void HPChange(float Current, float Max);
};
