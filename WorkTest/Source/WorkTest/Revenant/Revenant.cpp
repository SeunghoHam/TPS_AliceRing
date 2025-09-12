// Fill out your copyright notice in the Description page of Project Settings.


#include "Revenant/Revenant.h"

#include "Revenant/TelegraphActor.h"
#include "Revenant/RevenantAnimInstance.h"
#include "Kismet/GameplayStatics.h"
#include "System/EnemyRegistrySubsystem.h"
#include "Alice.h"
#include "AliceController.h"
#include "AlicePlayerWidget.h"
#include "SnapCameraManager.h"
#include "AnbyGameState.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"

ARevenant::ARevenant()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	ShoulderPivot= CreateDefaultSubobject<USceneComponent>(TEXT("ShoulderPivot"));
	ShoulderPivot->SetupAttachment(RootComponent);

	//ShoulderPivot->SetRelativeLocation(FVector(0.0f, 60.0f, 40.0f));
	ShoulderPivot->SetRelativeLocation(FVector(0.0f, 0.0f, 50.0f));
	
	// Spring Arm 생성
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(ShoulderPivot);
	SpringArm->TargetArmLength = 400.f;                // 카메라 거리 (약간 짧은 편)
	//SpringArm->SocketOffset = FVector(0.f, 60.f, 40.f); // 오른쪽 위에서 내려다보는 시점
	SpringArm->SocketOffset = FVector(0.f, 0.f, 0.f);
	SpringArm->TargetOffset = FVector(0.f, 0.f, 0.f);
	SpringArm->bEnableCameraLag = true;
	SpringArm->CameraLagSpeed = 10.f;
	SpringArm->CameraRotationLagSpeed = 10.f;
	
	SpringArm->bInheritYaw = true; // 좌우 움직임
	
	SpringArm->bInheritRoll = false; // 반전 방지 핵심 = false

	// Pivot으로 설정 변경되기
	SpringArm->bInheritPitch = true; // 상하 움직임 ( ShoulderPivot때문에 false로 해봄) 원래 True
	SpringArm->bUsePawnControlRotation = true;          // 입력에 따라 회전  원래 false

	
	
	// Follow Camera 생성
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(SpringArm);
	FollowCamera->FieldOfView = 85.f;

	// Pivot 설정변경
	FollowCamera->bUsePawnControlRotation = false; // 카메라는 직접 회전 제어안함 (원래 fasle

	SpringArm->SocketOffset = FVector::ZeroVector;
	SpringArm->TargetOffset = FVector(0,0,10.f);
}


// Called when the game starts or when spawned
void ARevenant::BeginPlay()
{
	Super::BeginPlay();

	AnimInstance = Cast<URevenantAnimInstance>(GetMesh()->GetAnimInstance());
	RevenantState = ERevenantState::Idle;
	SetOutLineEnable(false);
	if (!IndicatorClass)
	{
		GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Red,
		                                 TEXT("[Revenant] : IndicatorClass = null"));
	}
	if (!TelegraphMIBase)
	{
		GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Red,
		                                 TEXT("[Revenant] : TelegraphMIBase = null"));
	}
	/*
	if (Target)
	{
		GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Yellow,
		                                 FString::Printf(TEXT("Target : %s"), *Target->GetName()));
	}
*/
	bIsTreeActive= false;	

	FTimerHandle timer;
	GetWorld()->GetTimerManager().SetTimer(timer, this, &ARevenant::StatusInit,0.5f,false);
	
	//bIsTreeActive = false;

	//TreeActive();
	//BP_LevelStart(); //패턴 시작할때는 이거로
	//FirstPhaseFlow();
}
void ARevenant::StatusInit()
{
	BossStatusInitialize(100, 20);
	
	Target->SetBoss(this);
	
	FText bossName = FText::FromString("BossName :: Empty"); 
	Target->GetWidget()->OnSetBossName(bossName);
}
// Called every frame
void ARevenant::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if (bIsfocusing)
	{
		FocusTarget(DeltaTime);
	}
}

