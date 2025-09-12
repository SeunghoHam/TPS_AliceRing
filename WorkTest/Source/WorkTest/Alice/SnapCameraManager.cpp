// Fill out your copyright notice in the Description page of Project Settings.


#include "Alice/SnapCameraManager.h"

#include "Alice.h"
#include "AliceController.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

void USnapCameraManager::Initialize(UCameraComponent* _camera, USpringArmComponent* _springArm, USceneComponent* _pivot)
{
	SnapCamera = _camera;
	SpringArm = _springArm;
	ShoulderPivot = _pivot;

	PC = Cast<AAliceController>(UGameplayStatics::GetPlayerController(GetWorld(), 0));
	Player = Cast<AAlice>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
}


void USnapCameraManager::StartSnap(const FSnapCameraData& Data, AActor* _target)
{
	if (!SnapCamera) return;
	if (bIsSnapping) return;
	SnapData = Data;

	// 원본 기록
	//OriginCamLoc = SnapCamera->GetComponentLocation();
	OriginCameraRotation = SnapCamera->GetComponentRotation();
	OriginSocketOffset = SpringArm->SocketOffset;
	OriginTargetOffset = SpringArm->TargetOffset;
	OriginalFOV = SnapCamera->FieldOfView;
	SnapData.TargetActor = _target;

	OriginArmLength = SpringArm->TargetArmLength;
	TargetArmLength = SnapData.TargetArmLength;
	
	// 월드좌표 로컬좌표로 계산하기
	//const USceneComponent* Parent = ShoulderPivot->GetAttachParent();
	//const FTransform ParentXf = Parent ? Parent->GetComponentTransform() : FTransform::Identity;
	//TargetPivotLoc = ParentXf.InverseTransformPosition(SnapData.TargetPivotLocation); 

	if (SnapData.Phase == ESnapPhase::EnemyEmote || SnapData.Phase == ESnapPhase::ShowRevenant )
	{
		if (ShoulderActor)
		{
			TargetPivotLoc = ShoulderActor->GetActorLocation();
		}
		else
		{
			GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("ShoulderACtor = null"));
		}
	}
	/*
	else if ()
	{
		TargetPivotLoc = ShoulderActor->GetActorLocation();
	}*/
	else
	{
	TargetPivotLoc = SnapData.TargetPivotLocation;
	}


	// out 단계 시작
	Phase = SnapData.Phase;
	ElapsedTime = 0.f;
	CurrentPhaseDuration = SnapData.OutDuration; //FMath::Max(0.001f, SnapData.OutDuration);
	bIsSnapping = true;
}

void USnapCameraManager::StartSnapPreset(ESnapPreset Preset, AActor* _target)
{
	if (PC) PC->SetOnLookActive(false);
	StartSnap(MakePresetData(Preset), _target);
}


float USnapCameraManager::Ease01(float Alpha, float Exp) const
{
	Alpha = FMath::Clamp(Alpha, 0.f, 1.f);
	return FMath::InterpEaseInOut(0.f, 1.0f, Alpha, Exp);
}
FVector USnapCameraManager::ComputeAimPointWithOcclusion(const FVector& PivotWS, const FVector& RawAimWS)
{
	if (!SnapData.bOcclusionCheck) return RawAimWS;


	UWorld* World = GetWorld();
	if (!World) return RawAimWS;

	FHitResult Hit;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(CamOcclusion), false, nullptr);
	const bool bHit = World->LineTraceSingleByChannel(Hit, PivotWS, RawAimWS, SnapData.OcclusionChannel, Params);

	if (bDebugOcclusion)
	{
		DrawDebugLine(World, PivotWS, RawAimWS, bHit ? FColor::Red : FColor::Green, false, 0.f, 0, 0.5f);
		if (bHit) DrawDebugSphere(World, Hit.ImpactPoint, 8.f, 12, FColor::Yellow, false, 0.f);
	}

	// 가려졌고, 맞은 엑터가 우리가 노리고 있는 타깃이 아님. 표면 앞으로 살짝 이동시킥
	const bool bBlockedByOther =
		bHit
		&& Hit.GetActor()
		&& (!SnapData.TargetActor.IsValid()
			|| Hit.GetActor() != SnapData.TargetActor.Get());

	if (bBlockedByOther)
	{
		const FVector New = (PivotWS - Hit.ImpactPoint).GetSafeNormal();
		return Hit.ImpactPoint + New * OcclusionForwardBias;
	}

	// 가리지 않았음
	return RawAimWS;
}

