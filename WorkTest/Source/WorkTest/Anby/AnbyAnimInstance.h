// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "AnbyAnimInstance.generated.h"

/**
 * 
 */
UENUM(BlueprintType)
enum class EAnbyAnimState : uint8 // : uint = 8bit
{
	Locomotion = 0,
	Parring,
	Slash,
	EnableWeapon,
	DisableWeapon,
	Chain1,
	Chain2,
	Chain3,
	Chain4,
	Evade_Back,
	Evade_Left,
	Evade_Right,
};

class AAnby;
class AAnbyController;
class UskeletalMeshComponent;
UCLASS()
class WORKTEST_API UAnbyAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
private:
	virtual void NativeInitializeAnimation() override;

	virtual void NativeUpdateAnimation(float DeltaSeconds) override;
	

	void LookTarget();
	
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="LookAt", meta=(AllowPrivateAccess=true))
	FTransform SpineRotation;


	//FRotator LookAtRotator;
	//void TestPitchUP() ;
	//void TestRollUP();
	//void TestYawUP();
	
	UFUNCTION()
	void SetLookAtTarget(AActor* _target);

	AActor* LookAtTarget = nullptr;


	// Spine 본 회전에서 사용
	UPROPERTY(Blueprintreadwrite, EditAnywhere,Category=LookAt)
	FRotator LookAtRotation;

	
	UPROPERTY(Blueprintreadwrite, EditAnywhere,Category=LookAt)
	bool bUseLookAt= false;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category=LookAt)
	FName LookAtStartBone = "SpinePoint";
	
	///UPROPERTY(EditAnywhere,BlueprintReadWrite)
	//float Speed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	EAnbyAnimState AnimState; // In Editor ABP

	
	UFUNCTION(BlueprintCallable)
	void SetLocomotion(const EAnbyAnimState& state); /// OnStateAnimationEnds

	void ChangeAnbyAnimState(const EAnbyAnimState& state);


	void SetOwner(ACharacter* _owner);
private:
	//AAnbyController* Controller;
	ACharacter* Owner;

	USkeletalMeshComponent* MyMesh;
	
};
