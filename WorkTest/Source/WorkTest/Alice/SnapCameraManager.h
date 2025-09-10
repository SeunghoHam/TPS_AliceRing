// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SnapCameraManager.generated.h"

class UCameraComponent;
class USpringArmComponent;
class AAliceController;
class AAlice;
struct FTimerHandle;

/**
 * 
*/

USTRUCT(BlueprintType)
struct FSoftLockParams
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float DeadZoneDeg    = 4.f;   // 중심 데드존
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float BreakAngleDeg  = 80.f;  // 과도한 이탈 각도
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float BreakTime      = 0.6f;  // 유지(가림/이탈 허용)
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float SpeedNear      = 4.f;   // 작은 오차 속도
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float SpeedFar       = 14.f;  // 큰 오차 속도
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float SwitchBoostSec = 0.2f;  // 전환 가속 지속
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float SwitchBoostMul = 1.4f;  // 전환 가속 배수
};

UENUM()
enum ESnapPhase
{
	None,
	Hold1,
	Hold2,
	Hold3,
	Out, // 목표로 스냅
	Return, // 원상태로 복귀,
	Parring,
	ChargeModeGo,
	Charging,
	ChargeAttack,
	EnemyEmote,
	EmoteReturn,
	Avoid,
	ShowRevenant
	//Delay
};

UENUM(BlueprintType)
enum class ESnapPreset : uint8
{
	// 평타 꾹누르기
	Hold_Part1, // 오른쪽 어깨
	Hold_Part2, // 적 바로 앞으로
	Hold_Part3, // 왼쪽 아래
	Return,
	Parring,
	ChargeModeGo,
	Charging,
	ChargeAttack,
	EnemyEmote,
	EmoteReturn,
	Avoid,
	ShowRevenant
	//Delay,
};

USTRUCT(BlueprintType) // 에디터에서 정의할 수 있게?
struct FSnapCameraData
{
	GENERATED_BODY()
	ESnapPhase Phase = ESnapPhase::None;
	// 이동/회전 목표
	//UPROPERTY(EditAnywhere, BlueprintReadWrite) FVector TargetLocation = FVector::ZeroVector;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bMove = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bLookAtTarget = true;

	// 카메라 Rotation
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FRotator CameraRotation = FRotator::ZeroRotator;
	
	// FOV
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float TargetFOV = 85.f;

	// 타깃지정
	UPROPERTY(EditAnywhere, BlueprintReadWrite) TWeakObjectPtr<AActor> TargetActor;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FVector AimOffset = FVector(0,0,80); // 소켓 없을 때 머리높이 보정
	// 직접 좌표로 쓸 경우(타깃 없으면 사용)
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FVector TargetLocation = FVector::ZeroVector;
	
	//SpringArm
	//UPROPERTY(EditAnywhere, BlueprintReadWrite) bool  = 85.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bChangeTargetArm = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float TargetArmLength = 250.f;

	FVector ArmSocketOffset = FVector::ZeroVector;
	FVector ArmTargetOffset = FVector::ZeroVector;
	
	// ShoulderPivot
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FVector TargetPivotLocation = FVector::ZeroVector;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FVector RightShoulderRelative = FVector(0,60.f, 40.f);
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FVector LeftUnderRelative = FVector(0,-100.f, -50.f);
	
	// 타이밍
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float OutDuration = 0.2f;

	// 이징(곡선 감)
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float EaseExp = 2.0f; // 2~3 추천

	// @@ 타겟 변경에서 사용 @@ 
	// --- 회전 보간 속도(소프트 락온 느낌) ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float LookInterpSpeed_Out = 20.f;   // 스냅 시 빠르게
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float LookInterpSpeed_Return = 12.f; // 복귀 시 부드럽게

	// --- 가림(occlusion) 체크(선택) ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool  bOcclusionCheck = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) TEnumAsByte<ECollisionChannel> OcclusionChannel = ECC_Visibility;
};
UCLASS()
class WORKTEST_API USnapCameraManager : public UObject
{
	GENERATED_BODY()


public:
	void Initialize(UCameraComponent* _camera, USpringArmComponent* _springArm,USceneComponent* _pivot );
	void StartSnap(const FSnapCameraData& Data,AActor* _target);

