// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CombatComponent.generated.h"


class ABaseEnemy;
class AAnby;
// 토큰으로 공격 한 건을 고유하게 처리
USTRUCT()
struct FInComingAttackToken
{
	GENERATED_BODY()
	
	UPROPERTY()
	TWeakObjectPtr<ABaseEnemy> Attacker; // 공격자(Enemy)
	//TWeakObjectPtr<UAnimMontage> AttackMontage; // 공격하는 모션. Montage
	// Notify가 열어놓은 패링 가능 프레임 종료시간
	//float ExpireTime = 0.f;
	
	FInComingAttackToken() {}
	FInComingAttackToken(ABaseEnemy* attacker, UAnimMontage* attackMontage){}
	//FInComingAttackToken(AActor* Attacker, float ExpireTime) : Attacker(Attacker), ExpireTime(ExpireTime) {}
};

//UCLASS( BlueprintType, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class WORKTEST_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()
	UCombatComponent();
protected:
	virtual void BeginPlay() override;
public:
	
	bool RegisterAttack(const FInComingAttackToken& token); // 적 공격이 들어온다
	bool CheckParringSuccess() ; // 패링 시도

	UFUNCTION(BlueprintCallable, Category=Combat)
	ABaseEnemy* FindFrontEnemy(float radius);
	
	UFUNCTION(BlueprintCallable,BlueprintPure,Category=Combat)
	FVector GetMoveDestination(const FVector& target);
	
	UFUNCTION(BlueprintCallable, Category=Combat)
	void GetHalfCirclePosition(const FVector& owner, const FVector& target, float t);
	
	UFUNCTION(BlueprintCallable, Category=Combat)
	void CameraViewChange(const FQuat& StartQuat, const FQuat& TargetQuat, float t);

	
	void SetCanParring(bool value) { bCanParring = value;}
	UPROPERTY(BlueprintReadWrite,EditAnywhere, Category=Combat)
	float BlankAmount= 50.f;

	UPROPERTY(BlueprintReadWrite,EditAnywhere, Category=Combat)
	float ViewAngleDegree = 50.0f;

	void SetCableVisiblity(bool _value);

	// ----- 공격 관련 ------

	UFUNCTION(Server,Reliable)
	void ServerAttack(ABaseEnemy* _enemy,float _damage);
	
	UFUNCTION(BlueprintCallable)
	void DoAttack(FVector _halfSize,float _end, float _damage);
	
	UFUNCTION()
	void CheckAttackCollisionRange(FVector _halfSize, float _end, float _damage);
	
private:

	// 공격관련
	FVector HalfSize;
	float Start; // 곱해지는 값. 시작점
	float End; // 공격범위에 곱해지는 값. 끝점

	FVector CollisionStart;
	FVector CollisionEnd;
	
	//ATestEnemy* Enemy;
	
	UPROPERTY()
	bool bCanParring =false;

	AAnby* anby;	
	// 현재 들어와 있는 공격 토큰 (복수 공격에 대해서 패링하려면 TArray로 변경함)
	UPROPERTY()
	TOptional<FInComingAttackToken> CurrentAttack;
};
