// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "AnbyGameState.generated.h"

/**
 * 
 */
class AZZZEnemy;
// 멀티캐스트로 할당해야댐
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnBossHPChangedSignature, float ,Current,float ,Max);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBossSettingSignature, AZZZEnemy* ,Boss);
UCLASS()
class WORKTEST_API AAnbyGameState : public AGameStateBase
{
	GENERATED_BODY()
public:


	// 서버에서 체력 감소 요청이 들어옴
	void ApplyBossDamage(float _damage);
	
	// BossCurrentHP가 변경되면 OnRep 함수가 실행됨
	// OnRep함수에서는 브로드캐스트로 다 전파시킴
	UPROPERTY(ReplicatedUsing=OnRep_BossHP) 
	float BossCurrnetHP;

	UFUNCTION()
	void OnRep_BossHP();
	
	UPROPERTY(Replicated)
	float BossMaxHP;


	void SetBossHP(float NewHP);
	// UI 바인딩용 델리게이트
	UPROPERTY(BlueprintAssignable)
	FOnBossHPChangedSignature OnBossHPChanged; // 델리게트 기반으로 할당

	// 보스 이름 설정하기
	UPROPERTY(BlueprintAssignable)
	FOnBossSettingSignature OnTargetBossChanged;

	UPROPERTY(ReplicatedUsing = OnRep_SetBoss)
	AZZZEnemy* CurrentTargetEnemy;
	
	UFUNCTION()
	void OnRep_SetBoss();
	
private:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	//FName* BossName;
};
