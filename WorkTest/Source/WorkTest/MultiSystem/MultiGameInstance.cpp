// Fill out your copyright notice in the Description page of Project Settings.


#include "MultiSystem/MultiGameInstance.h"

#include "Engine/Engine.h"
#include "UObject/ConstructorHelpers.h"

#include "OnlineSessionSettings.h"
#include "Interfaces/OnlineSessionInterface.h" //  GetNamedSession 의 SessionName
#include "OnlineSessionSettings.h"
#include "Blueprint/UserWidget.h"
#include "Online/OnlineSessionNames.h" // SEARCH_PRESENCE 에서 사용 
#include "ProfilingDebugging/BootProfiling.h"


#include "Widget/MainMenu.h"
//#include "MultiSystem/"

const static FName SESSION_NAME = TEXT("Game");
const static FName SERVER_NAME_SETTINGS_KEY = TEXT("ServerName");

UMultiGameInstance::UMultiGameInstance(const FObjectInitializer& ObjectInitialzier)
{

	//GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, TEXT("게임 인스턴스 생성자"));

	//static ConstructorHelpers::FClassFinder<UUserWidget> WBP_MainMenu(TEXT("/Game/Widget/WBP_MainMenu"));
	//if (WBP_MainMenu.Class != nullptr) MenuClass = WBP_MainMenu.Class;

	//static ConstructorHelpers::FClassFinder<UUserWidget> WBP_InGameMenu(TEXT("/Game/Widget/WBP_MainMenu"));
	//if (WBP_InGameMenu.Class != nullptr) MenuClass = WBP_InGameMenu.Class;
	if (!Template_MenuClass) return;
	MenuClass = Template_MenuClass.GetClass(); // 이렇게 가져와도 좋은데
	if (!Template_InGameMenuClass) return;
	InGameMenu= Template_InGameMenuClass.GetClass();
}

void UMultiGameInstance::Init()
{
	/*
	if (Template_InGameMenuClass)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Blue, TEXT("GetInGameMenu"));
		InGameMenuClass = Template_InGameMenuClass;
		
	}
	if (Template_MenuClass)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Blue, TEXT("GetMainMenu"));
		MenuClass = Template_MenuClass;
	}

	
*/
	
	//Super::Init();
	IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
	if (Subsystem!= nullptr)
	{
		SessionInterface = Subsystem->GetSessionInterface();
		if (SessionInterface.IsValid())
		{
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("Subsystem Delegate Complete"));
			SessionInterface->OnCreateSessionCompleteDelegates.AddUObject(this, &UMultiGameInstance::OnCreateSessionComplete);
			SessionInterface->OnDestroySessionCompleteDelegates.AddUObject(this, &UMultiGameInstance::OnDestroySessionComplete);
			SessionInterface->OnFindSessionsCompleteDelegates.AddUObject(this, &UMultiGameInstance::OnFindSessionsComplete);
			SessionInterface->OnJoinSessionCompleteDelegates.AddUObject(this, &UMultiGameInstance::OnJoinSessionComplete);
		}
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("Subsystem Is No"));
	}
	if (GEngine!= nullptr)
	{
		GEngine->OnNetworkFailure().AddUObject(this, &UMultiGameInstance::OnNetworkFailure);
	}
	
}


void UMultiGameInstance::LoadMenuWidget()
{
	if (!IsValid(MenuClass))
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("MenuClass is Null"));
		return;
	}
	
	MainMenu = CreateWidget<UMainMenu>(this, MenuClass);
	MainMenu->SetUp();
	MainMenu->SetMenuInterface(this);
}

void UMultiGameInstance::InGameLoadMenu()
{
	UE_LOG(LogTemp, Log, TEXT("GameLoadMenu"));
	//UMenuWidget* GameWidget = CreateWidget<UMenuWidget>(this, InGameMenuClass);
	//GameWidget->SetUp();
	//GameWidget->SetMenuInterface(this);
}

void UMultiGameInstance::Host(FString ServerName)
{
	DesiredServerName = ServerName;
	if (SessionInterface.IsValid())
	{
		auto ExistingSession = SessionInterface->GetNamedSession(SESSION_NAME);
		if (ExistingSession != nullptr)
		{
			GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, TEXT("게임인스턴스 : 세션이 있어서 지울거임"));
			SessionInterface->DestroySession(SESSION_NAME);
		}
		else
		{
			GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, TEXT("게임인스턴스 :: CreateSesison "));
			CreateSession();
		}
	}
}

