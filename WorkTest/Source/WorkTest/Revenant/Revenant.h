// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AliceBase.h"
#include "GameFramework/Character.h"
#include "TelegraphActor.h"
#include "ZZZEnemy.h"
//#include "Revenant/TelegraphData.h"
#include "Revenant.generated.h"

class AAnbyGameState;
UENUM(BlueprintType)
enum class ERevenantState : uint8
{
	//GENERATED_BODY()
	AttackReady,
	Attack,
	Groggy,
	Idle,
	Move,
	Death,
	SecondPhase,
	CastCharging,
};
UENUM(BlueprintType)
enum class EAttackType : uint8
{
	Shoot,
	Cast,
	Throw,
};
class URevenantAnimInstance;
class ATelegraphActor;
class UTelegraphData;
UCLASS()
class WORKTEST_API ARevenant : public AZZZEnemy
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ARevenant();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	void FocusTarget(float _alpha);
	bool bIsfocusing = false;

	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	FRotator CurrentRot = FRotator::ZeroRotator;
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	FRotator TargetRot = FRotator::ZeroRotator;
	
public:
	// 기본 카메라 구성
	UPROPERTY(EditAnywhere,BlueprintReadOnly, Category=Pivot)
	class USceneComponent* ShoulderPivot; // 어개 기준 고정을 위해서
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	class USpringArmComponent* SpringArm;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	class UCameraComponent* FollowCamera;
	
	UFUNCTION(BlueprintImplementableEvent,BlueprintCallable)
	void TreeActive();
	UFUNCTION(BlueprintImplementableEvent,BlueprintCallable)
	void TreeDisActive();

	UFUNCTION(BlueprintCallable,BlueprintImplementableEvent)
	void EffectStart();

	UFUNCTION(BlueprintCallable,BlueprintImplementableEvent)
	void EffectEnd();
	
	//FText GetBossName();
	UFUNCTION(BlueprintCallable, BlueprintPure)
	bool GroggyCheck();
	
	void CreateAttackCollision(FVector _halfSize, float _end);
	UFUNCTION(BlueprintCallable)
	void Attack_Shoot();
	UFUNCTION(BlueprintCallable)
	void Attack_Ready();



	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	void BP_LevelStart();

	UFUNCTION(BlueprintCallable)
	void LevelStart_part1();
	UFUNCTION(BlueprintCallable)
	void LevelStart_part2();
	UFUNCTION(BlueprintCallable)
	void LevelStart_Part3();

	UFUNCTION(BlueprintCallable,BlueprintImplementableEvent)
	void CameraAction_SecondPhase();

	
	UFUNCTION(BlueprintCallable)
	void ShowEmote();

	UFUNCTION(BlueprintCallable)
	void ShowCastRange();

	UFUNCTION(BlueprintCallable)
	void TryJump();

	//UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	//void SetSecondPhase();


	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	void SetSecondPhaseMesh();

	void FirstMeetFlow();


	// 체력 50퍼이하 감소시 페이즈 전환
	void FirstPhaseFlow();	
	UFUNCTION(BlueprintCallable)
	void SecondPhaseFlow(); // 위치이동 & 다시 화면 활성화
	UFUNCTION(BlueprintCallable)
	void ThirdPhaseFlow(); 

	UFUNCTION(BlueprintCallable)
	void FullImageHide();

	
	//bool bIsFullImageGradation = false;
	
	
	// 캐릭터와의 거리로 패턴 고르기
	UFUNCTION(BlueprintCallable)
	bool CheckDistance(); 
	UFUNCTION(BlueprintCallable, BlueprintPure)
	bool CheckTreeActive() {return bIsTreeActive;}
	bool bIsTreeActive = true;

	void SetTreeActive();
	void CastExplosion();
	void Attack_Shoot2();

	void SetIdle();
	void SetGroggyPrev();
	void SetGroggy();
	void SetGroggyIdle();

	void SetJump();
	UFUNCTION(BlueprintCallable)
	void SetEmote();

	
	virtual void CheckCurrentHP() override;
	
	UFUNCTION(BlueprintCallable, BlueprintPure)
	bool GetSecondPhase();

	UFUNCTION(BlueprintCallable)
	void SetAnimState(ERevenantAnimState _animInstance);


	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	void BP_AttackWaitEnd(); // AttackWarnTime만큼 딜레이 후에 가도록
	
	UPROPERTY(BlueprintReadOnly);
	float AttackWarnTime = 0.3f;

	void BossStateInit();
private:
	float AttackHideTime = 0.2f;
	float AttackActiveTime= 0.3f;
	
	bool bIsSecondPhase = false;

	void StatusInit();
	// Revenant 상태
	bool bUsGroggy = false;
	URevenantAnimInstance* AnimInstance;

	// Telegraph
	// 표시 액터 클래스(디폴트는 C++ 액터)
	UPROPERTY(EditAnywhere, Category="Telegraph")
	TSubclassOf<ATelegraphActor> IndicatorClass;

	// 표시용 머티리얼(IndicatorClass가 BP라면 그쪽에 세팅해도 OK)
	UPROPERTY(EditAnywhere, Category="Telegraph")
	TObjectPtr<UMaterialInterface> TelegraphMIBase;

	// === API ===
	UFUNCTION(BlueprintCallable, Category="Telegraph")
	void BeginTelegraph(const FTelegraphParams& InParams);

	UFUNCTION(BlueprintCallable, Category="Telegraph")
	void ActivatePhase();

	UFUNCTION(BlueprintCallable, Category="Telegraph")
	void EndTelegraph();

	// RectAngle
	void ShowRectTelegraph(const FVector& CenterWS, const FVector& ForwardWS,
			//		   float Length, float Width,
					   float WarnTime = 0.6f, float ActiveTime = 0.25f, float FadeTime = 0.2f);

	void ShowCircleTelegraph(const FVector& centerWS, float _radius, float _halfAngleDeg,float _warnTime, float _activeTime, float _fadeOutTime);
private:
	void EnableCounter();
	AAnbyGameState* GS;
	ERevenantState RevenantState = ERevenantState::Idle;
	// 서버에서만 보관하는 타이머
	FTimerHandle PhaseTimerHandle;
	FTimerHandle FadeTimerHandle;

	
	FTimerHandle TurnHandle;
	FTimerHandle AttackSoundTimerHandle;
	float TurnRate;
	// 클라가 로컬 표시를 위해 생성하는 액터(복제 아님)
	UPROPERTY(Transient) TObjectPtr<ATelegraphActor> LocalIndicator = nullptr;
	
	UPROPERTY() FTelegraphParams Params;
	UPROPERTY() ETelegraphPhase Phase;

	void ChangeParams();
	void ChangePhase();
	void EnsureLocalIndicator();
	void DestroyLocalIndicator();

	// 페이드 아웃 로직(로컬)
	void StartLocalFadeOut(float Duration);
	float FadeAlpha = 0.0f;

	// Attack Type
	FVector HalfSize= FVector(50.0f, 100.0f, 25.0f);
	float End = 1200.0f;
	//float width  = 400.0f;
};
