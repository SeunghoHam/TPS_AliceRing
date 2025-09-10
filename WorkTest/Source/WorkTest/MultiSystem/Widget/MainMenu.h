// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MenuWidget.h"
#include "Blueprint/UserWidget.h"
#include "MainMenu.generated.h"

/**
 * 
 */

USTRUCT()
struct FServerData // 온랴라인세션에서 인식에 사용할 데이터들
{
	GENERATED_BODY()
	
	FString Name;
	uint16 CurrentPlayers;
	uint16 MaxPlayers;
	FString HostUsername;
};
UCLASS()
class WORKTEST_API UMainMenu : public UMenuWidget // WidgetMenu를 할당받음
{
	GENERATED_BODY()
public:
	// 생성자
	UMainMenu(const FObjectInitializer & ObjectInitializer);
	
	void SetServerList(TArray<FServerData> ServerNames);

	void SelectIndex(uint32 Index);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Multi)
	//TObjectPtr<class UUserWidget> Template_ServerRowClass;
	TObjectPtr<class UServerRow> Template_ServerRowClass;
protected:
	virtual bool Initialize();


	
private:
	TSubclassOf<class UServerRow> ServerRowClass; // WBP에서 띄워질 ServerRow
	//UServerRow* ServerRowClass;

	UPROPERTY(meta = (BindWidget))
	class UButton* HostButton;

	UPROPERTY(meta = (BindWidget))
	class UButton* JoinButton;

	UPROPERTY(meta = (BindWidget))
	class UButton* QuitButton;

	
	UPROPERTY(meta = (BindWidget))
	class UButton* CancelJoinMenuButton;

	UPROPERTY(meta = (BindWidget))
	class UButton* ConfirmJoinMenuButton;

	
	UPROPERTY(meta = (BindWidget))
	class UWidgetSwitcher* MenuSwitcher;


	/* Memu */
	UPROPERTY(meta = (BindWidget))
	class UWidget* MainMenu;

	UPROPERTY(meta = (BindWidget))
	class UWidget* JoinMenu;

	UPROPERTY(meta = (BindWidget))
	class UWidget* HostMenu;


	
	UPROPERTY(meta = (BindWidget))
	class UEditableTextBox* ServerHostName;
	
	UPROPERTY(meta = (BindWidget))
	class UButton* CancelHostMenuButton;

	UPROPERTY(meta = (BindWidget))
	class UButton* ConfirmHostMenuButton;
	
	UPROPERTY(meta = (BindWidget))
	class UPanelWidget* ServerList;

	UFUNCTION()
	void HostServer();

	UFUNCTION()
	void JoinServer();
	
	UFUNCTION()
	void OpenHostMenu();

	UFUNCTION()
	void OpenJoinMenu();

	UFUNCTION()
	void OpenMainMenu();
	
	UFUNCTION()
	void QuitPressed();
	
	TOptional<uint32> SelectedIndex;

	void UpdateChildren();
};
