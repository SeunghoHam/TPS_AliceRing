// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AliceBase.h"
#include "GameFramework/Character.h"
#include "ZZZEnemy.generated.h"

class AAlice;
struct FTimerHandle;
class UEnemyRegistrySubsystem;
UCLASS()
class WORKTEST_API AZZZEnemy : public AAliceBase
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AZZZEnemy();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
public:	
	// Called every frame
	//virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	void SetOutLineEnable(bool _enable);
	void SetCharacter(AActor* _actor);
	
	bool CanCounterAttack(){ return bCanCounter; }
	bool bCanCounter =false; // 이거 설정 필요


	UFUNCTION(BlueprintCallable,BlueprintPure)
	AAlice* GetAlice(){return Target;}
protected:
	AAlice* Target;
	UEnemyRegistrySubsystem* RS;
	void RegistryEnemy();
	void UnregisterEnemy();
private:

	

	



};