float USnapCameraManager::AngleToSpeed(float Deg) const
{
	const float t = FMath::GetMappedRangeValueClamped(FVector2D(0.f, 90.f), FVector2D(0.0f, 1.0f), Deg);
	return FMath::Lerp(SoftParams.SpeedNear, SoftParams.SpeedFar, t);
}




//static const float ShoulderY = +60.f; // 오른쪽(+Y)
//static const float ShoulderZ = +40.f; // 약간 위
//static const float ArmLen    = 200.f; // 거리 350

void USnapCameraManager::SetEnemyEmoteShoulderPivotActor(AActor* _actor)
{
	ShoulderActor= _actor;
}

void USnapCameraManager::EnableSoftLock(AActor* Target, const FVector& AimOffset)
{
	CurrentTarget = Target;
	SnapData.TargetActor = Target; // 기존 파이프라인과 일치
	SnapData.AimOffset = AimOffset;
	bSoftLockActive = (Target != nullptr);
	bHasSmoothedAim = false; // 새 타깃에 맞춰 리셋
	GraceTimer = 0.f;
	SwitchBoostTimer = 0.f;
}

void USnapCameraManager::DisableSoftLock()
{
	bSoftLockActive = false;
	GraceTimer = 0.f;
	SwitchBoostTimer = 0.f;
}

void USnapCameraManager::SwitchSoftLock(AActor* NewTarget, const FVector& AimOffset)
{
	EnableSoftLock(NewTarget, AimOffset);
	SwitchBoostTimer = SoftParams.SwitchBoostSec; // 전환 가속
}

void USnapCameraManager::TickSoftLock(float DeltaSeconds)
{
	if (!bSoftLockActive || !CurrentTarget || !PC || !SpringArm) return;

	const FVector Pivot = SpringArm->GetComponentLocation();

	// 1) 원본 에임 포인트(머리/가슴 보정)
	const FVector RawAim = GetAimPoint(CurrentTarget, SnapData.AimOffset);
	// ← 이미 있음 :contentReference[oaicite:4]{index=4}

	// 2) 가림 보정
	const FVector AimOccl = ComputeAimPointWithOcclusion(Pivot, RawAim);
	// ← 이미 있음 :contentReference[oaicite:5]{index=5}
	const bool bClear = AimOccl.Equals(RawAim, 1e-2f);

	// 3) 에임 포인트 스무딩 (지터 억제)
	const float aimLerp = bClear ? AimLerpSpeedClear : AimLerpSpeedBlocked;
	// 네 멤버 그대로 사용 :contentReference[oaicite:6]{index=6}
	if (!bHasSmoothedAim)
	{
		SmoothedAimWS = AimOccl;
		bHasSmoothedAim = true;
	}
	else { SmoothedAimWS = FMath::VInterpTo(SmoothedAimWS, AimOccl, DeltaSeconds, aimLerp); }

	// 4) 목표 회전(롤 0)
	FRotator Desired = UKismetMathLibrary::FindLookAtRotation(Pivot, SmoothedAimWS);
	Desired.Roll = 0.f;

	
	// 5) 각도 오차 & 데드존
	const FRotator Cur = PC->GetControlRotation();
	const float ErrYaw = FMath::Abs(FRotator::NormalizeAxis(Desired.Yaw - Cur.Yaw));
	const float ErrPitch = FMath::Abs(FRotator::NormalizeAxis(Desired.Pitch - Cur.Pitch));
	const float ErrMax = FMath::Max(ErrYaw, ErrPitch);
	if (ErrMax < SoftParams.DeadZoneDeg) return;

	// 6) 그레이스 타임: 과각/가림 유지猶予
	const bool bTooFar = ErrMax > SoftParams.BreakAngleDeg;
	const bool bOccluded = !bClear;
	if (bTooFar || bOccluded) GraceTimer += DeltaSeconds;
	else GraceTimer = 0.f;
	if (GraceTimer > SoftParams.BreakTime)
	{
		DisableSoftLock();
		return;
	}

	// 7) 가변 속도 + 전환 가속
	float speed = AngleToSpeed(ErrMax);
	if (SwitchBoostTimer > 0.f)
	{
		speed *= SoftParams.SwitchBoostMul;
		SwitchBoostTimer -= DeltaSeconds;
	}

	// 8) 컨트롤러 회전 보간(한 경로)
	const FRotator NewRot = FMath::RInterpTo(Cur, Desired, DeltaSeconds, speed);
	PC->SetControlRotation(FRotator(NewRot.Pitch, NewRot.Yaw, 0.f));
	
}