void ARevenant::FocusTarget(float _alpha)
{
	//Super::FocusTarget(_alpha);
	//GEngine->AddOnScreenDebugMessage(-1, 0.2f, FColor::Yellow,TEXT("회전쓰"));

	//FQuat rot = FMath::Lerp(CurrentQuat, TargetQuat, _alpha);
	//SetActorRotation(rot);

	//const float YawOnly = newRot.Yaw;
	//const FRotator newRot = (Target->GetActorLocation() - GetActorLocation()).GetSafeNormal().Rotation();
	FRotator newRot = FMath::RInterpTo(CurrentRot, TargetRot, _alpha, 400.0f);
	float YawOnly = newRot.Yaw;
	SetActorRotation(FRotator(0.f, YawOnly, 0.f), ETeleportType::None);
}


bool ARevenant::GroggyCheck()
{
	bool bisGroggy = RevenantState == ERevenantState::Groggy ? true : false;
	return bisGroggy;
}

void ARevenant::CreateAttackCollision(FVector _halfSize, float _end)
{
	//FVector Forward = GetActorForwardVector();
	const FVector F = GetActorForwardVector().GetSafeNormal();
	const FVector CollisionStart = GetActorLocation() + F * 50.f;
	const FVector CollisionEnd = GetActorLocation() + F * End;

	//DrawDebugSphere(GetWorld(), CollisionStart, 32.0f, 16, FColor::Blue, false, 1.0f);
	//DrawDebugSphere(GetWorld(), CollisionEnd, 32.0f, 16, FColor::Green, false, 1.0f);

	TArray<FHitResult> OutHits;
	FRotator Orient = GetActorRotation();
	FQuat Rot = Orient.Quaternion();

	FCollisionShape BoxShape = FCollisionShape::MakeBox(HalfSize);
	// FCollisionQueryParams& Params;
	bool bHit = GetWorld()->SweepMultiByChannel(
		OutHits,
		CollisionStart,
		CollisionEnd,
		Rot,
		ECC_Visibility,
		BoxShape);

	const float Travel = FMath::Max(End - 50.f, 0.f);
	const float TotalLen = Travel + 2.f * HalfSize.X; // = Length
	const float HalfTotalLen = TotalLen * 0.5f; // = HalfX + Travel/2
	const FVector CoverCenter = CollisionStart + F * HalfTotalLen;
	const FVector CoverExtent = FVector(HalfTotalLen, HalfSize.Y, HalfSize.Z);

	//FVector Center = (CollisionStart + CollisionEnd) * 0.5f;
	//FVector BoxExtend = _halfSize;

	//DrawDebugBox(GetWorld(), CoverCenter, CoverExtent, Rot, FColor::Blue, false, 0.5f);
	if (bHit)
	{
		for (const FHitResult& HitActor : OutHits)
		{
			if (HitActor.GetActor()->ActorHasTag("Player"))
			{
				AAlice* alice = Cast<AAlice>(HitActor.GetActor());
				alice->GetDamaged(40.0f);
				FVector dir =(Target->GetActorLocation() - GetActorLocation()).GetSafeNormal() * 1000.0f;
				alice->LaunchCharacter(dir,true,false);
			}
		}
	}
}
void ARevenant::Attack_Shoot2()
{
	//AnimInstance->SetAnimState(ERevenantAnimState::Attack_Slow);
	PlaySound(ESoundName::effect_Groggy, 1.0f); // Shoot Sound로  변경
	bIsfocusing = false;
}
void ARevenant::Attack_Shoot()
{
	if (RevenantState != ERevenantState::AttackReady) return;
	if (GroggyCheck()) return;

	RevenantState = ERevenantState::Attack;
	AnimInstance->SetAnimState(ERevenantAnimState::Attack_Slow);
	CreateAttackCollision(HalfSize, End);
	EffectStart();
	
	// 1. WeaponRready 자세 후
	// 1-2 Shoot 0.1 초 전 소리 먼전
	// 3. 일정 시간 뒤 Shoot
}

