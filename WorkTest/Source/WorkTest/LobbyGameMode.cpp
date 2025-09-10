// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyGameMode.h"

#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"

void ALobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	if (GameState) // GameState는 TObjectPtr 타입
	{
		int32 NumberOfPlayers  = GameState.Get()->PlayerArray.Num(); // 포인터 반환 // 플레이어가 들어올떄마다 업데이트
		if (GEngine)
		{
			// Add Screen~ 1 파라미터에 1 넣으면
			// 새 메세지를 추가할 때 이전 키로 대체
			GEngine->AddOnScreenDebugMessage(1,
				60.0f,
				FColor::Yellow,
				FString::Printf(TEXT("Players In game : %d"), NumberOfPlayers));


			APlayerState* PlayerState =  NewPlayer->GetPlayerState<APlayerState>();
			if (PlayerState)
			{
				FString PlayerName =  PlayerState->GetPlayerName();
				GEngine->AddOnScreenDebugMessage(-1,
	60.0f,
				FColor::Cyan,
	FString::Printf(TEXT("%s Join Game"),*PlayerName));
			}
	
		}
		
	}
}

void ALobbyGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);
	
	APlayerState* PlayerState =  Exiting->GetPlayerState<APlayerState>();
	if (PlayerState)
	{
		int32 NumberOfPlayers = GameState.Get()->PlayerArray.Num();
		GEngine->AddOnScreenDebugMessage(1,
		60.0f,
		FColor::Yellow,
		FString::Printf(TEXT("Players In game : %d"), NumberOfPlayers));
		//int32 NumberOfPlayers  = GameState.Get()->PlayerArray.Num(); // 포인터 반환 // 플레이어가 들어올떄마다 업데이트
		
		FString PlayerName =  PlayerState->GetPlayerName();
		GEngine->AddOnScreenDebugMessage(-1,
60.0f,
		FColor::Cyan,
FString::Printf(TEXT("%s Join Game"),*PlayerName));
	}
}
