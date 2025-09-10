// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AliceBase.h"
#include "GameFramework/Character.h"
#include "Alice.generated.h"

class AZZZEnemy;
class ADoro;
class AAliceController;
class ADamageNumberActor;
class UDamageNumberSubsystem;
class UAliceAnimInstance;
class UAlicePlayerWidget;
class UDialogueDataAsset;
class ARevenant;
class ABlockingVolume;

// 공격 상태(간단 버전)
UENUM(BlueprintType)
enum class EAttackState : uint8
{
	Idle,
	Attacking,
	Parring,
	Avoid,
	Death,
	Action,
};
UCLASS()
class WORKTEST_API AAlice : public AAliceBase
{
	GENERATED_BODY()

public:
	AAlice();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
public:
	UPROPERTY(EditAnywhere)
	TObjectPtr<ABlockingVolume> BlockingVolumeActor;

	void BlockingVolumeActive(bool _value);

	UPROPERTY(EditAnywhere)
	TSubclassOf<UDialogueDataAsset> DialogueDataAsset;
	
	//UDialogueDataAsset* DialogueData;

	UFUNCTION(BlueprintCallable, BlueprintPure)
	UAlicePlayerWidget* GetWidget(){return PlayerWidget;}
	// 화면에 데미지 보이게 하기
	//UPROPERTY(EditAnywhere, Category=DamageNumber)
	TSubclassOf<ADamageNumberActor> DamageNumberActorClass;
	// Snap 연출 매니저 (UObject 기반)
	UPROPERTY()
	class USnapCameraManager* SnapManager;
	
	// 기본 카메라 구성
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	class USpringArmComponent* SpringArm;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	class UCameraComponent* FollowCamera;

	
	UPROPERTY(EditAnywhere,BlueprintReadOnly, Category=Pivot)
	class USceneComponent* ShoulderPivot; // 어개 기준 고정을 위해서
	//class USoundComponent* SoundComponent = nullptr;
	FVector StartLoc;
	FVector TargetLoc;
	FVector NewLoc;
	
	void SetSpringArmLength(float _amount);
	
	// AnimNotify 함수
	void OpenComboWindow();
	void CloseComboWindow();
	void DoHit();
	void OnMontageEnded();
	void OnRecover();

	void ChargeEnd(); // Charge단계에서 진행함

	void Death();
	
	// 타겟 설정하기
	void SetTarget();
	void LookTarget();
	void HuntCountUp();
	// AttachWeapon
	TSubclassOf<AActor> WeaponTemplate;
	
	void AttachWeapon(TSubclassOf<AActor> _weapon);

	UFUNCTION(BlueprintCallable,BlueprintImplementableEvent)
	void SetMeshEnable(bool _value);

	void BossStageInit();
public:
	float ElapsedTime = 0.0f;
	void TryAttack();
	void ResetCombo();
	void DelayReset();
	
	void CreateAttackCollision(FVector _halfSize = FVector(50.f,50.f,25.f), float _end = 200.f);

	void LM_Press();
	
	void Rm_Press();
	void RM_Release();
	
	void Shift_Press();
	void Shift_Release();

	void R_Press();
	virtual void AvoidAction() override;
	virtual void CheckCurrentHP() override;
	virtual void CheckCurrentMP() override;
	UFUNCTION(BlueprintCallable,BlueprintImplementableEvent)
	void ParringSound();
	void TryParring();
	AActor* FindNearestEnemy();

	void BindEnemyWidget();
	bool IsMoving() { return bIsMoving;}

	
	// FindEnemy
	AActor* FindNearestEnemy_Registry(float MaxRadius);
	void SetBoss(ARevenant* _boss);
	
	//void StartShowDamage();
	//void StopShowDamage();
	bool IsActionState()const;

	FTimerHandle BulletTimeTimer;
	float SavedGlobalTimeDilation = 1.f;

	UFUNCTION() void StartBulletTime(float _worldFactor /*ex:0.2f*/,float _localFactor ,float RealDuration /*sec*/);
	UFUNCTION() void EndBulletTime();

	void SetAliceAttackState(const EAttackState& _state);

	//UPROPERTY(EditAnywhere)
	//TArray<ADoroSpawnPoint*> SpanwPointArray;
	//TArray<TObjectPtr<ADoroSpawnPoint>> DoroSpawnPointArray;

	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	TObjectPtr<AActor> Portal;
	UFUNCTION(BlueprintImplementableEvent)
	void PortalDestroy();
	
	//UFUNCTION(BlueprintImplementableEvent)
	void BossBattleSetting();
	
	UPROPERTY(EditAnywhere)
	TArray<TObjectPtr<ADoro>> DoroArray;
	void SpawnDoro();
	
private:
	
	FTimerHandle DoroSpawnTimer;
	void UpdateHuntUI();
	int32 HuntCount = 0;
	int32 MaxHuntCount = 3;
	bool CanTryAttack();
	bool bCameraActive = true;
	FVector CheckFront(float _length);
	void RandomAttackVocal();
	
	
	bool bIsChargeSuccess = false;
	//void ShowDamage();
	//FTimerHandle DamageShowTimer;
	FTimerHandle ResetHandle;
	FTimerHandle AvoidTimerHandle;
	//bool bIsAvoiding= false;
	bool bIsCharged = false; // 차지 모드 돌입
	bool bIsChargeIsFull = false; 
	bool bIsMoving =false;
	bool bIsRMClick = false;
	float RMCheckTime=  0.0f;
	
	bool bIsCanAttack =true;
	bool bIsParring = false;
	int CurrentStepIndex = 0;
	float AttackMoveAmount = 100;
	float AttackMoveTime =0.25f;
	FTimerHandle AttackAbleDelayTimer;
	EAttackState AttackState;
	bool bComboWindowOpen =false; // 콤보 입력 활성화
	bool bBufferedNext = false; // 다음 버퍼 진행여부
	int32 ComboCountIndex = 0;
	
	
	TSubclassOf<UAlicePlayerWidget> PlayerWidgetClass;
	//UAlicePlayerWidget* PlayerWidget;
	UAliceAnimInstance* AnimInstance;
	AZZZEnemy* CurrentTarget;
	ARevenant* Boss;
	//AAliceController* PC;
	//UDamageNumberSubsystem* DamageNumberSubsystem;

	FTimerHandle VideoDelayTimer;



	// 카메라 변경시키기요
public:
	//UFUNCTION(BlueprintCallable) void SetEnemyCameraTarget(AActor* InEnemy);
	UFUNCTION(BlueprintCallable) void ToggleCamera(bool _isMine,float _blendtime = 1.5f);   // 내 ↔ 적
private:
	//TWeakObjectPtr<AActor> EnemyTarget;
	bool bOnEnemyView = false;

	UPROPERTY(EditAnywhere, Category="Camera")
	float BlendTime = 1.5f;

	UPROPERTY(EditAnywhere, Category="Camera")
	TEnumAsByte<EViewTargetBlendFunction> BlendFunc = VTBlend_Cubic;

	UPROPERTY(EditAnywhere, Category="Camera")
	float BlendExp = 2.0f;

};
