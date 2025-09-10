// Fill out your copyright notice in the Description page of Project Settings.


#include "MultiSystem/Widget/InGameMenu.h"
#include "Components/Button.h"

bool UInGameMenu::Initialize()
{
	bool Success = Super::Initialize();
	if (!Success) return false;
	
	CancelButton->OnClicked.AddDynamic(this, &UInGameMenu::CancelPressed);
	QuitButton->OnClicked.AddDynamic(this, &UInGameMenu::QuitPressed);
	return true;
}

void UInGameMenu::CancelPressed()
{
	TearDown();
}

void UInGameMenu::QuitPressed()
{
	if (MenuInterface != nullptr)
	{
		TearDown();
		MenuInterface->LoadMainMenu();
	}
}
