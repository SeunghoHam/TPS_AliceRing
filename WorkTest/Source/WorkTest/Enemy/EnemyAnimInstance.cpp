// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/EnemyAnimInstance.h"

void UEnemyAnimInstance::ChangeAnimState(const EEnemyAnimState& state)
{
	this->AnimState = state;
}
