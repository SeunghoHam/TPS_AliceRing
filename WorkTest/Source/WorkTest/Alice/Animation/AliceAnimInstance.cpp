// Fill out your copyright notice in the Description page of Project Settings.


#include "Alice/Animation/AliceAnimInstance.h"

void UAliceAnimInstance::SetAliceAnimState(const EAliceAnimState& _newState)
{
	AliceAnimState = _newState;
	//GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Blue,FString::Printf(TEXT("Current State Change")));
}