FVector USnapCameraManager::GetAimPoint(AActor* Target, const FVector& FallbackOffset)
{
	if (!Target) return FVector::ZeroVector;
	return Target->GetActorLocation() + FallbackOffset; // 바라보는 시점 오프셋 조절해줘야함
	// 엑터 중앙점에서 얼굴쪽으로?
}

void USnapCameraManager::UpdateLookOnRotation(float _deltaTime)
{
	// Lockon확인하기
	if (!CurrentTarget) return;
	if (!PC) return;
	const FVector Pivot = SpringArm->GetComponentLocation();

	// 원본(이상적) 에임 포인트 : head/Chest 등을 Offset으로 위치조정
	const FVector RawAim = GetAimPoint(CurrentTarget, FVector::ZeroVector);

	// 가림 보정한 에임 포인트
	const FVector AimOccl = ComputeAimPointWithOcclusion(Pivot, RawAim);

	// 부드러운 에임 포인트로 수렴하기(시야 트림에 따라 속도 다르게 함
	const float LerpSpeed = (AimOccl.Equals(RawAim, 1e-2f) ? AimLerpSpeedClear : AimLerpSpeedBlocked);
	if (!bHasSmoothedAim)
	{
		SmoothedAimWS = AimOccl;
		bHasSmoothedAim = true;
	}
	else
	{
		SmoothedAimWS = FMath::VInterpTo(SmoothedAimWS, AimOccl, _deltaTime, LerpSpeed);
	}

	// 컨트롤러 회전 보간 -> springArm이 따라감 ( 롤 제거)
	FRotator Desired = UKismetMathLibrary::FindLookAtRotation(Pivot, SmoothedAimWS);
	Desired.Roll = 0.f; // 롤 고정 ( 좌우반전 방지)

	const FRotator Cur = PC->GetControlRotation();
	const float InterpSpeed = 10.f; // 스냅 10~14, 소프트 6~8
	const FRotator Newrot = FMath::RInterpTo(Cur, Desired, _deltaTime, InterpSpeed);

	PC->SetControlRotation(Newrot);
}

