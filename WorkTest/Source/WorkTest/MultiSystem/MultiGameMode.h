// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "MultiGameMode.generated.h"

/**
 * 
 */
UCLASS()
class WORKTEST_API AMultiGameMode : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	void PostLogin(APlayerController* NewPlayer) override;;
	void Logout(AController* Exiting) override;
private:
	void StartGame();
	 uint32 NumberOfPlayer=0;
	FTimerHandle GameStartTimer;
};