void ARevenant::Attack_Ready()
{
	if (RevenantState != ERevenantState::Idle) return;
	//TurnRate = 0.0f;
	bIsfocusing = true;
	CurrentRot = GetActorRotation();
	TargetRot = (Target->GetActorLocation() - GetActorLocation()).GetSafeNormal().Rotation();
	// float warnTime = 0.2f; // BehaviorTree의 공격딜레이 시간보다 0.1초 빠르게
	TWeakObjectPtr<ARevenant> WeakThis(this);
	GetWorld()->GetTimerManager().ClearTimer(TurnHandle);
	GetWorld()->GetTimerManager().SetTimer(TurnHandle, [WeakThis, this]()
	{
		if (!WeakThis.IsValid()) return;
		if (!GroggyCheck())
		{
			bIsfocusing = false;
			RevenantState = ERevenantState::AttackReady;
			AnimInstance->SetAnimState(ERevenantAnimState::Idle_Weapon);
			FVector SpawnLoc = GetActorLocation();
			SpawnLoc.Z -= 88.f;
			ShowRectTelegraph(SpawnLoc, GetActorForwardVector(),AttackWarnTime,AttackActiveTime,AttackHideTime);
			BP_AttackWaitEnd();
		}
	}, AttackWarnTime, false);
	
}

void ARevenant::LevelStart_Part3()
{
	Target->GetWidget()->ClearDialogue();

	Target->GetWidget()->ToggleView();
	Target->ToggleCamera(true); // 등장 마무리(내 시점)
}

void ARevenant::LevelStart_part2()
{
	//AAlice* alice = Cast<AAlice>(Target);
	Target->GetWidget()->ClearDialogue();
	Target->GetWidget()->ShowByKey("2");
}

void ARevenant::LevelStart_part1()
{
	AnimInstance->SetAnimState(ERevenantAnimState::LevelStart);


	FVector EnemyToCharDir = (Target->GetActorLocation() - GetActorLocation()).GetSafeNormal();
	FVector CharToEnemyDir = -EnemyToCharDir;

	FRotator EnemyRot = EnemyToCharDir.Rotation();
	FRotator CharRot = CharToEnemyDir.Rotation();

	SetActorRotation(EnemyRot);

	Target->GetWidget()->ToggleView();
	Target->ToggleCamera(false); // 등장 (적 초점)
	Target->SetActorRotation(CharRot);


	Target->GetWidget()->ShowByKey("1");

	// 일단 서로 바라보기
	//AAlice* alice = Cast<AAlice>(Target);
	//alice->SnapManager->SetEnemyEmoteShoulderPivotActor(this);
	//alice->SnapManager->EnableSoftLock(this,FVector::ZeroVector);
	//alice->SnapManager->StartSnapPreset(ESnapPreset::ShowRevenant,this);


	//BP_LevelStart();
}

void ARevenant::ShowEmote()
{
	SetEmote();
	bIsTreeActive = false;
	FVector EnemyToCharDir = (Target->GetActorLocation() - GetActorLocation()).GetSafeNormal();
	FVector CharToEnemyDir = -EnemyToCharDir;

	FRotator EnemyRot = EnemyToCharDir.Rotation();
	FRotator CharRot = CharToEnemyDir.Rotation();

	// 일단 서로 바라보기
	SetActorRotation(EnemyRot);
	Target->SetActorRotation(CharRot);
	Target->ToggleCamera(false); // 적 시점으로

	/*
	AAlice* alice = Cast<AAlice>(Target);
	alice->SnapManager->SetEnemyEmoteShoulderPivotActor(this);
	alice->SnapManager->EnableSoftLock(this, FVector(50.f, -50, 0.f));
	alice->SnapManager->StartSnapPreset(ESnapPreset::EnemyEmote, this);
	*/
}