void USnapCameraManager::FixShoulderCamera(float _deltaTime)
{
	if (!PC) return;
	// 1.기준 회전 : Roll제거, Yaw/Pitch 만 사용하기
	FRotator ctrl = PC->GetControlRotation();
	ctrl.Roll = 0.f;

	// 2. 어꺠방향은 Yaw-only 기준의 right. 위는 WorldUp
	const FRotator YawOnly(0.f, ctrl.Yaw, 0.f);
	const FVector Fwd = YawOnly.Vector(); // 전방
	const FVector Right = FRotationMatrix(YawOnly).GetUnitAxis(EAxis::Y); // 항상 화면상의 오른쪽
	// yaw only 인 상태에서 right를 구해서. 옆 방향 부호가 바뀌지 않음. Roll=0으로 인해서 암의 좌/우 반전을 차단함
	// SpringArm의 Offset 상태에 의존하지 않고 절대좌표로 계산.

	const FVector Up = FVector::UpVector;

	// 3) 피벗 기준으로 "절대" 목표 위치 계산
	const FVector PivotWS = ShoulderPivot->GetComponentLocation();
	SpringArm->SetWorldLocation(PivotWS);
	SpringArm->SetWorldRotation(FRotator(ctrl.Pitch, ctrl.Yaw, 0.f));

	// 5) 오프셋은 0으로 유지하고, 카메라를 암 끝 위치로 스냅
	SpringArm->SocketOffset = FVector::ZeroVector; // 오프셋은 쓰지 않음(가변성 차단)
	SpringArm->TargetOffset = FVector::ZeroVector;

	// 암이 충돌 보정으로 당겨질 수 있으므로, 카메라를 직접 위치 맞출 필요는 없음
	// (원하면 부드러운 보간으로 위치 수렴) 
	SnapCamera->SetRelativeLocation(FMath::VInterpTo(SnapCamera->GetRelativeLocation(), FVector::ZeroVector, _deltaTime,
	                                                 20.f));
	SnapCamera->SetRelativeRotation(FMath::RInterpTo(SnapCamera->GetRelativeRotation(), FRotator::ZeroRotator,
	                                                 _deltaTime, 20.f));
}

/*
void USnapCameraManager::SetLookTarget(AActor* _target, FVector _offset)
{
	// offset기본값 ( 0,0,80)
	// 소켓은 없음
	CurrentTarget = _target;
	SnapData.TargetActor = _target;
	SnapData.AimOffset = _offset;
	GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Blue, FString::Printf(TEXT("Target : %s"), *CurrentTarget->GetName()));
}
*/
FVector USnapCameraManager::GetCameraLookToEnemyPoint() const
{
	if (CurrentTarget == nullptr) return FVector::ZeroVector;
	FVector dir = (Player->GetActorLocation() - CurrentTarget->GetActorLocation()).GetSafeNormal();
	FVector createpos = CurrentTarget->GetActorLocation() + dir * 150;
	return createpos;
}

