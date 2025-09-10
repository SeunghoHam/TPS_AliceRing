// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MenuInterface.h" // 인터페이스의 기능을 사용함
#include "Engine/GameInstance.h"

#include "OnlineSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "MenuInterface.h"

#include "MultiGameInstance.generated.h"
/**
 * 
 */

class UInGameMenu;
class UMainMenu;
//class UEnemyRegistrySubsystem;
UCLASS()
class WORKTEST_API UMultiGameInstance : public UGameInstance , public IMenuInterface 
{
	GENERATED_BODY()
public:
// 게임 인스턴스에서 인터페이스에서 선언한 함수도 정의해줌

	// /FObjectInitializer 는 뭘하는 친구일까?
	UMultiGameInstance(const FObjectInitializer& ObjectInitialzier);
	
	virtual void Init();

	UFUNCTION(BlueprintCallable)
	void LoadMenuWidget();

	UFUNCTION(BlueprintCallable)
	void InGameLoadMenu();

	UFUNCTION(Exec) // 이게 ` 로 실행시키는 함수
	void Host(FString ServerName) override;

	UFUNCTION(Exec)
	void Join(uint32 Index) override;
	void StartSession();

	virtual void LoadMainMenu() override;

	void RefreshServerList() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Template)
	//TSubclassOf<class UUserWidget> Template_MenuClass;
	TObjectPtr<UMainMenu> Template_MenuClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Template)
	//TSubclassOf< UUserWidget> Template_InGameMenuClass;
	TObjectPtr< UInGameMenu> Template_InGameMenuClass;
	
private:
	TSubclassOf<UUserWidget> MenuClass;
	TSubclassOf<UInGameMenu> InGameMenu;
	//TSubclassOf<class UUserWidget> InGameMenuClass;

	class UMainMenu* MainMenu;

	IOnlineSessionPtr SessionInterface;
	TSharedPtr<class FOnlineSessionSearch> SessionSearch;

	void OnCreateSessionComplete(FName SessionName, bool Success);
	void OnDestroySessionComplete(FName SessionName, bool Success);
	void OnFindSessionsComplete(bool Success);
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
	void OnNetworkFailure(UWorld* World, UNetDriver* NetDriver, ENetworkFailure::Type FailureType, const FString& ErrorString);

	FString DesiredServerName;
	void CreateSession();


};