void ARevenant::ShowCastRange()
{
	if (RevenantState != ERevenantState::Idle) return;
	RevenantState = ERevenantState::CastCharging;
	AnimInstance->SetAnimState(ERevenantAnimState::CastChargingStart);
	float amount = 500.0f;
	FVector Point1 = GetActorRightVector() * amount;
	FVector Point2 = GetActorForwardVector() * amount;
	FVector Point3 = -GetActorForwardVector() * amount;
	FVector Point4 = -GetActorRightVector() * amount;

	FVector SpawnLoc = GetActorLocation();
	SpawnLoc.Z -= 88.f;
	float radius = 800.f;
	float halfAngleDeg = 60.f;
	float warnTime = 1.0f;
	float activeTime = 0.4f;
	float fadeOutTime = 0.3f;

	ShowCircleTelegraph(SpawnLoc, radius, halfAngleDeg, warnTime, activeTime, fadeOutTime);
	GetWorld()->GetTimerManager().ClearTimer(AttackSoundTimerHandle);
	GetWorld()->GetTimerManager().SetTimer(AttackSoundTimerHandle, this,
	                                       &ARevenant::CastExplosion, warnTime + activeTime, false);


	FTimerHandle RangeShow;
	GetWorld()->GetTimerManager().SetTimer(RangeShow, this,
	                                       &ARevenant::EnableCounter, warnTime - 0.2f, false);
}

void ARevenant::TryJump()
{
	Jump();
	AnimInstance->SetAnimState(ERevenantAnimState::Jump);
}



bool ARevenant::CheckDistance()
{
	if (!Target) return false;
	float Dist = FVector::Dist(Target->GetActorLocation(), GetActorLocation());
	//GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, FString::Printf(TEXT("[Revenant] Dist : %f"), Dist));
	if (Dist >= 300)
	{
		return false;
	}
	else
	{
		return true;
	}
}

void ARevenant::CastExplosion()
{
	if (RevenantState != ERevenantState::CastCharging) return;

	//GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green,TEXT("[Revenant] CastExplosion"));
	AnimInstance->SetAnimState(ERevenantAnimState::Cast);

	float Radius = 1000.0f;
	FCollisionShape SphereShape = FCollisionShape::MakeSphere(Radius);

	 TArray<FHitResult> Hits;
	// FCollisionQueryParams& Params;
	bool bHit = GetWorld()->SweepMultiByChannel(
		Hits,
		GetActorLocation(),
		GetActorForwardVector() * (Radius * 0.5f),
		FQuat::Identity,
		ECC_Visibility,
		SphereShape);

	//DrawDebugSphere(GetWorld(), GetActorLocation(), Radius,16, FColor::Green,false ,2.0f);
	if (bHit)
	{
		for	(FHitResult Hit : Hits)
		{
			if (Hit.GetActor()->ActorHasTag("Player"))
			{
				AAlice* alice = Cast<AAlice>(Hit.GetActor());
				alice->GetDamaged(50.0f);
				FVector dir =(Target->GetActorLocation() - GetActorLocation()).GetSafeNormal() * 2000.0f;
				alice->LaunchCharacter(dir,true,false);
			}
		}
	
	}
}




void ARevenant::SetIdle()
{
	if (RevenantState == ERevenantState::Attack)
	{
		EffectEnd();
	}
	AnimInstance->SetAnimState(ERevenantAnimState::Idle_NoWeapon);
	RevenantState = ERevenantState::Idle;
}

void ARevenant::SetGroggyPrev()
{
	bIsTreeActive = false;
	RevenantState = ERevenantState::Groggy;
}

void ARevenant::SetGroggy()
{
	//AnimInstance->StopAllMontages(0.15f);
	bIsTreeActive = false;
	AnimInstance->SetAnimState(ERevenantAnimState::StunStart);
	RevenantState = ERevenantState::Groggy;
	PlaySound(ESoundName::effect_Groggy, 2.0f);
}

void ARevenant::SetGroggyIdle()
{
	AnimInstance->SetAnimState(ERevenantAnimState::Idle_Stun);
	//bIsTreeActive = true;
}

void ARevenant::SetJump()
{
	AnimInstance->SetAnimState(ERevenantAnimState::Jump);
}

void ARevenant::SetEmote()
{
	AnimInstance->SetAnimState(ERevenantAnimState::Emote);
}