void USnapCameraManager::TickSnap(float DeltaTime)
{
	if (!bIsSnapping || !SnapCamera) return;

	ElapsedTime += DeltaTime;
	float Alpha = FMath::Clamp(ElapsedTime / CurrentPhaseDuration, 0.f, 1.f);
	float Smooth = Ease01(Alpha, SnapData.EaseExp); // EaseEXp 2.0 ~ 2.4 권장

	if (Phase == ESnapPhase::Hold1)
	{
		TickSnapBase(Alpha);
		// 다음 단계 전환
		if (Alpha >= 1.f)
		{
			bIsSnapping = false;

			//Player->StopShowDamage();
			
			TWeakObjectPtr<USnapCameraManager> WeakThis(this);
			GetWorld()->GetTimerManager().SetTimer(DelayTimer, [WeakThis, this]()
			{
				if (!WeakThis.IsValid()) return;
				StartSnapPreset(ESnapPreset::Hold_Part2, CurrentTarget);
				Player->GetMesh()->SetVisibility(false);
				
			}, 0.5f,false);
			
		}
	}
	else if (Phase == ESnapPhase::Hold2)
	{
		TickSnapBase(Alpha);
		if (Alpha >= 1.f)
		{
			bIsSnapping = false;
			GetWorld()->GetTimerManager().ClearTimer(DelayTimer);
			TWeakObjectPtr<USnapCameraManager> WeakThis(this);
			GetWorld()->GetTimerManager().SetTimer(DelayTimer, [WeakThis, this]()
			{
				// 람다 표현식으로 함수 정의하기
				if (!WeakThis.IsValid()) return;
				StartSnapPreset(ESnapPreset::Hold_Part3, CurrentTarget);
				Player->GetMesh()->SetVisibility(true);
			}, 1.0f, false);
		}
	}
	else if (Phase == ESnapPhase::Hold3)
	{
		TickSnapBase(Alpha);
		if (Alpha >= 1.f)
		{
			bIsSnapping = false;
			GetWorld()->GetTimerManager().ClearTimer(DelayTimer);
			TWeakObjectPtr<USnapCameraManager> WeakThis(this);
			GetWorld()->GetTimerManager().SetTimer(DelayTimer, [WeakThis, this]()
			{
				// 람다 표현식으로 함수 정의하기
				if (!WeakThis.IsValid()) return;
				StartSnapPreset(ESnapPreset::Return, nullptr);
				//Player->GetMesh()->SetVisibility(true);
			}, 1.0f, false);
			//ElapsedTime = 0.f;
			//CurrentPhaseDuration = FMath::Max(0.001f, SnapData.ReturnDuration);
		}
	}
	else if (Phase ==  ESnapPhase::Parring)
	{
		TickSnapBase(Alpha);
		if (Alpha>= 1.0f)
		{
			bIsSnapping = false;
			
			GetWorld()->GetTimerManager().ClearTimer(DelayTimer);
			TWeakObjectPtr<USnapCameraManager> WeakThis(this);
			GetWorld()->GetTimerManager().SetTimer(DelayTimer, [WeakThis, this]()
			{
				
				// 람다 표현식으로 함수 정의하기
				if (!WeakThis.IsValid()) return;
				StartSnapPreset(ESnapPreset::Return, nullptr);
				//Player->GetMesh()->SetVisibility(true);
			}, 1.0f, false);
		}
	}
	else if (Phase==ESnapPhase::ChargeModeGo)
	{
		TickSnapBase(Alpha);
		if (Alpha >= 1.f)
		{
			bIsSnapping = false;
			StartSnapPreset(ESnapPreset::Return, nullptr);
		}
	}
	else if (Phase ==  ESnapPhase::Charging)
	{
		TickSnapBase(Alpha);
		if (Alpha>= 1.0f)
		{
			bIsSnapping = false;
			/*
			GetWorld()->GetTimerManager().ClearTimer(DelayTimer);
			TWeakObjectPtr<USnapCameraManager> WeakThis(this);
			GetWorld()->GetTimerManager().SetTimer(DelayTimer, [WeakThis, this]()
			{
				
				// 람다 표현식으로 함수 정의하기
				if (!WeakThis.IsValid()) return;
				StartSnapPreset(ESnapPreset::Return, nullptr);
				//Player->GetMesh()->SetVisibility(true);
			}, 1.0f, false);*/
		}
	}
	else if (Phase== ESnapPhase::ChargeAttack)
	{
		TickSnapBase(Alpha);
		if (Alpha>= 1.0f)
		{
			bIsSnapping = false;
			StartSnapPreset(ESnapPreset::Return, nullptr);
		}
	}
	else if (Phase == ESnapPhase::EnemyEmote)
	{
		TickSnapBase(Alpha);
		if (Alpha >= 1.0f)
		{
			bIsSnapping= false;
			
			GetWorld()->GetTimerManager().ClearTimer(DelayTimer);
			TWeakObjectPtr<USnapCameraManager> WeakThis(this);
			GetWorld()->GetTimerManager().SetTimer(DelayTimer, [WeakThis, this]()
			{
				// 람다 표현식으로 함수 정의하기
				if (!WeakThis.IsValid()) return;
				StartSnapPreset(ESnapPreset::EmoteReturn, nullptr);
				//Player->GetMesh()->SetVisibility(true);
			}, 4.0f, false);
		}
	}	else if (Phase == ESnapPhase::ShowRevenant)
	{
		TickSnapBase(Alpha);
		if (Alpha >= 1.0f)
		{
			bIsSnapping= false;
			/*
			GetWorld()->GetTimerManager().ClearTimer(DelayTimer);
			TWeakObjectPtr<USnapCameraManager> WeakThis(this);
			GetWorld()->GetTimerManager().SetTimer(DelayTimer, [WeakThis, this]()
			{
				// 람다 표현식으로 함수 정의하기
				if (!WeakThis.IsValid()) return;
				StartSnapPreset(ESnapPreset::Return, nullptr);
				//Player->GetMesh()->SetVisibility(true);
				
			}, 8.0f, false);*/
		}
	}
	else if (Phase == ESnapPhase::EmoteReturn)
	{
		TickSnapBase(Alpha);
		// 종료
		if (Alpha >= 1.f)
		{
			bIsSnapping = false;
			CurrentTarget = nullptr;
			if (PC) PC->SetOnLookActive(true);
		}
	}
	else if (Phase == ESnapPhase::Avoid)
	{
		TickSnapBase(Alpha);
		// 종료
		if (Alpha >= 1.f)
		{
			bIsSnapping = false;
			StartSnapPreset(ESnapPreset::Return,nullptr);
			//CurrentTarget = nullptr;
			//if (PC) PC->SetOnLookActive(true);
		}
	}
	else if (Phase == ESnapPhase::Return)
	{
		TickSnapBase(Alpha);
		// 종료
		if (Alpha >= 1.f)
		{
			bIsSnapping = false;
			CurrentTarget = nullptr;
			if (PC) PC->SetOnLookActive(true);
		}
	}
	
}

