// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "WorkTestCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;
/*
USTRUCT()
struct FActorMove
{
	// 서버에 OnTick되어야할 요소들
	// 이동에서는 Throttle, Streeing
	// Deltatime을 OnTick(서버전송)에서 사용할거

	// 서버로 전송시킬 변수(Thrttle, StreeingThrow)를 struct 내부에서 선언함
	// 개별 복제가 필요없으니 UPROPERTY 선언에서 Replicated를 뺌

	UPROPERTY()
	float Throttle;

	UPROPERTY()
	float DeltaTime;
};

USTRUCT()
struct FActorState
{
	// 여기서 상태를 확인할거
	
	UPROPERTY()
	FVector Velocity;

	UPROPERTY()
	FVector Transform;

	UPROPERTY()
	FActorMove LastMoves;
	// 서버에서 마지막 이동을 수신할때
	// 승인되지 않은 이동 목록을 확인해서 검사함
	// 오래된 이동을 제거하고 새로운 이동으로 교체하여 유지

};
*/
DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

UCLASS(config=Game)
class AWorkTestCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;
	
	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* QskillAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* EskillAction;


	void Tick(float Deltatime) override;
public:
	AWorkTestCharacter();
	
	void QSkill();
	void ESkill();
	
protected:

	void TestJump();

	UFUNCTION(Server, Reliable) // WithValidation 추가하면 _Validation 함수도 추가로 만들어야함 bool 타입
	void Server_Jump();

	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);
	
private:
	// Replicated
	// 1. Uproperty(Replicated)로 변수 선언
	// 2. GetLifetimeReplicatedProps()함수 완성
	// 3. 복제하기

	// 
	//UPROPERTY(Replicated) // Replicated <-> ReplicatedUsing
	UPROPERTY(ReplicatedUsing = OnRep_ReplicatedTransform) // 인수, 반환 있으면 안됨. 그저 트리거임)
	FVector ReplicatedTransform;

	UPROPERTY(Replicated)
	FVector CustomVelocity;
	

	/*
	UPROPERTY(Replicated)
	FActorState ServerState;
	*/
	


	UFUNCTION()
	void OnRep_ReplicatedTransform(); // 인수, 반환 있으면 안됨. 그저 트리거임

	// Override Replicate Properties function
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	// To add mapping context
	virtual void BeginPlay();

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
};