void UMultiGameInstance::Join(uint32 Index)
{
	if (!SessionSearch.IsValid()) return;
	if (MainMenu != nullptr) 
	{
		MainMenu->TearDown();
	}
	SessionInterface->JoinSession(0, SESSION_NAME, SessionSearch->SearchResults[Index]);
}

void UMultiGameInstance::StartSession()
{
	SessionInterface->StartSession(SESSION_NAME);
}

void UMultiGameInstance::LoadMainMenu()
{
	APlayerController* PlayerController = GetFirstLocalPlayerController();

	GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Green,TEXT("게임 인스턴스 : LoadMainMenu"));
	//PlayerController->ClientTravel("/Game/")
	//PlayerController->ClientTravel();
}

void UMultiGameInstance::RefreshServerList()
{
	SessionSearch = MakeShareable(new FOnlineSessionSearch());
	if (SessionSearch.IsValid())
	{
		//SessionSearch->bIsLanQuery = true;
		SessionSearch->MaxSearchResults = 100;
		SessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);
		UE_LOG(LogTemp, Warning, TEXT("Starting Find Session"));
		SessionInterface->FindSessions(0, SessionSearch.ToSharedRef());
	}
}

void UMultiGameInstance::OnCreateSessionComplete(FName SessionName, bool Success)
{
	if (!Success)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, TEXT("Could Not Create Session"));
		return;
	}
	if (MainMenu != nullptr)
	{
		MainMenu->TearDown();
	}
	//UEngine* Engine = GetEngine();
	UWorld* World = GetWorld();
	World->ServerTravel("/Game/Maps/Lobby?listen");
	GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, TEXT("게임 인스턴스 : OnCreateSessionComplete"));
}

void UMultiGameInstance::OnDestroySessionComplete(FName SessionName, bool Success)
{
	if (Success)
	{
		CreateSession();
	}
}

void UMultiGameInstance::OnFindSessionsComplete(bool Success)
{
	if (Success && SessionSearch.IsValid() && MainMenu != nullptr)
	{
		// Finish FIne session
		TArray<FServerData> ServerNames;
		for (const FOnlineSessionSearchResult& SearchResult : SessionSearch->SearchResults)
		{
			GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red,
				FString::Printf(TEXT("Found Session Name : %s"), *SearchResult.GetSessionIdStr()));


			FServerData Data;
			Data.MaxPlayers = SearchResult.Session.SessionSettings.NumPublicConnections;
			Data.CurrentPlayers = Data.MaxPlayers - SearchResult.Session.NumOpenPublicConnections;
			Data.HostUsername = SearchResult.Session.OwningUserName;
			FString ServerName;

			if (SearchResult.Session.SessionSettings.Get(SERVER_NAME_SETTINGS_KEY, ServerName))
			{
				Data.Name = ServerName;
			}
			else
			{
				Data.Name = "Can't Find Name";
			}
			ServerNames.Add(Data);
		}
		MainMenu->SetServerList(ServerNames);
	}
}

void UMultiGameInstance::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	if (!SessionInterface.IsValid())return;

	FString Address;
	if (!SessionInterface->GetResolvedConnectString(SessionName, Address))
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red,TEXT("GameInstnace::OnJoinComplete , 커넥션 스트링을 받을 수 없음"));
		return;
	}
	UEngine* Engine = GetEngine();
	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Blue,FString::Printf(TEXT("Joining %s"), *Address));

	APlayerController* PlayerController = GetFirstLocalPlayerController();
	PlayerController->ClientTravel(Address, ETravelType::TRAVEL_Absolute);

}

void UMultiGameInstance::OnNetworkFailure(UWorld* World, UNetDriver* NetDriver, ENetworkFailure::Type FailureType,
	const FString& ErrorString)
{
	// 네크워크 연결 실패하면 메인화면으로 감
	LoadMainMenu();
}

void UMultiGameInstance::CreateSession()
{
	if (SessionInterface.IsValid())
	{
		FOnlineSessionSettings SessionSettings;
		if (IOnlineSubsystem::Get()->GetSubsystemName() == "NULL") // 서브시스템 이름이 NULL. 즉 스팀연결이 아닌 로컬환경
		{
			GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Black, TEXT("NULL"));
			SessionSettings.bIsLANMatch = true;
		}
		else
		{
			GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Black, TEXT("Steam"));
			SessionSettings.bIsLANMatch = false;
		}
		SessionSettings.NumPublicConnections = 5;
		SessionSettings.bShouldAdvertise = true;
		SessionSettings.bUsesPresence = true;
		SessionSettings.Set(SERVER_NAME_SETTINGS_KEY, DesiredServerName, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);

		SessionInterface->CreateSession(0, SESSION_NAME, SessionSettings);
	}
}