void USnapCameraManager::TickSnapBase(float _alpha)
{
	SpringArm->SocketOffset = FVector::ZeroVector;
	SpringArm->TargetOffset = FVector::ZeroVector;

	// ShoulderPivot
	const FVector pivotLoc = FMath::Lerp(OriginPivotLoc, TargetPivotLoc, _alpha);
	ShoulderPivot->SetRelativeLocation(pivotLoc);

	// TargetArmLength
	const float armLength = FMath::Lerp(OriginArmLength, TargetArmLength, _alpha);
	SpringArm->TargetArmLength = armLength;

	// FOV
	const float fov = FMath::Lerp(OriginalFOV, SnapData.TargetFOV, _alpha);
	SnapCamera->SetFieldOfView(fov);
}
FSnapCameraData USnapCameraManager::MakePresetData(ESnapPreset Preset) const
{
	FSnapCameraData D;

	switch (Preset)
	{
	case ESnapPreset::Hold_Part1:
		// 좌클릭 꾹 ( 
		D.Phase = ESnapPhase::Hold1;
		D.TargetPivotLocation = SnapData.RightShoulderRelative;
		D.TargetArmLength = 200.0f;
		D.bChangeTargetArm = true;
		D.bMove = true; // Offset이 변경되는지
		D.bLookAtTarget = false; // 적 바라봐야댐
		D.TargetFOV = 60; //FMath::Max(10.f, (SnapCamera ? SnapCamera->FieldOfView : 85.f) - 2.0f);
		D.OutDuration = 0.5f; // 현재위치 -> Part1까지의 걸리는 시간
		D.EaseExp = 2.0f;
		break;
	case ESnapPreset::Hold_Part2:
		D.Phase = ESnapPhase::Hold2;
		D.TargetPivotLocation = GetCameraLookToEnemyPoint();
		D.TargetArmLength = 120.0f; // 최소 80~120을 두고 보통 220~320에서 연출함
		D.bChangeTargetArm = true;
		D.bMove = true;
		D.TargetFOV = 80;
		D.OutDuration = 0.55f;
		D.EaseExp = 2.0f;
		break;
	case ESnapPreset::Hold_Part3:
		D.Phase = ESnapPhase::Hold3;
		D.TargetPivotLocation = SnapData.LeftUnderRelative;
		D.bChangeTargetArm = false;
		D.bMove = true; // Offset이 변경되는지
		D.bLookAtTarget = false; // 적 바라봐야댐
		D.TargetFOV = 90; //FMath::Max(10.f, (SnapCamera ? SnapCamera->FieldOfView : 85.f) - 2.0f);
		D.OutDuration = 1.0f; // 현재위치 -> Part1까지의 걸리는 시간
		D.EaseExp = 2.0f;
		break;
	case ESnapPreset::Parring:
		D.Phase = ESnapPhase::Parring;
		D.TargetPivotLocation = FVector(0.f, -120.f, 0.f); // z -50 
		D.TargetArmLength = 80.f;
		D.bChangeTargetArm = true;
		D.bMove = true; // Offset이 변경되는지
		D.bLookAtTarget = false; // 적 바라봐야댐
		D.TargetFOV = 70; //FMath::Max(10.f, (SnapCamera ? SnapCamera->FieldOfView : 85.f) - 2.0f);
		D.OutDuration = 0.6f; // 현재위치 -> Part1까지의 걸리는 시간
		D.EaseExp = 2.0f;
		break;
	case ESnapPreset::Return:
		D.Phase = ESnapPhase::Return;
		D.TargetPivotLocation = FVector(0.f,0.f, 50.f);
		D.TargetArmLength = 300.f;
		D.bMove = true;
		D.TargetFOV = 85.f;
		D.OutDuration = 0.3f;
		D.EaseExp = 2.0f;
		break;
	case ESnapPreset::ChargeModeGo:
		D.Phase = ESnapPhase::ChargeModeGo;
		D.TargetPivotLocation = FVector(50.0f, 0.0f, 50.0f); // z -50 
		D.TargetArmLength = 0.f;
		D.TargetFOV = 80.f; 
		D.OutDuration = 0.3f; 
		break;
	case ESnapPreset::Charging:
		D.Phase = ESnapPhase::Charging;
		D.TargetPivotLocation = FVector(0.f, 0.f, 50.f); // z -50 
		D.TargetArmLength = 250.f;
		D.TargetFOV = 70; 
		D.OutDuration = 0.4f; 
		break;
	case ESnapPreset::ChargeAttack:
		D.Phase = ESnapPhase::ChargeAttack;
		D.TargetPivotLocation = FVector(0.f, 0.f, 50.f); // z -50 
		D.TargetArmLength = 150.f;
		D.TargetFOV = 80.f; 
		D.OutDuration = 0.5f;
	case ESnapPreset::EnemyEmote:
		D.Phase = ESnapPhase::EnemyEmote;
		D.TargetPivotLocation = FVector::ZeroVector; // 따로 설정함
		D.TargetArmLength = 50.f;
		D.TargetFOV = 50.f; 
		D.OutDuration = 1.0f;
		break;
	case ESnapPreset::EmoteReturn:
		D.Phase = ESnapPhase::EmoteReturn;
		D.TargetPivotLocation = FVector(0.f,0.f, 50.f);
		D.TargetArmLength = 300.f;
		D.TargetFOV = 85.f; 
		D.OutDuration = 3.0f;
		break;
	case ESnapPreset::Avoid:
		D.Phase = ESnapPhase::Avoid;
		D.TargetPivotLocation = FVector(0.f,0.f, 50.f);
		D.TargetArmLength = 400.f;
		D.TargetFOV = 90.f;
		D.OutDuration = 0.3f;
		break;
	case ESnapPreset::ShowRevenant:
		D.Phase = ESnapPhase::ShowRevenant;
		D.TargetPivotLocation = FVector(-0.f,0.f, 0.f);
		D.TargetArmLength = 0.f;
		D.TargetFOV = 90.f;
		D.OutDuration = 2.0f;
		break;
	default:
		break;
	}
	return D;
}
