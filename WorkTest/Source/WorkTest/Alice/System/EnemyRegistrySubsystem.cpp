// Fill out your copyright notice in the Description page of Project Settings.


#include "Alice/System/EnemyRegistrySubsystem.h"
#include "Engine/GameInstance.h"

void UEnemyRegistrySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UE_LOG(LogTemp, Warning, TEXT("[Registry] Initialize GI=%s"),
	*GetGameInstance()->GetName());
}

void UEnemyRegistrySubsystem::Deinitialize()
{
	UE_LOG(LogTemp, Warning, TEXT("[Registry] Deinitialize"));
	Super::Deinitialize();
}
