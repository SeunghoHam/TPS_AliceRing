// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "DamageNumberSubsystem.generated.h"

/**
 * 
 */
class AAlice;
class ADamageNumberActor;
UCLASS()
class WORKTEST_API UDamageNumberSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
public:

	UDamageNumberSubsystem();
	// 풀링시스템 사용
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	ADamageNumberActor* Acquire(APlayerController* _pc);
	void Release(ADamageNumberActor* _actor);

	UPROPERTY(EditAnywhere)
	TSubclassOf<ADamageNumberActor> DamageActorClass;
private:

	
	//AAlice* alice;
	UPROPERTY() TArray<ADamageNumberActor*> Pool;
	TWeakObjectPtr<APlayerCameraManager> CachedCam; // 카메라 캐시를 왜 가져옴?
};
