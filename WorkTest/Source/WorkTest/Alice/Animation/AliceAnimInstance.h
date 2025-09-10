// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "AliceAnimInstance.generated.h"

/**
 * 
 */
UENUM(BlueprintType)
enum class EAliceAnimState : uint8
{
	Idle,
	Dash,
	Walk,
	A,
	B,
	C,
	D,
	Parring,
	Recover,
	GoChargeMode,
	ChargeAttackReady,
	ChargeAttack,
	Avoid,
	Hit,
	Death
};
UCLASS()
class WORKTEST_API UAliceAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	//UFUNCTION(BlueprintCallable)
	void SetAliceAnimState(const EAliceAnimState& _newState);
	//UFUNCTION(BlueprintCallable,BlueprintPure) EAliceAnimState GetAliceAnimState();

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	EAliceAnimState AliceAnimState;
private:

};
