// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
//#include "AnbyWeapon.h"
//#include  "CombatComponent.h" // 그냥 추가해봐

#include "OnlineSessionSettings.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlineSessionDelegates.h" // FOnSession~ 에서 사용
#include "Interfaces/OnlineSessionInterface.h"
#include "Online/OnlineSessionNames.h" // SEARCH_PRESNENCE 매크로 사용. SessionSearch->QuerySetting
#include "Anby.generated.h"


class AAnbyWeapon;
class ABaseEnemy;
class AAnbyController;

// Component
class UCameraComponent;
class USpringArmComponent;
class UCombatComponent;
class UGrappleMoveComponent;
class UCamSettingComponent;
class UPingSpawnComponent;
class UMovementReplicator;

// Animation
class UAnbyAnimInstance;

// Widget
class UPlayerWidget;



UENUM(BlueprintType)
enum class EAnbyState : uint8 // : uint = 8bit
{
	Normal, // 동작중이지 않은 기본 이동 상태
	Working, // 캐릭터 동작중(공격, 패링) = 입력 밑 다른 이동 불가
	ChainAttacking,
	WeaponStateChange,
	Ping,
	//	Parring,
	// 타입마다 다른거 파악해야함
};

UENUM(BlueprintType)
enum class EMovementState : uint8
{
	Ground, // 기본, 그래플링 안함
	Hook, // 장치 꽂아둠
	Moving, // 이동중인경우
	Grappling, // 매달려있는 가능성 있음
	Falling, // 공중에서 연결해제, 중력 적게 적용시키기
};

UENUM(BlueprintType)
enum class EAttackInput : uint8 // : uint = 8bit
{
	Chain,
	Grapple
};

UENUM(BlueprintType)
enum class EViewType : uint8
{
	ThirdSight,
	FirstSight,
};

UCLASS(Config=Game)
//class WORKTEST_API AAnby : public ACharacter
class AAnby : public ACharacter
{
	GENERATED_BODY()

	// Sets default values for this character's properties
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* CameraComponent;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* SpringArmComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UCombatComponent* CombatSystem;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UGrappleMoveComponent* GrappleSystem;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UMovementReplicator* Replicator;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UCamSettingComponent* CameraSystem;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UPingSpawnComponent* PingSystem;
	
	//UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	//UStatusComponent* StatusSystem;
	
	FORCEINLINE  UCameraComponent* GetCamera() const { return CameraComponent; }
	FORCEINLINE  USpringArmComponent* GetSpringArm() const { return SpringArmComponent; }
	

	virtual void OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode = 0) override;
public:
	AAnby();

	UCameraComponent* GetCameraComponent() {return CameraComponent;}
	//UFUNCTION(Category= "Combat")
	 UCombatComponent* GetCombatComponent() { return CombatSystem; }
	UFUNCTION()
	UPingSpawnComponent* GetPingComponent(){return PingSystem;}
protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
public:
	// ----- CableComponent -----
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Cable,Replicated)
	class UCableComponent* GrappleCable;

	void InitGrapple();

	UFUNCTION(Server, Reliable, Category=Cable)
	void CableAttachToMovePoint(AActor* actor);

	UFUNCTION(Server,Reliable,Category=Cable)
	void ShowGrapple();
	
	UFUNCTION(Server,Reliable,Category=Cable)
	void HideGrapple();

	void GrappleCancelLaunch(const FVector& _direction, float _amount);


	// ----- Cam Component ------
	UFUNCTION(BlueprintCallable,BlueprintImplementableEvent, Category=Camera)
	void ChangeView();
	
	// ----- Ping ------
	UPROPERTY(Editanywhere, BlueprintReadWrite, Category = Ping)
	TSubclassOf<class APingActor> TemplatePingActor;
	
	UFUNCTION(Server,Reliable, BlueprintCallable, Category=Ping)
	void ServerSendPing(const FVector& Location);

	UFUNCTION(Server,Reliable)
	void ClientShowPing(const FVector& Location);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastSpawnPing(const FVector& Location);
	
	void PingPreviewStart();
	void PingPreviewEnd();
	
	// ----- Ping -----
	void SetPingPreviewActive(const bool& _value) { bIsPingPreview = _value;}
	
	bool bIsPingPreview =false;
	
	bool PingPreviewCheck() const { return bIsPingPreview; }
	// ----- Move -----
	UFUNCTION(Server, Reliable,BlueprintCallable)
	void ServerMoveCharacter(const FVector& TargetLocation);
	
	UFUNCTION(BlueprintCallable, Category = "AnimNotify")
	void ResetAnimState(); // AttackEnd. Go Locomotion

	UFUNCTION(BlueprintCallable,BlueprintPure, Category = "Weapon")
	AAnbyWeapon* GetAnbyWeapon();
	
	// Ping
	void HandlePingStart();
	void HandlePingEnd();
	bool bIsWheelClicked = false;
	// ----- Grappling ----
	void HandleGrappleStart();
	void HandleGrappleCancel();

	void HandleGrapplePreviewStart();
	void HandleGrappleShoot();

	void HandleGrappleOff();

	// Space bar
	void HandleSpaceStart();
	void HandleSpaceEnd();
	bool bIsSpacePressed= false;

	// Wheel
	void SetWheelAmount(float value);
	
	// ----- Look UP ---- SpirngArm 길이 조절
	void ControlSpringArmLength(float Deltatime);
	
	// ----- Parring ----- 
	UFUNCTION()
	void TryParring();
	
	UFUNCTION(BlueprintImplementableEvent, Category=Combat)
	void BP_Parring();
	void EndParring();

	// ----- Attack -----
	void TryAttack();
	
	void SetTarget();
	void ResetTarget();
	UFUNCTION(BlueprintImplementableEvent)
	void BP_ResetTarget();
	
	void RightClickAction();
	void CheckCombo(); // InputQueue 검사해서 진행할지 안할지 확인
	void AttackLaunchCharacter();
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Combat)
	float MoveAmount = 700.f;

	UFUNCTION(BlueprintCallable, Category=Combat)
	void LookAtEnemy();

	
	// ----- Weapon ----- 
	void ActiveWeapon();
	void DisActiveWeapon();

	UFUNCTION(BlueprintImplementableEvent, Category =Weapon)
	void BP_ActiveWeapon();
	UFUNCTION(BlueprintImplementableEvent, Category =Weapon)
	void BP_DisActiveWeapon();
	// ----- Anim, State -----

	UFUNCTION(BlueprintCallable, Category = "State")
	void ChangeAnbyState(const EAnbyState& state);


	bool IsWorking() { return AnbyState == EAnbyState::Working ? true : false; };
	bool IsAttacking() { return AnbyState  == EAnbyState::ChainAttacking ? true : false;	}
	bool IsWeaponChanging() { return AnbyState  == EAnbyState::WeaponStateChange ? true : false;	}


	EMovementState GetMovementState();
	
	bool GetWeaponState();
	
	void CallCurrentState();

	UFUNCTION(BlueprintCallable, BlueprintPure,Category= "Combat")
	int32 GetComboCount() { return ComboCount; }

	
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	ABaseEnemy* MyTarget;
	
	// In Editor Include Object
	UPROPERTY(editanywhere, Category=Combat, meta=(AllowPrivateAccess=true))
	TSubclassOf<class AAnbyWeapon> WeaponTemplate;

	UFUNCTION()
	void HandleAttackInput(const EAttackInput& inputType);

	UFUNCTION()
	void HandleLeftMouseEnd();
	
	// ----- Camera -----
	FVector GetCameraForwardVector();


	UFUNCTION(BlueprintCallable, BlueprintPure)
	UAnbyAnimInstance* GetAnbyAnimInstance();

	UFUNCTION(BlueprintCallable)
	void SetMovementState(const EMovementState& value);
	
	UPROPERTY(BlueprintReadWrite)
	EMovementState MovementState;

	// ----- View Type -----
	UFUNCTION(BlueprintCallable, Category=ViewType)
	void SetBaseTargetArmLength(float _baseLength) { BaseLength = _baseLength;}

	UFUNCTION()
	float GetBaseLength() const {return BaseLength;}


	// ViewType
	UFUNCTION(BlueprintCallable, Category=ViewType)
	void SetTargetMinArmLength();
	
	UFUNCTION(BlueprintCallable, Category=ViewType)
	void ChangeViewType(EViewType _type);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category=ViewType)
	EViewType GetViewType() const {return ViewType;}
	// cosnt 붙여서 값 복사형 반환
	
	UPROPERTY(EditAnywhere, Category=ViewType)
	EViewType ViewType;

	// UI
	UPROPERTY(EditAnywhere, Category="UI")
	TSubclassOf<UPlayerWidget> PlayerWidgetClass;
	
	UPROPERTY()
	UPlayerWidget* PlayerWidgetInstance;
	
private:
	AAnbyController* AnbyController;
	
	EAttackInput AttackInput;
	TQueue<EAttackInput> InputQueue;  // enum 또는 단순 bool도 가능
	FTimerHandle ProcessInputTimerHandle;
	void PerformAttack(); // 입력된 AttackInput 바로 재생
	void ProcessBufferedInput(); // 예약된 AttackInput 재생

	void ClearInputQueue();

	void InitControllerValue(); // 컨트롤러에서 수정하거나 적용한 값 가져오기
// Anim, State
	
	UPROPERTY()
	UAnbyAnimInstance* AnimInstance;
	UPROPERTY()
	EAnbyState AnbyState;
	UPROPERTY()
	AAnbyWeapon* AnbyWeapon;

	
	
	// AttachWeapon
	UFUNCTION()
	void AttachWeapon(TSubclassOf<AAnbyWeapon> Weapon); // const
	
	bool bIsWeaponActive = true;
	
	// Attack
	UPROPERTY()
	int32 ComboCount;
	bool bGoNextComboAttack;
	bool bCanComboAttackInput = true;
	
	// Button Press - Release Check
	bool bIsPressShift =false;
	bool bIsPressRM = false;

	// Status Value
	float upperMaxPitch; // -50
	float underMaxPitch; // 10
	float amount; // 3 //  pitch 에 따라서 감소하는 정도

	// ----- Wheel Value -----
	float WheelAmount =0; // 이건 컨트롤러에 없어도 됨
	
	float minWheelAmount; //= -20;
	float maxWheelAmount; //= 20;


	// ----- SrpingArm -----
	float BaseLength;// = 300.0f;
	float MinLength; // = 150.0f; 1인칭 시점으로 변환될때는 0으로 바꿔야함
	float MaxLength; // = 500.0f;
	
	float SaveAmount;
	
public:
};