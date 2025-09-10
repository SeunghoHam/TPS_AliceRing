// Fill out your copyright notice in the Description page of Project Settings.


#include "MultiSystem/Widget/MenuWidget.h"
#include "MultiSystem/MenuInterface.h"
#include "AnbyController.h"

// #include "Framework/Application/SlateApplication.h"


void UMenuWidget::SetUp()
{
	this->AddToViewport();

	UWorld* World = GetWorld();
	APlayerController* PlayerController = World->GetFirstPlayerController();
	
	FInputModeUIOnly InputModeData;
	InputModeData.SetWidgetToFocus(this->TakeWidget());
	InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);

	PlayerController->SetInputMode(InputModeData);
	PlayerController->bShowMouseCursor = true;

	//PlayerController->SetIgnoreLookInput(true);
	//PlayerController->SetIgnoreMoveInput(true);

	GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, TEXT("MenuWidget : Setup"));
}
void UMenuWidget::TearDown()
{
	//this->RemoveFromViewport(); // 이거 원래 활성화되어있음
	UWorld* World =GetWorld();

	APlayerController* PlayerController = World->GetFirstPlayerController();

	FInputModeGameOnly InputModeData;
	PlayerController->SetInputMode(InputModeData);
	PlayerController->bShowMouseCursor =  false;

	PlayerController->SetIgnoreLookInput(false);
	PlayerController->SetIgnoreMoveInput(false);
	
	GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, TEXT("MenuWidget : TearDown"));
	
}

void UMenuWidget::SetMenuInterface(IMenuInterface* _MenuInterface)
{
	this->MenuInterface = _MenuInterface;
}