void ARevenant::CheckCurrentHP()
{
	//Super::CheckCurrentHP();

	//CurrentHP -= Damage;
	//AddHP(-Damage);
	AddBossHP(-Damage);
	AddBossMP(-1);
	//GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Magenta, FString::Printf(TEXT("HP : %f"), CurrentHP));

	if (CurrentMP<= 5.0f)
	{
		SetGroggy();
	}
	if (CurrentHP <= 0.0f)
	{
		RevenantState = ERevenantState::Death;
		bIsTreeActive = false;
		//AnimInstance->SetAnimState(Ereve)
	}
	else if (CurrentHP <= 50.f && !bIsSecondPhase)
	{
		//GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green,TEXT("SecondPhase돌입"));
		
		// 입력 및 BT 막기단계
		FirstPhaseFlow();
	}
}

void ARevenant::FirstMeetFlow()
{
	
}



void ARevenant::FirstPhaseFlow()
{
	bIsSecondPhase = true;
	bIsTreeActive = false;
	RevenantState = ERevenantState::SecondPhase;
	AliceController->SetOnLookActive(false);
	Target->SetAliceAttackState(EAttackState::Action);
	
	// 화면 막기
	Target->GetWidget()->FullImageActive(true); // alpha 0 -> 1
	Target->GetWidget()->ToggleView();
	Target->GetWidget()->bIsFullImageGradation = true; // false는 BTTask 에서 호출하는 함수랑 같이
	//bIsFullImageGradation = true;
	//TWeakObjectPtr<ARevenant> WeakThis;

	FTimerHandle timerHandle;
	GetWorld()->GetTimerManager().SetTimer(timerHandle, this, &ARevenant::SecondPhaseFlow, 0.5f, false);
	//GetWorld()->GetTimerManager().SetTimer(timerHandle, this, &ARevenant::ThidPhaseFlow, 3.f, false);
}

void ARevenant::SecondPhaseFlow()
{
	//GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green,TEXT("SecondPhase 2"));

	Target->ToggleCamera(false, 0.f);
	
	// 위치조정하기
	MoveEmotePointActor();
	Target->MoveEmotePointActor();
	
	// SetSecondPhaseMesh(); BP 로 역할넘겨
	CameraAction_SecondPhase(); // 블루프린트에서 동작
	
	//Target->SetMeshEnable(false);
	
	//FTimerHandle timerHandle;
	//GetWorld()->GetTimerManager().SetTimer(timerHandle, this, &ARevenant::ThidPhaseFlow, 5.f, false);
}

void ARevenant::ThirdPhaseFlow()
{
	GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green,TEXT("SecondPhase End"));
	Target->ToggleCamera(true);
	// 배경음악 가동?
	Target->GetWidget()->ToggleView();
	AliceController->SetOnLookActive(true);
	
	GetWorld()->GetTimerManager().ClearTimer(PhaseTimerHandle);
	//FTimerHandle PhaseHandle;
	GetWorld()->GetTimerManager().SetTimer(PhaseTimerHandle, this, &ARevenant::SetTreeActive, 0.8f, false);
	//bIsTreeActive = true;
	// 도착 완료되면 BT 및 입력가능상태로 변경시키기
}

void ARevenant::SetTreeActive()
{
	bIsTreeActive = true;
}

void ARevenant::FullImageHide()
{
	Target->GetWidget()->FullImageActive(false);
	Target->GetWidget()->bIsFullImageGradation = false; 
}

bool ARevenant::GetSecondPhase()
{
	return bIsSecondPhase;
}

void ARevenant::SetAnimState(ERevenantAnimState _animInstance)
{
	AnimInstance->SetAnimState(_animInstance);
}

void ARevenant::BossStateInit()
{
	
}


void ARevenant::BeginTelegraph(const FTelegraphParams& InParams)
{
	//GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Green,TEXT("BeginTelegraph"));

	Params = InParams;
	Phase = ETelegraphPhase::Warn;
	ChangeParams(); // 서버 로컬 즉ㄷ시 반영? 솔로니까 그냥 적용?
	ChangePhase();
	if (Params.WarnTime > 0.f)
	{
		GetWorld()->GetTimerManager().SetTimer(
			PhaseTimerHandle, this, &ARevenant::ActivatePhase, InParams.WarnTime, false);
	}
	else
	{
		ActivatePhase();
	}
	//ActivatePhase();
}

