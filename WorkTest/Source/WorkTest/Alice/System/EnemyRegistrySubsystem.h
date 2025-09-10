// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ZZZEnemy.h"

#include "EnemyRegistrySubsystem.generated.h"
/**
 * 
 */

UCLASS()
class WORKTEST_API UEnemyRegistrySubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	protected:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
public:
	void RegisterEnemy(AZZZEnemy* E)   { Enemies.Add(TWeakObjectPtr<AZZZEnemy>(E)); }
	void UnregisterEnemy(AZZZEnemy* E) { Enemies.Remove(TWeakObjectPtr<AZZZEnemy>(E)); }
	const TArray<TWeakObjectPtr<AZZZEnemy>>& GetEnemies() const { return Enemies; }

private:
	UPROPERTY() TArray<TWeakObjectPtr<AZZZEnemy>> Enemies;
};
