// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseCharacter.h"
#include "GameFramework/Character.h"
#include "BaseEnemy.generated.h"

class UEnemyAnimInstance;
class AAnby; // 표적 설정
class UStatusComponent;
class USkeletalMeshComponent;
/**
 * 
 */
UCLASS()
class WORKTEST_API ABaseEnemy : public ABaseCharacter//public AActorBase, public ACharacter
{
	GENERATED_BODY()

	ABaseEnemy();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
private:
	FTimerHandle TimerHandle;
	//void TestAttack();
	void StopTimer();

public:

	void TakeDamageFromCombat(float _damage);



	
	UFUNCTION(BlueprintCallable)
	void TestDamaged(float _damage);
	
	// 보스체력 동기화
	UPROPERTY(ReplicatedUsing = OnRep_CurrnetHP)
	float _bossCurrentHP;

	UPROPERTY(Replicated)
	float _bossMaxHP;
	
	UFUNCTION()
	void OnRep_CurrnetHP();


	// 클라에서 발생 시 
	UFUNCTION(Reliable, Server)
	void ServerHitEnemy(float _damage);
	
	// BossHPWidget에서 이름 가져올때
	FText GetBossName();
		
	
protected:
	
	UFUNCTION()
	UStatusComponent* GetStatusComponent() { return StatusSystem; }
	
	void AttachWeapon(TSubclassOf<AActor> weapon);
	
	UPROPERTY(editanywhere, Category=Combat, meta=(AllowPrivateAccess=true))
	TSubclassOf<class AActor> WeaponTemplate;
	
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void TestAttack();

	UFUNCTION()
	void SetTargetCharacter(AAnby* anby);

	AAnby* GetTargetCharacter();

	UFUNCTION(BlueprintCallable, BlueprintPure)
	UEnemyAnimInstance* GetEnemyAnimInstance();



	UFUNCTION(BlueprintCallable)
	UEnemyAnimInstance* SetAnimInstance(USkeletalMeshComponent* meshComp);


	// 피격 각 파생객체에서 정의하기
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	void GetDamaged(float damage);
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	void IsDead();

private:
	UPROPERTY()
	AAnby* TargetCharacter;

	UStatusComponent* StatusSystem;

	//UPROPERTY(BlueprintReadWrite, EditAnywhere)
	UEnemyAnimInstance* EnemyAnimInstance;



	UPROPERTY(ReplicatedUsing = OnRep_SetEnemy)
	FName Name;
	
	UFUNCTION()
	void OnRep_SetEnemy();
};