void ARevenant::ActivatePhase()
{
	//GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Green,TEXT("ActivePhase"));
	Phase = ETelegraphPhase::Active;
	ChangePhase();

	const float ActiveDur = FMath::Max(Params.ActiveTime, 0.0f);
	GetWorld()->GetTimerManager().SetTimer(
		PhaseTimerHandle, this, &ARevenant::EndTelegraph, ActiveDur, false);

	// 여기서 실제 공격 판정(서버) 호출하면 된다.
	// ex) MyOwner->DoHitCheck(RepParams, ...);
}

void ARevenant::EndTelegraph()
{
	Phase = ETelegraphPhase::Ending;
	ChangePhase();
	// 페이드 후 정리
	const float FadeDur = FMath::Max(Params.FadeOutTime, 0.05f);
	GetWorld()->GetTimerManager().SetTimer(
		PhaseTimerHandle, [this]()
		{
			// 최종 파괴
			Phase = ETelegraphPhase::None;
			ChangePhase();
			//Phase(); // 이거 온렙일때만 하면 되지ㅣ않음?
			Params = FTelegraphParams(); // 리셋
		}, FadeDur, false);
}

void ARevenant::ShowRectTelegraph(const FVector& CenterWS, const FVector& ForwardWS,
                                  //float Length, float Width,
                                  float WarnTime, float ActiveTime, float FadeTime)
{
	const FVector F = ForwardWS.GetSafeNormal();

	// 스윕 시작점 = 텔레그래프 CenterWS (Z는 이전처럼 -88 보정 또는 라인트레이스)
	FVector centerWS = CenterWS + F * 50.f;
	centerWS.Z = GetActorLocation().Z - 88.f; // 현재 방식 유지(필요시 라인트레이스로 대체)

	const float Travel = FMath::Max(End - 50.f, 0.f);
	const float matLength = Travel + (HalfSize.X * 2.f); // ← 스윕 경로 + 박스 길이
	const float matWidth = HalfSize.Y * 2.f;

	//GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::White, TEXT("matLength : %f"), matLength);
	UE_LOG(LogTemp, Warning, TEXT("MatLength : %f"), matLength);
	FTelegraphParams TP;
	TP.Shape = ETelegraphShape::Rectangle; // ★ Rectangle
	TP.CenterWS = centerWS;
	TP.ForwardWS = ForwardWS.GetSafeNormal(); // 진행 방향
	TP.Length = matLength; // 전방 길이
	TP.Width = matWidth; // 폭
	//TP.Radius      = FMath::Max(Length, Width);    // (머티리얼 내부에선 쓰지 않아도 안전빵)
	TP.Feather = 60.f;

	TP.WarnTime = WarnTime; // 0→1 상승 시간(AlphaSpeed 기준 보간)
	TP.ActiveTime = ActiveTime; // 유지
	TP.FadeOutTime = FadeTime; // 1→0 하강 시간

	BeginTelegraph(TP);
	// 표시 시작

	//ActiveTelegraph->ApplyParams(TP);              // 머티리얼 파라미터/스케일/회전 적용
	//ActiveTelegraph->SetPhase(ETelegraphPhase::Warn); // 상승 시작 → 내부 Tick이 Alpha를 1까지 올리고 Active 유지 후 FadeOut
}

void ARevenant::ShowCircleTelegraph(const FVector& centerWS, float _radius, float _halfAngleDeg, float _warnTime,
                                    float _activeTime, float _fadeOutTime)
{
	FTelegraphParams TP;
	TP.Shape = ETelegraphShape::Circle; // ★ Rectangle
	TP.CenterWS = centerWS;
	TP.Radius = _radius;
	TP.ForwardWS = GetActorForwardVector(); //ForwardWS.GetSafeNormal();    // 진행 방향
	TP.Length = _radius; // 전방 길이
	TP.Width = _radius * 2; // 폭
	//TP.Radius      = FMath::Max(Length, Width);    // (머티리얼 내부에선 쓰지 않아도 안전빵)
	TP.Feather = 60.f;
	TP.HalfAngleDeg = _halfAngleDeg;
	TP.WarnTime = _warnTime; // 0→1 상승 시간(AlphaSpeed 기준 보간)
	TP.ActiveTime = _activeTime; // 유지
	TP.FadeOutTime = _fadeOutTime; // 1→0 하강 시간

	BeginTelegraph(TP);
}

