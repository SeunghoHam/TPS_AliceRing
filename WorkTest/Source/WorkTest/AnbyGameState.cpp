// Fill out your copyright notice in the Description page of Project Settings.


#include "AnbyGameState.h"

#include "Net/UnrealNetwork.h"

void AAnbyGameState::ApplyBossDamage(float _damage)
{
	if (!HasAuthority()) return;
	BossCurrnetHP = FMath::Clamp(BossCurrnetHP - _damage, 0.f, BossMaxHP);
	GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Green,TEXT("OnRep 자동실행"));

	// 서버의 로컬 UI도 갱신되게 직접실행함
	OnBossHPChanged.Broadcast(BossCurrnetHP, BossMaxHP);
	
}

void AAnbyGameState::OnRep_BossHP()
{
	OnBossHPChanged.Broadcast(BossCurrnetHP,BossMaxHP);
	GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green,TEXT("GS-OnRep_BosshHP"));
	
	UE_LOG(LogTemp, Warning, TEXT("CurrentHP: %f"), BossCurrnetHP);
	UE_LOG(LogTemp, Warning, TEXT("NaxHP: %f"), BossMaxHP);
	
	// CurrnetHP값이 변경되면, OnRep_BossHP가 실행되고.
	// OnBossHPChanged 브로드캐스트를 실행시키면서 Widget에서 값을 받고, UI를 변경시킴
}

void AAnbyGameState::OnRep_SetBoss()
{
	OnTargetBossChanged.Broadcast(CurrentTargetEnemy);
	GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green,TEXT("GS-OnRep_SetBoss"));
}

void AAnbyGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AAnbyGameState, BossMaxHP);
	DOREPLIFETIME(AAnbyGameState, BossCurrnetHP);
	
	DOREPLIFETIME(AAnbyGameState, CurrentTargetEnemy);
	
}
