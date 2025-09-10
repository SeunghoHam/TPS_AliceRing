// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "RevenantAnimInstance.generated.h"

/**
 * 
 */
UENUM(BlueprintType)
enum class ERevenantAnimState : uint8
{
	Idle_NoWeapon,
	Idle_Weapon,
	Attack_Slow, // Primary_Fire_Slow
	Cast, // 트랩 발동
	CastChargingStart,
	Throw, // 수류탄 던지기
	Hit,// 피격
	Reload,
	LevelStart, // 등장
	StunStart,
	Idle_Stun,
	Emote, // 화면연출용
	Jump,
	Death
};
UCLASS()
class WORKTEST_API URevenantAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	ERevenantAnimState CurrentAnimState=  ERevenantAnimState::Idle_NoWeapon;

	UFUNCTION(BlueprintCallable)
	void SetAnimState(ERevenantAnimState _state);
};
