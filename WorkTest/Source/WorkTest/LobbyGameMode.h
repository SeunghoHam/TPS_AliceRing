// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "LobbyGameMode.generated.h"

/**
 * 
 */
UCLASS()
class WORKTEST_API ALobbyGameMode : public AGameModeBase
{
	GENERATED_BODY()
public:
	// 로그인 파라미터는 PlayerController,
	// 로그아웃 파라미터는 Controller
	virtual void PostLogin(APlayerController* NewPlayer) override; // 플레이어컨트롤러에서 복제된 함수를 호출하는 첫 번째 장소

	virtual void Logout(AController* Exiting) override;

	
};
