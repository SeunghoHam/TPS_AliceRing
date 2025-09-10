// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "EnemyAnimInstance.generated.h"


UENUM(BlueprintType)
enum class EEnemyAnimState : uint8 // : uint = 8bit
{
	Locomotion = 0,
	Attack,
	Hit,
	Dead,
};

UCLASS()
class WORKTEST_API UEnemyAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	EEnemyAnimState AnimState; // In Editor ABP
	
	UFUNCTION(BlueprintCallable)
	void ChangeAnimState(const EEnemyAnimState& state);
};
