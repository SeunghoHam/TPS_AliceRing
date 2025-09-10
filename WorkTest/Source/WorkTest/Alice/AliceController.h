// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "AliceController.generated.h"

/**
 * 
 */
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

class AAlice;
UCLASS()
class WORKTEST_API AAliceController : public APlayerController
{
	GENERATED_BODY()

	protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
public:
	AAliceController();
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputMappingContext* IMC_Player;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* LookAction;

	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* LMAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* RMAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* ShiftAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* EAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* RAction;
	
	// OnLoookMaxRange
	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category=Value)
	float upperMaxPitch = -50.0f; 
	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category=Value)
	float underMaxPitch = 10.0f;
	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category=Value)
	float amount = 3.0f; // pitch 에 따라서 감소하는 정도

	void SetOnLookActive(bool _active);
	
private:

	bool bIsOnLookActive =true;
	void OnMove(const FInputActionValue& Value);
	
	void OnLook(const FInputActionValue& Value);

	void OnAttack_Press();
	void OnAttack_Release();
	void OnRM_Press();
	void OnRM_Release();
	void OnShift_Press();
	void OnShift_Release();
	void OnE_Press();
	void OnR_Press();
	//void OnE_Release();
	
	
	AAlice* Alice;

	bool bShiftPressed = false;
};
