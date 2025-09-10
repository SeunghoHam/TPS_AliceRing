// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ZZZEnemy.h"
#include "GameFramework/Character.h"
#include "Doro.generated.h"

class UDoroWidget;
class UWidgetComponent;
class AAlice;
UCLASS()
class WORKTEST_API ADoro : public AZZZEnemy
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ADoro();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	//virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaTime) override;
	virtual void Landed(const FHitResult& Hit) override;

	virtual void CheckCurrentHP() override;
public:
	UPROPERTY(EditAnywhere)
	TObjectPtr<UWidgetComponent> DoroWidget;

	UPROPERTY()
	 TObjectPtr<UDoroWidget> DoroWidgetInstance;
	
	UPROPERTY(EditAnywhere)
	TSubclassOf<UDoroWidget> DoroWidgetClass;
	
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	void DoroAngry(bool _isAngry);

	UFUNCTION(BlueprintCallable,BlueprintPure)
	bool IsAttacking(){return bIsAttack;}

	UFUNCTION(BlueprintCallable, BlueprintPure)
	bool IsLanded(){return bIsLanded;}

	UFUNCTION(BlueprintCallable, BlueprintPure)
	bool ActiveCheck();

	void SetDoroActive(bool _isActive);//{bIsActive = _isActive;}
	bool bIsActive= false;
private:
	//AAlice* aliceInstance;
	//UCapsuleComponent* Collision;
	void CreateAttackCollision(FVector _halfSize, float _end);

	
	float Ease01(float Alpha, float Exp) const;
	// 공격
	FVector StartLoc = FVector::ZeroVector;
	FVector TargetLoc = FVector::ZeroVector;
	bool bIsAttack = false;
	float AttackAlpha = 0.f;
	FVector CheckFront(float _length);
	void HopStart();
	FTimerHandle JumpTimerHandle;
	FTimerHandle DoroAttackTimerHandle;

	UFUNCTION(BlueprintCallable)
	void TryHop();

	UFUNCTION(BlueprintCallable)
	void AttackPrev();
	UFUNCTION(BlueprintCallable)
	void TryAttack();
	UFUNCTION(BlueprintCallable)
	void EndAttack();


	float InRateWithDistance =0.f;
	FVector halfSize = FVector(100.f, 50.f,50.f);
	float end = 300.f;
	bool bIsAngry = false;
	
	float HopZ = 600.f;
	float HopXY = 420.0f;
	float MinHopInterval = 0.15f;
	// 스쿼지 스트레이 스프링(감쇠 진동)
	float WobbleStiffness= 18.f; // 기존 24
	float WobbleDAmping = 8.f; // 기존 8
	float TAkeoffKick = 1.1f; // 기존 0.6 . 0.8~1.1추천
	float LandKick = 1.6f; // 기존 1.1 . 1.2~1.6 추천

	bool bIsLanded = true;
	
	float alpha = 0.0f;
	bool bIsRot = false;
	FQuat StartRot = FRotator::ZeroRotator.Quaternion();
	FQuat TargetRot = FRotator::ZeroRotator.Quaternion();
	FVector MoveDir = FVector::ZeroVector;
	float TimeSinceLastHop = 0.f;
	
	//스프링 상태
	float Wobble = 0.f;
	float WobbleVel = 0.f;

	void IntergrateWobble(float _deltaTime);
	void ApplySquashScale(float _wobble);

	void LookTarget();
	void SetMoveDir(bool _isOnTarget);
	void SetMeshSize(float _delta);
	FVector CurrentScale = FVector::OneVector;
};

