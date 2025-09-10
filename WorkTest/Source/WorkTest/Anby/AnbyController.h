// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "AnbyController.generated.h"


class UInputMappingContext;
class UInputAction;
class AAnby;
struct FInputActionValue;


UENUM(BlueprintType)
enum EGameInputMode : uint8
{
	GameOnly UMETA(DisplayName = "Game Only"),
	UIOnly UMETA(DisplayName = "UI Only"),
	Mix UMETA(DisplayName = "Mix"),
};

UCLASS()
class WORKTEST_API AAnbyController : public APlayerController
{
	GENERATED_BODY()

public:
	AAnbyController();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputMappingContext* IMC_Player;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* LookAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* WheelAction;

		
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* WheelClickAction;


	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* JumpAction; // 일단 핑으로 쓰긴함
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* ParringAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* AttackAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* WeaponAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* GrappleMoveAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* GrappleShootAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* GrappleOffAction;


	
	// Look UP
	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category=Value)
	float upperMaxPitch = -50.0f; 
	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category=Value)
	float underMaxPitch = 10.0f;
	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category=Value)
	float amount = 3.0f; // pitch 에 따라서 감소하는 정도

	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category=Value)
	float minWheelAmount = -20;
	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category=Value)
	float maxWheelAmount = 20;

	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category=Value)
	float BaseLength = 300.0f;
	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category=Value)
	float MinLength = 150.0f;
	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category=Value)
	float MaxLength = 500.0f;
	
	UFUNCTION(BlueprintCallable, Category=InputMode)
	void SetGameInputMode(EGameInputMode _inputMode);

	UFUNCTION(BlueprintCallable, BlueprintPure,Category=InputMode)
	EGameInputMode GetInputMode() { return InputMode;}
protected:
	virtual void SetupInputComponent() override;

	// To add mapping context
	virtual void BeginPlay() override;
	//virtual void OnPossess(APawn* InPawn) override;
	virtual void OnPossess(APawn* InPawn) override;

private:
	void OnMove(const FInputActionValue& Value);
	
	void OnLook(const FInputActionValue& Value);
	float minValue;
	float maxValue;
	UFUNCTION(Server, Reliable)
	void OnLook_Server(const FInputActionValue& Value);

	void OnWheel(const FInputActionValue& Value);

	void OnWheelPressed();
	void OnWheelReleased();
	
	void ApplyLookRotation(FVector2D LookAxisVector);
	void OnParring();
	void OnEndParring();

	void OnAttack(); // TODO :  AttackType
	void OnAttackEnd();

	void OnRMaction();
	
	void OnSpaceTrigger();
	void OnSpaceRelease();

	void OnGrappleOff();
	
	void OnShiftPressed();
	void OnShiftReleased();
	
	void OnRmPressed();
	void OnRmReleased();
	
	void OnChangeWeaponActive();

	AAnby* Anby;
	
	EGameInputMode InputMode;

	//UPROPERTY(Replicated)	
	//FVector2D LookAxisVector;

	//UFUNCTION(Server, Reliable)
	//void OnServerLook(const FInputActionValue& Value);
	
	UPROPERTY(ReplicatedUsing = OnRep_Rotation)
	FRotator ReplicatedRotation;

	
	UFUNCTION()
	void OnRep_Rotation();
	
	
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
public:
	UPROPERTY(BlueprintReadWrite)
	float DirectionRight=0;
	UPROPERTY(BlueprintReadWrite)
	float DirectionFront=0;

	float WheelAmount;
	// 콤보공격 검사단계에서 입력queue의 가장 마지막이 공격이라면 공격진행, 아니라면 마지막 입력된거로 진행함
};
