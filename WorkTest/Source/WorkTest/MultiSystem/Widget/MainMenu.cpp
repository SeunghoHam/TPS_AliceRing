// Fill out your copyright notice in the Description page of Project Settings.

#include "MainMenu.h"
#include "MultiSystem/Widget/MainMenu.h"
//#include "UObject/ConstructorHelpers.h" // blueprint 찾기용
#include "Components/Button.h"
#include "Components/EditableTextBox.h"
#include "Components/TextBlock.h"
#include "Components/WidgetSwitcher.h"

#include "ServerRow.h"


UMainMenu::UMainMenu(const FObjectInitializer& ObjectInitializer)
{
	if (!Template_ServerRowClass) return;
	 ServerRowClass = Template_ServerRowClass.GetClass(); // 이렇게 가져와도 좋은데

	//static ConstructorHelpers::FClassFinder<UUserWidget> WBP_ServerRow(TEXT("/Game/Widget/WBP_ServerRow"));
	//if (WBP_ServerRow.Class != nullptr) ServerRowClass = WBP_ServerRow.Class;
}
bool UMainMenu::Initialize()
{
	bool Success = Super::Initialize();

	HostButton->OnClicked.AddDynamic(this, &ThisClass::OpenHostMenu);
	JoinButton->OnClicked.AddDynamic(this, &ThisClass::OpenJoinMenu);
	QuitButton->OnClicked.AddDynamic(this, &ThisClass::QuitPressed);

	ConfirmHostMenuButton->OnClicked.AddDynamic(this, &ThisClass::HostServer);	
	CancelHostMenuButton->OnClicked.AddDynamic(this, &ThisClass::OpenMainMenu);	

	ConfirmJoinMenuButton->OnClicked.AddDynamic(this, &ThisClass::JoinServer);	
	CancelJoinMenuButton->OnClicked.AddDynamic(this, &ThisClass::OpenMainMenu);	
	return true;
}

void UMainMenu::SetServerList(TArray<FServerData> ServerNames)
{
	UWorld* World = GetWorld();
	if (!ensure(World != nullptr))
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("서버리스트 가져오는데 월드가 없으셈"));
		return;
	}
	ServerList->ClearChildren();

	uint32 i =0;
	if (ServerRowClass == nullptr)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, TEXT("[MainMenu] : ServerRowClass = null"));		
		return;
	}
	
	for ( const FServerData& ServerData : ServerNames)
	{
		
		UServerRow* Row = CreateWidget<UServerRow>(World, ServerRowClass);

		Row->ServerName->SetText(FText::FromString(ServerData.Name));
		Row->HostUser->SetText(FText::FromString(ServerData.HostUsername));

		FString FractionText = FString::Printf(TEXT("%d/%d"), ServerData.CurrentPlayers, ServerData.MaxPlayers);
		Row->ConnectionFraction->SetText(FText::FromString(FractionText));
		Row->Setup(this, i);
		++i;

		ServerList->AddChild(Row);
	}
}

void UMainMenu::SelectIndex(uint32 Index)
{
	SelectedIndex = Index;
	UpdateChildren();
}


void UMainMenu::UpdateChildren()
{
	for (int32 i = 0; i < ServerList->GetChildrenCount(); ++i)
	{
		auto Row = Cast<UServerRow>(ServerList->GetChildAt(i));
		if (Row != nullptr)
		{
			Row->Selected = (SelectedIndex.IsSet() && SelectedIndex.GetValue() == i);
		}
	}
}

void UMainMenu::HostServer()
{
	GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Blue, TEXT("MainMenu : HostServer"));
	if (MenuInterface != nullptr)
	{
		FString ServerName = ServerHostName->Text.ToString(); //.ToString();
		MenuInterface->Host(ServerName);
		//GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Blue, TEXT("MainMenu : HostServer"));
		TearDown();
	}
	
}

void UMainMenu::JoinServer()
{
	if (SelectedIndex.IsSet() && MenuInterface != nullptr)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, TEXT("JoinServer"));
		MenuInterface->Join(SelectedIndex.GetValue());
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("Selected Index Not Set"));
	}
}

void UMainMenu::OpenHostMenu()
{
	MenuSwitcher->SetActiveWidget(HostMenu);
}

void UMainMenu::OpenJoinMenu()
{
	MenuSwitcher->SetActiveWidget(JoinMenu);
	if (MenuInterface!= nullptr	)
	{
		MenuInterface->RefreshServerList();
	}
}

void UMainMenu::OpenMainMenu()
{
	MenuSwitcher->SetActiveWidget(MainMenu);
}

void UMainMenu::QuitPressed()
{
	UWorld* World = GetWorld();
	
	APlayerController* PlayerController = World->GetFirstPlayerController();
	PlayerController->ConsoleCommand("quit");
}

