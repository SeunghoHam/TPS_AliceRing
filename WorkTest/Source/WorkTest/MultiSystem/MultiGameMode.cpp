// Fill out your copyright notice in the Description page of Project Settings.


#include "MultiSystem/MultiGameMode.h"
#include "TimerManager.h"
#include "MultiGameInstance.h"


void AMultiGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	if (NumberOfPlayer >= 2)
	{
		GetWorldTimerManager().SetTimer(GameStartTimer, this, &AMultiGameMode::StartGame, 5);
	}
}
void AMultiGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);
	--NumberOfPlayer;
}

void AMultiGameMode::StartGame()
{
	auto GameInstance = Cast<UMultiGameInstance>(GetGameInstance());
	if (GameInstance == nullptr) return;

	UWorld* World = GetWorld();

	bUseSeamlessTravel = true;
	World->ServerTravel("/Game/Maps/Grapple?Listen");
	GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, TEXT(" MultiGameMode : StartGame : 서버 트레블->Grapple "));
}