void ARevenant::EnableCounter()
{
	SetOutLineEnable(true);
	bCanCounter = true;
	TWeakObjectPtr<ARevenant> WeakThis(this);
	FTimerHandle CounterTimerHandle;
	GetWorld()->GetTimerManager().SetTimer(CounterTimerHandle, [WeakThis, this]()
	{
		if (!WeakThis.IsValid()) return;
		SetOutLineEnable(false);
		bCanCounter = false;
	}, 0.5f, false);
}

void ARevenant::ChangeParams()
{
	EnsureLocalIndicator();
	if (LocalIndicator)
	{
		LocalIndicator->ApplyParams(Params);
		// 서버에서 머티리얼 지정이 없었어도 Base를 넘겨주면 Indicator가 사용
		if (LocalIndicator->GetClass()->FindPropertyByName(TEXT("TelegraphMIBase")) && TelegraphMIBase)
		{
			// 안전: C++ 버전은 내부에서 이미 보유. BP 파생에 대비해 남겨둠(선택)
		}
	}
}

void ARevenant::ChangePhase()
{
	EnsureLocalIndicator();

	if (!LocalIndicator)
		return;

	switch (Phase)
	{
	case ETelegraphPhase::Warn:
		LocalIndicator->SetPhase(ETelegraphPhase::Warn);
		break;
	case ETelegraphPhase::Active:
		LocalIndicator->SetPhase(ETelegraphPhase::Active);
		break;
	case ETelegraphPhase::Ending:
		LocalIndicator->SetPhase(ETelegraphPhase::Ending);
		//StartLocalFadeOut(FMath::Max(Params.FadeOutTime, 0.05f)); 
		break;
	case ETelegraphPhase::None:
	default:
		DestroyLocalIndicator();
		break;
	}
}

void ARevenant::EnsureLocalIndicator()
{
	if (LocalIndicator && !IsValid(LocalIndicator))
	{
		LocalIndicator = nullptr;
	}

	if (!LocalIndicator && GetWorld())
	{
		TSubclassOf<ATelegraphActor> UseClass = IndicatorClass;
		if (!UseClass) { UseClass = ATelegraphActor::StaticClass(); }

		FActorSpawnParameters P;
		P.Owner = this;
		P.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		LocalIndicator = GetWorld()->SpawnActor<ATelegraphActor>(UseClass, Params.CenterWS, FRotator::ZeroRotator, P);
		if (TelegraphMIBase && LocalIndicator && LocalIndicator->TelegraphMIBase == nullptr)
		{
			LocalIndicator->TelegraphMIBase = TelegraphMIBase;
		}
		if (LocalIndicator)
		{
			LocalIndicator->ApplyParams(Params);
			LocalIndicator->SetPhase(Phase);
		}
	}
}

void ARevenant::DestroyLocalIndicator()
{
	if (LocalIndicator)
	{
		LocalIndicator->Destroy();
		LocalIndicator = nullptr;
	}
}

void ARevenant::StartLocalFadeOut(float Duration)
{
	if (LocalIndicator == nullptr) return;

	// 타이머 기반 페이드
	FadeAlpha = 0.f;

	GetWorld()->GetTimerManager().SetTimer(
		FadeTimerHandle,
		[this, Duration]()
		{
			if (!LocalIndicator)
			{
				GetWorld()->GetTimerManager().ClearTimer(FadeTimerHandle);
				return;
			}
			const float Dt = GetWorld()->GetDeltaSeconds();
			FadeAlpha = FMath::Clamp(FadeAlpha + Dt / FMath::Max(Duration, 1e-3f), 0.f, 1.f);
			//LocalIndicator->SetOpacity(FMath::Lerp(1.f, 0.f, FadeAlpha)); 이 역할은 Tick에서 함
			if (FadeAlpha >= 1.f)
			{
				//DestroyLocalIndicator();
				GetWorld()->GetTimerManager().ClearTimer(FadeTimerHandle);
			}
		},
		0.f, true);
}
