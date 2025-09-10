// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CamSettingComponent.generated.h"

class AAnby;
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class WORKTEST_API UCamSettingComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UCamSettingComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	//virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;


	
	// 타임라인 시작 전 목표로 하는 값 설정해주기
	UFUNCTION(BlueprintCallable, Category=Camera)
	void SetPrevSetting();
	
	// 타임라인으로 변경
	UFUNCTION(BlueprintCallable, Category=Camera)
	void SetSpringArmLength(float _alpha);
	
	// 타임라인 종료 후 실행도리거
	UFUNCTION(BlueprintCallable, Category=Camera)
	void ChangeCharacterViewType();


	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Camera)
	float ThirdSightTargetArmLength = 300.0f;
	
private:
	AAnby* Anby;
	float CurrentArmLength;
	float TargetArmLength;
};
