// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MenuInterface.h"
#include "MenuWidget.generated.h"

/**
 * 
 */
UCLASS()
class WORKTEST_API UMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetUp(); // 팝업창 열기
	void TearDown(); // 팝업창 내리기
	
	void SetMenuInterface(IMenuInterface* _MenuInterface);

protected:
	IMenuInterface* MenuInterface; // MainMenu에서 사용할예정( MenuWidget을 상속받음)
};