	void StartSnapPreset(ESnapPreset Preset, AActor* _target); //const FVector& TargetWorldLocation);

	void TickSnapBase(float _alpha);

	void TickSnap(float DeltaTime);

private:
	AAliceController* PC;
	UCameraComponent* SnapCamera;
	USpringArmComponent* SpringArm;
	USceneComponent* ShoulderPivot;
	

	AActor* CurrentTarget = nullptr;
	AAlice* Player = nullptr;
public:
	
	void SetEnemyEmoteShoulderPivotActor(AActor* _actor);
	AActor* ShoulderActor= nullptr;
	// 락온 API
	void EnableSoftLock(AActor* Target, const FVector& AimOffset = FVector(0,0,80));
	void DisableSoftLock();
	void SwitchSoftLock(AActor* NewTarget, const FVector& AimOffset = FVector(0,0,80));

	// 틱에서 호출
	void TickSoftLock(float DeltaSeconds);


	// 바라보는 점
	static FVector GetAimPoint(AActor* Target, const FVector& FallbackOffset);

	void UpdateLookOnRotation(float _deltaTime);
	
	void FixShoulderCamera(float _deltaTime);
	
	void SetLookTarget(AActor* _target, FVector _offset = FVector(0,0,80));


	FVector GetCameraLookToEnemyPoint() const;
	
private:
	
	// 적 바라보기
	bool bLockOn =false;
	// 내부 헬퍼
	FSnapCameraData MakePresetData(ESnapPreset Preset)const;//const FVector& TargetWorldLocation) const;
	float Ease01(float Alpha, float Exp) const; // 0~1 이징
	
	FSnapCameraData SnapData;


	// 가림보정 / 토글/설정
	FORCEINLINE void SetOcclusionCheck(bool bEnable) {SnapData.bOcclusionCheck = bEnable;}
	FORCEINLINE void SetOcclusionChannel(ECollisionChannel Ch) {SnapData.OcclusionChannel = Ch;}
	
	// Occlusion 처리
	FVector ComputeAimPointWithOcclusion(const FVector& PivotWS, const FVector& RawAimWS);
	FVector DesiredWS = FVector::ZeroVector;
	// 부드러운 에임 보간 상태
	FVector SmoothedAimWS = FVector::ZeroVector;
	bool    bHasSmoothedAim = false;
	
	// Occlusion세팅
	float   OcclusionForwardBias = 10.f;   // 가림 표면에서 카메라쪽으로 얼마나 띄울지(cm)
	float   AimLerpSpeedClear    = 12.f;   // 시야 트였을 때 타깃으로 되돌아가는 속도
	float   AimLerpSpeedBlocked  = 16.f;   // 가려졌을 때 표면 포인트로 붙는 속도. 커지면 가려질때 더 표면에 빠르게 붙음
	bool    bDebugOcclusion      = true;  // 디버그 라인/스피어
	
	// 원본 상태
	//FVector OriginCamLoc = FVector::ZeroVector;
	FRotator OriginCameraRotation = FRotator::ZeroRotator;
	FVector OriginSocketOffset =FVector::ZeroVector;
	FVector OriginTargetOffset = FVector::ZeroVector;

	FVector OriginPivotLoc;
	FVector TargetPivotLoc;
	
	float OriginalFOV= 85.0f;
	float OriginArmLength = 250.0f;
	float TargetArmLength = 0.f;
	
	FTimerHandle DelayTimer;
	
	// 진행 상태
	ESnapPhase Phase = ESnapPhase::None;
	ESnapPreset CurrentSnap = ESnapPreset::Hold_Part1;
	float ElapsedTime = 0.0f;
	float CurrentPhaseDuration = 0.f;
	bool bIsSnapping = false;


	 // 내부 헬퍼 Soft Lock aPI
    float AngleToSpeed(float Deg) const;

    // 상태
    bool   bSoftLockActive = false;
    float  GraceTimer = 0.f;
    float  SwitchBoostTimer = 0.f;

    FSoftLockParams SoftParams;
};
