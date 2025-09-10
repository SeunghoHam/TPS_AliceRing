// Fill out your copyright notice in the Description page of Project Settings.


#include "Alice/Alice.h"
#include "AliceController.h"
#include "SnapCameraManager.h" // UObject 기반 SnapManager
#include "Kismet/GameplayStatics.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h" // GetCharacterMovement()
#include "EngineUtils.h" // TActorIterator
#include "ZZZEnemy.h"
#include "System/EnemyRegistrySubsystem.h"
#include "DamageNumberActor.h"
#include "DamageNumberSubsystem.h"
#include "Animation/AliceAnimInstance.h"
#include "AlicePlayerWidget.h"
#include "Doro.h"
#include "Engine/SkeletalMeshSocket.h" // AttachWeapon
#include "Alice/System/SoundComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Alice/DialogueDataAsset.h"
#include "Components/TextBlock.h"
#include "Engine/BlockingVolume.h"
#include "Revenant/Revenant.h"

// Sets default values
AAlice::AAlice()
{
	PrimaryActorTick.bCanEverTick = true;

	ShoulderPivot = CreateDefaultSubobject<USceneComponent>(TEXT("ShoulderPivot"));
	ShoulderPivot->SetupAttachment(RootComponent);

	//ShoulderPivot->SetRelativeLocation(FVector(0.0f, 60.0f, 40.0f));
	ShoulderPivot->SetRelativeLocation(FVector(0.0f, 0.0f, 50.0f));

	// Spring Arm 생성
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(ShoulderPivot);
	SpringArm->TargetArmLength = 400.f; // 카메라 거리 (약간 짧은 편)
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
	SpringArm->bUsePawnControlRotation = true; // 입력에 따라 회전  원래 false


	// Follow Camera 생성
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(SpringArm);
	FollowCamera->FieldOfView = 85.f;

	// Pivot 설정변경
	FollowCamera->bUsePawnControlRotation = false; // 카메라는 직접 회전 제어안함 (원래 fasle

	SpringArm->SocketOffset = FVector::ZeroVector;
	SpringArm->TargetOffset = FVector(0, 0, 10.f);
	// 캐릭터 회전 관련 설정

	bUseControllerRotationYaw = false; // 캐릭터가 카메라 회전에 따라 회전되지 않도록
	GetCharacterMovement()->bOrientRotationToMovement = true; // 이동 방향 기준으로 회전
	GetCharacterMovement()->RotationRate = FRotator(0.f, 1280.f, 0.f); // 회전 속도 빠르게

	//SoundComponent = CreateDefaultSubobject<USoundComponent>(TEXT("SoundComponent"));

	static ConstructorHelpers::FClassFinder<ADamageNumberActor> DamageActorBPClass(
		TEXT("/Game/A_ProjectAlice/Actor/DamageNumber/BP_DamageNumber.BP_DamageNumber_C"));
	static ConstructorHelpers::FClassFinder<UAlicePlayerWidget> PlayerWidgetBPClass(
		TEXT("/Game/A_ProjectAlice/Widgets/WBP_AlicePlayer.WBP_AlicePlayer_C"));
	static ConstructorHelpers::FClassFinder<AActor> WeaponTemplateBPClass(
		TEXT("/Game/A_ProjectAlice/Actor/Sword/BP_Katana.BP_Katana_C"));
	/*
	static ConstructorHelpers::FObjectFinder<UDialogueDataAsset> DialogueAssetObject(
		TEXT("/Game/A_ProjectAlice/DataAsset/DialogueDataAsset.DialogueDataAsset"));*/
	if (DamageActorBPClass.Class != nullptr)
	{
		DamageNumberActorClass = DamageActorBPClass.Class;
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[Alice] DamageActor Direction is Wrong "));
	}
	if (PlayerWidgetBPClass.Class != nullptr)
	{
		PlayerWidgetClass = PlayerWidgetBPClass.Class;
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[Alice] PlayerWidget Direction is Wrong "));
	}
	if (WeaponTemplateBPClass.Class != nullptr)
	{
		WeaponTemplate = WeaponTemplateBPClass.Class;
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[Alice] Katana Direction is Wrong "));
	}
	/*
	if (DialogueAssetObject.Succeeded())
	{
		DialogueData = DialogueAssetObject.Object;
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[Alice] DialogueDataAsset  is Wrong "));
	}*/
}

void AAlice::BeginPlay()
{
	Super::BeginPlay();
	StatusInitialize(200, 10); // 체력 초기화
	AnimInstance = Cast<UAliceAnimInstance>(GetMesh()->GetAnimInstance());
	SnapManager = NewObject<USnapCameraManager>(this);

	if (SnapManager)
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Cyan, TEXT("Get SnapCamera"));
		SnapManager->Initialize(FollowCamera, SpringArm, ShoulderPivot); // 카메라 전달
	}
	AnimInstance->SetAliceAnimState(EAliceAnimState::Idle);
	if (PlayerWidgetClass != nullptr)
	{
		PlayerWidget = CreateWidget<UAlicePlayerWidget>(GetWorld(), PlayerWidgetClass);
		if (PlayerWidget)
		{
			PlayerWidget->AddToViewport();
			PlayerWidget->BindCharacter(this); // StatusInitialize 후에 하기
		}
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, TEXT("AlicePlayerWidgetClass = null"));
	}

	if (DialogueDataAsset!= nullptr)
	{
		
		PlayerWidget->DialogueData = Cast<UDialogueDataAsset>( DialogueDataAsset.Get());
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, TEXT("DialogueDataAsset = null"));
	}

	AttachWeapon(WeaponTemplate);

	
	UpdateHuntUI();
	GetWidget()->DiedPanelActive(false);
	GetWidget()->BossUIActive(false);
	GetWorld()->GetTimerManager().SetTimer(DoroSpawnTimer, this, &AAlice::SpawnDoro, 1.0f, false);
	//SetTarget();
}

void AAlice::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (SnapManager && bCameraActive)
	{
		SnapManager->TickSoftLock(DeltaTime);
		SnapManager->FixShoulderCamera(DeltaTime); // 컨트롤러 회전 반영 직후
		SnapManager->TickSnap(DeltaTime); // 스냅은 그 다음에 적용되도록
	}

	if (bIsCharged) // 차지모드 활성화
	{
		// 우클릭 차지 상태
		if (bIsRMClick) RMCheckTime += DeltaTime;
		else RMCheckTime -= DeltaTime;

		RMCheckTime = FMath::Clamp(RMCheckTime, 0.0f, 1.0f);
		PlayerWidget->CheckTime = RMCheckTime;

		SpringArm->TargetArmLength = 300 - (RMCheckTime * 120.0f);
	}
	
	//if (!bIsMoving) return;
	if (bIsMoving)
	{
		ElapsedTime += DeltaTime;
		float Alpha = FMath::Clamp(ElapsedTime / AttackMoveTime, 0.f, 1.f);
		NewLoc = FMath::Lerp(StartLoc, TargetLoc, Alpha);
		SetActorLocation(NewLoc);
		if (Alpha >= 1.0f)
		{
			bIsMoving = false;
			ElapsedTime = 0;
		}
	}
}

void AAlice::BlockingVolumeActive(bool _value)
{
	ABlockingVolume* volumeActor = BlockingVolumeActor.Get();
	volumeActor->SetActorEnableCollision(_value);
	//BlockingVolumeActor.Get() =_value;
}

void AAlice::SetSpringArmLength(float _amount)
{
	float Current = _amount;

	// 0일때 250
	// -50 일때 100
	// 10 일때 100

	// 60 100
	// 50 250
	// 0 100
}

void AAlice::OpenComboWindow()
{
	bIsCanAttack = true;
	bComboWindowOpen = true;
}

void AAlice::CloseComboWindow()
{
	bIsCanAttack = false;
	bComboWindowOpen = false;

	if (bBufferedNext)
	{
		bBufferedNext = false;
		if (ComboCountIndex == 1)
		{
			AttackState = EAttackState::Attacking;
			AnimInstance->SetAliceAnimState(EAliceAnimState::B);
			ComboCountIndex++;
			AttackMoveAmount = 50.0f;
			AttackMoveTime = 0.15f;
		}
		else if (ComboCountIndex == 2)
		{
			AnimInstance->SetAliceAnimState(EAliceAnimState::C);
			ComboCountIndex++;
			AttackMoveAmount = 150.0f;
			AttackMoveTime = 0.2f;
		}
		else if (ComboCountIndex == 3)
		{
			AnimInstance->SetAliceAnimState(EAliceAnimState::D);
			ComboCountIndex++;
			AttackMoveAmount = 200.0f;
			AttackMoveTime = 0.2f;
		}
		else
		{
			GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Cyan, TEXT("최대콤보. 아무것도 안하기"));
		}
		// 끝나면 다음거로 진행하도록
	}
	else
	{
		ResetCombo();
		//OnRecover();
	}
}

void AAlice::DoHit()
{
	FVector dir = GetActorForwardVector();
	if (CurrentTarget != nullptr)
	{
		dir = (CurrentTarget->GetActorLocation() - GetActorLocation()).GetSafeNormal();
		//dir.Z = GetActorLocation().Z;
	}
	StartLoc = GetActorLocation();
	FVector Check = CheckFront(AttackMoveAmount);
	if (Check == FVector::ZeroVector)
	{
		// 닿은 객체가 없음
		TargetLoc = (dir * AttackMoveAmount) + GetActorLocation();
	}
	else
	{
		TargetLoc = Check;
	}

	//PlayerWidget->ShowByIndex(0);
	//PlayerWidget->ShowByKey("1");
	bIsMoving = true; // 이거 같이 써도 될것같은데?
	PlaySound(ESoundName::effect_Slash1, 0.5f);
	RandomAttackVocal();
	CreateAttackCollision();
}

void AAlice::OnMontageEnded()
{
	// 섹션이 끝나고도 입력이 없는상태 = 리셋시킴
	// 넘어갈거면 CloseComboWindow 종료지점에서 넘김
	ResetCombo();
}

void AAlice::OnRecover()
{
	AnimInstance->SetAliceAnimState(EAliceAnimState::Recover);
}

void AAlice::ChargeEnd()
{
	// 안쓴다뎃스
	/*
	if (RMCheckTime >= 1.0f)
	{
		bIsChargeSuccess = true;
	}
	else
	{
		bIsChargeSuccess = true;
	}*/
}

void AAlice::Death()
{
	AttackState = EAttackState::Death;
	AnimInstance->SetAliceAnimState(EAliceAnimState::Death);
	AliceController->SetOnLookActive(false);
	if (Boss)
	{
		Boss->bIsTreeActive = false;
	}
	GetWidget()->DiedPanelActive(true);
	
	AliceController->bShowMouseCursor = true;
	GetWidget()->FullImageShow(true);
}

void AAlice::SetTarget()
{
	AActor* EnemyTarget = FindNearestEnemy_Registry(1000.f); // FLT_MAX

	if (!EnemyTarget)
	{
		CurrentTarget = nullptr;
		return;
	}
	CurrentTarget = Cast<AZZZEnemy>(EnemyTarget);
	GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Blue,
	                                 FString::Printf(TEXT("현재 타겟 : %s"), *CurrentTarget->GetName()));
}

void AAlice::LookTarget()
{
	if (CurrentTarget == nullptr) return;

	const FRotator newRot = (CurrentTarget->GetActorLocation() - GetActorLocation()).GetSafeNormal().Rotation();
	const float YawOnly = newRot.Yaw;
	SetActorRotation(FRotator(0.f, YawOnly, 0.f), ETeleportType::None);
}

void AAlice::HuntCountUp()
{
	HuntCount = FMath::Clamp(HuntCount +=1, 0, MaxHuntCount);
	UpdateHuntUI();
	if (HuntCount >= MaxHuntCount)
	{
		//FText qn = FText::FromString("보스를 처치해");
		PlayerWidget->ChangeQuestName();
		PlayerWidget->QuestDirection->SetVisibility(ESlateVisibility::Hidden);
		// 도로롱 잡기 클리어 
	}
	else
	{
		SpawnDoro();
	}
}

void AAlice::AttachWeapon(TSubclassOf<AActor> _weapon)
{
	if (WeaponTemplate) // WeaponTemplate is valid?
	{
		//UE_LOG(LogTemp, Log, TEXT("Try Attach1"));
		AActor* SpawnWeapon = GetWorld()->SpawnActor<AActor>(WeaponTemplate);

		//weapon = Cast<AActor>(SpawnWeapon);

		const USkeletalMeshSocket* WeaponSocket = GetMesh()->GetSocketByName("WeaponPoint");

		if (WeaponSocket && SpawnWeapon)
		{
			GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Red, TEXT("[Alice] WeaponAttach"));

			//"Engine/SkeletalMeshSocket.h" 헤더파일 필요
			WeaponSocket->AttachActor(SpawnWeapon, GetMesh());
		}
		else
		{
			GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Red, TEXT("[Alice] Soocket Null!!!"));
		}
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Red, TEXT("[Alice] WeaponTemplate Null!!!"));
	}
}

void AAlice::BossStageInit()
{
	
}

void AAlice::TryAttack()
{
	// 예시: 가장 가까운 적 방향으로 Snap 연출
	/*
	AActor* EnemyTarget = FindNearestEnemy_Registry(FLT_MAX);
	if (EnemyTarget == nullptr)
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Red, TEXT("TargetIsNull"));
		return;
	}*/
	if (!bIsCanAttack) return;
	if (!CanTryAttack()) return;

	SetTarget();
	bIsCanAttack = false;
	LookTarget();
	if (AttackState == EAttackState::Attacking && bComboWindowOpen && !bBufferedNext)
	{
		// 다음 공격으로 보내는 쪽
		bBufferedNext = true; // 입력 버퍼링 줌
		AttackState = EAttackState::Attacking;
		return;
	}

	if (AttackState == EAttackState::Idle)
	{
		// 공격상태가 아니었다면 1단계부터 시작
		CurrentStepIndex = 0;
		AnimInstance->SetAliceAnimState(EAliceAnimState::A);
		AttackState = EAttackState::Attacking;
		AttackMoveAmount = 50.0f;
		AttackMoveTime = 0.15f;
		ComboCountIndex++;
	}
	// XY Override : 수평 속도를 기존값과 합칠지, 덮어쓸지 결정
	// true : 현재 캐릭터의 XY 속도를 무시하고 덮어씀
	// false : 현재 속도 + LaunchVelocity를 합산
	// Z Override : 수직 
	//LaunchCharacter(MoveDirection,true,false);
	// 점프라면 false, true
}


void AAlice::ResetCombo()
{
	//FTimerHandle ResetTimer;
	GetWorld()->GetTimerManager().ClearTimer(ResetHandle);
	GetWorld()->GetTimerManager().SetTimer(ResetHandle, this, &AAlice::DelayReset
 ,0.12f, false);
}

void AAlice::DelayReset()
{
	AttackState = EAttackState::Idle;
	AnimInstance->SetAliceAnimState(EAliceAnimState::Idle);
	ComboCountIndex = 0;
	bBufferedNext = false;
	bComboWindowOpen = false;
	bIsCanAttack = true;
	bIsChargeSuccess = false;
}

void AAlice::CreateAttackCollision(FVector _halfSize, float _end)
{
	//FVector Forward = GetActorForwardVector();
	FVector CollisionStart = (GetActorForwardVector() * 50) + GetActorLocation();
	FVector CollisionEnd = (GetActorForwardVector() * _end) + GetActorLocation();

	TArray<FHitResult> OutHits;
	FRotator Orient = GetActorRotation();
	FQuat Rot = Orient.Quaternion();

	FCollisionShape BoxShape = FCollisionShape::MakeBox(_halfSize);
	// FCollisionQueryParams& Params;
	bool bHit = GetWorld()->SweepMultiByChannel(
		OutHits,
		CollisionStart,
		CollisionEnd,
		Rot,
		ECC_Visibility,
		BoxShape);

	FVector Center = (CollisionStart + CollisionEnd) * 0.5f;
	FVector BoxExtend = _halfSize;

	/*
	DrawDebugBox(
		GetWorld(),
		Center,
		BoxExtend,
		Rot,
		FColor::Blue,
		false,
		0.5f);
*/
	if (bHit)
	{
		for (const FHitResult& HitActor : OutHits)
		{
			if (HitActor.GetActor()->ActorHasTag("Enemy"))
			{
				//AAliceBase* enemy = Cast<AAliceBase>(HitActor.GetActor());
				AZZZEnemy* enemy = Cast<AZZZEnemy>(HitActor.GetActor());
				enemy->GetDamaged(25.f);
				//ShowDamage();
			}
		}
	}
}

void AAlice::LM_Press()
{
	if (bIsRMClick) return;
	bIsRMClick = true;
	SetTarget();
	if (CurrentTarget == nullptr)
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Red, TEXT("TargetIsNull"));
		//return;
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Red,
		                                 FString::Printf(TEXT("Target : %s"), *CurrentTarget->GetName()));
	}
	// 오프셋 조금 설정하기
	SnapManager->EnableSoftLock(CurrentTarget, FVector(0, 0, -80.f));
	SnapManager->StartSnapPreset(ESnapPreset::Hold_Part1, CurrentTarget);

	//StartShowDamage();
}


void AAlice::Rm_Press()
{
	if (bIsRMClick) return;
	bIsRMClick = true;
	if (bIsCharged)
	{
		// 강화된 상태에서 차징공격 준비
		AnimInstance->SetAliceAnimState(EAliceAnimState::ChargeAttackReady);
		if (CurrentTarget)
		{
			SnapManager->EnableSoftLock(CurrentTarget);
		}
	}
	else
	{
		// 비차지 상태에서 

		// 영상 재생
		PlayerWidget->ToggleView();
		PlayerWidget->SetVideoOpacityEnable(true);
		PlayerWidget->Video_ChargeModeGo();

		TWeakObjectPtr<AAlice> WeakThis(this);
		GetWorld()->GetTimerManager().ClearTimer(VideoDelayTimer);
		GetWorld()->GetTimerManager().SetTimer(VideoDelayTimer, [WeakThis, this]()
		{
			// 강화 충전단계
			// 영상 종료 후 실행되도록
			if (!WeakThis.IsValid()) return;
			PlayerWidget->SetVideoOpacityEnable(false);
			PlayerWidget->IsChargeState = true;
			PlayerWidget->ToggleView();

			bIsCharged = true;
			AnimInstance->SetAliceAnimState(EAliceAnimState::GoChargeMode);
		}, 1.1f, false);
	}
}

void AAlice::RM_Release()
{
	bIsRMClick = false;
	if (!bIsCharged) return;

	if (RMCheckTime >= 1.0f)
	{
		bIsChargeIsFull = true;
	}
	else bIsChargeIsFull = false;

	if (!bIsChargeIsFull)
	{
		// 충전 다 안됨
		AnimInstance->SetAliceAnimState(EAliceAnimState::Idle);
		SnapManager->StartSnapPreset(ESnapPreset::Return, nullptr);
	}
	else
	{
		// 충전 다 됨
		AnimInstance->SetAliceAnimState(EAliceAnimState::ChargeAttack);
		PlayerWidget->IsChargeState = false;
		PlayerWidget->SetBackGroundReset();
		SnapManager->StartSnapPreset(ESnapPreset::ChargeAttack, CurrentTarget);
		bIsCharged = false;
		//SoundComponent->Play(ESoundName::vocal_Laugh, GetMesh(),FName("Mouth"));
	}
}

void AAlice::Shift_Press()
{
	if (bIsAvoiding) return;
	if (AttackState != EAttackState::Idle) return;

	//GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Blue, TEXT("Avoid!"));
	AttackState = EAttackState::Avoid;
	SnapManager->StartSnapPreset(ESnapPreset::Avoid, nullptr);
	AnimInstance->SetAliceAnimState(EAliceAnimState::Avoid);

	float refreshTime = 0.3f;
	AttackMoveTime = refreshTime;
	StartLoc = GetActorLocation();

	FVector Check = CheckFront(400);
	if (Check == FVector::ZeroVector)
	{
		// 닿은 객체가 없음
		TargetLoc = (GetActorForwardVector() * 400) + GetActorLocation();
	}
	else
	{
		TargetLoc = Check;
	}
	// LaunchCharacter(GetActorLocation()+(TargetLoc - GetActorLocation().GetSafeNormal()) * FVector::Dist(Check, GetActorLocation()),
	//	true, false);
	bIsMoving = true;
	bIsAvoiding = true;
	PlayerWidget->AvoidPanelActive(true);
	TWeakObjectPtr<AAlice> WeakThis(this);
	GetWorld()->GetTimerManager().ClearTimer(AvoidTimerHandle);
	GetWorld()->GetTimerManager().SetTimer(AvoidTimerHandle, [WeakThis, this]()
	{
		if (!WeakThis.IsValid()) return;
		PlayerWidget->AvoidPanelActive(false);
		bIsAvoiding = false;
		bIsMoving = false;
		AttackState = EAttackState::Idle;
		AnimInstance->SetAliceAnimState(EAliceAnimState::Idle);
		EndBulletTime();
		GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Green, TEXT("Avoid End!"));
	}, refreshTime, false);
}

void AAlice::Shift_Release()
{
	
}

void AAlice::R_Press()
{
	GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Yellow,TEXT("[Alice] R_Press"));

	// 포탈 바라보면서 사라지게 하기
	SpringArm->bUsePawnControlRotation = false;
	bUseControllerRotationYaw = true; // 캐릭터가 카메라 회전에 따라 회전되지 않도록
	SpringArm->bInheritYaw = false; // 좌우 움직임
	GetWidget()->FullImageShow(true);
	PortalDestroy();

	FTimerHandle handle;
	GetWorld()->GetTimerManager().SetTimer(handle,this, &AAlice::BossBattleSetting, 1.5f, false);
}

void AAlice::BossBattleSetting()
{
	SetActorRotation(FRotator(0.f, 0.f, 90.f));
	MoveEmotePointActor();
	GetWidget()->FullImageShow(false);
	SpringArm->bUsePawnControlRotation = true;
	bUseControllerRotationYaw = false; // 캐릭터가 카메라 회전에 따라 회전되지 않도록
	SpringArm->bInheritYaw = true; // 좌우 움직임

	GetWidget()->BossUIActive(true);


	TWeakObjectPtr<AAlice> WeakThis(this);
	FTimerHandle timeHandle;
	GetWorld()->GetTimerManager().SetTimer(timeHandle, [WeakThis, this]()
	{
		if (!WeakThis.IsValid()) return;
		Boss->SetTreeActive();
	}, 1.0f, false);
}


void AAlice::AvoidAction()
{
	Super::AvoidAction();
	GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Yellow,TEXT("[Alice] Avoid Success"));
	StartBulletTime(0.4f, 0.9f, 0.8f);
}

void AAlice::CheckCurrentHP()
{
	if (AttackState == EAttackState::Action) return;
	AddHP(-Damage);
	AddMP(-1);
	GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::White, FString::Printf(TEXT("CurrentHP : %f"), CurrentHP));
	if (CurrentHP <= 0.f)
	{
		Death();
	}
	else
	{
		AttackState = EAttackState::Action;
		AnimInstance->SetAliceAnimState(EAliceAnimState::Hit);
	}
}

void AAlice::CheckCurrentMP()
{
	//Super::CheckCurrentMP();
}

void AAlice::TryParring()
{
	if (AttackState != EAttackState::Idle)return;
	if (CurrentTarget == nullptr)
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Red,TEXT("[Paring] TargetIsNull"));
		return;
	}

	// 도로가 bIsAngry= true라면
	if (CurrentTarget->CanCounterAttack())
	{
		if (CurrentTarget == Boss)
		{
			Boss->SetGroggyPrev();
		}
		ParringSound();
		LookTarget();
		AttackState = EAttackState::Parring;
		AnimInstance->SetAliceAnimState(EAliceAnimState::Parring);
		bIsCanAttack = false;

		StartLoc = GetActorLocation();
		float Distance = FVector::Dist(GetActorLocation(), CurrentTarget->GetActorLocation());
		FVector dir = GetActorLocation() + GetActorForwardVector() * 100;
		//if (Distance > 500.0f)
		{
			dir = ((GetActorLocation() - CurrentTarget->GetActorLocation()).GetSafeNormal() * 150.0f) + CurrentTarget->
				GetActorLocation();
			dir.Z = GetActorLocation().Z;
		}
		TargetLoc = dir;

		AttackMoveTime = 0.6f;
		bIsMoving = true;

		// 오프셋 설정

		SnapManager->EnableSoftLock(CurrentTarget, FVector(-40.f, 0, 0.f));
		//SnapManager->EnableSoftLock(CurrentTarget,FVector(0,0,20.f));
		SnapManager->StartSnapPreset(ESnapPreset::Parring, CurrentTarget);


		//GetWorld()->GetTimerManager().SetTimer(timeHandle,)

		TWeakObjectPtr<AAlice> WeakThis(this);
		FTimerHandle timeHandle;
		GetWorld()->GetTimerManager().SetTimer(timeHandle, [WeakThis, this]()
		{
			if (!WeakThis.IsValid()) return;
			Boss->SetGroggy();
		}, AttackMoveTime, false);
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Blue, TEXT("도로 화 안남"));
	}
}

AActor* AAlice::FindNearestEnemy()
{
	// 가장 가까운 Enemy 태그 액터 찾기
	float ClosestDist = FLT_MAX;
	AActor* Closest = nullptr;

	for (TActorIterator<AActor> It(GetWorld()); It; ++It)
	{
		if (It->ActorHasTag("Enemy"))
		{
			float Dist = FVector::Dist(GetActorLocation(), It->GetActorLocation());
			DrawDebugLine(GetWorld(), GetActorLocation(), It->GetActorLocation(), FColor::Green, false, 1.0f);
			if (Dist < ClosestDist)
			{
				Closest = *It;
				ClosestDist = Dist;
			}
		}
	}
	return Closest;
}

void AAlice::BindEnemyWidget()
{
	//ARevenant* Boss =  CurrentTarget
	//PlayerWidget->BindBoss(CurrentTarget);
}

AActor* AAlice::FindNearestEnemy_Registry(float MaxRadius)
{
	auto* R = GetGameInstance()->GetSubsystem<UEnemyRegistrySubsystem>();
	if (!R) return nullptr;

	const FVector C = GetActorLocation();
	const float MaxR2 = MaxRadius * MaxRadius;

	float BestD2 = FLT_MAX;
	AActor* Best = nullptr;

	for (auto& Weak : R->GetEnemies())
	{
		AActor* E = Weak.Get();
		if (!IsValid(E)) continue;

		const float d2 = FVector::DistSquared(C, E->GetActorLocation());
		if (d2 < BestD2 && d2 <= MaxR2 /* && E->IsTargetable() */)
		{
			BestD2 = d2;
			Best = E;
		}
	}
	return Best;
}

void AAlice::SetBoss(ARevenant* _boss)
{
	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Magenta,
	                                 FString::Printf(TEXT("[Alice] Boss : %s"), *_boss->GetName()));
	Boss = _boss;
	if (PlayerWidget)
	{
		PlayerWidget->BindBoss(Boss);
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red,TEXT("[Alice] SetBoss : PlayerWidget = null"));
	}
}

bool AAlice::IsActionState() const
{
	if (AttackState == EAttackState::Idle) return false;
	return true;
}

void AAlice::StartBulletTime(float _worldFactor /*ex:0.2f*/,float _localFactor ,float RealDuration /*sec*/)
{
	// 월드/자신 시간 분리 슬로모 시작
	// WorldScale: 전체 월드 배속 (예: 0.2f면 5배 느리게)
	// SelfScale : "내 캐릭터의 절대 배속" (예: 1.0f면 정상, 0.5f면 절반 속도, 2.0f면 2배 빨라짐)
	UWorld* W = GetWorld();
	if (!W) return;

	// 중복 적용 방지: 기존 효과 종료
	EndBulletTime();

	// 기존 값 저장
	SavedGlobalTimeDilation = W->GetWorldSettings()->TimeDilation;   // 현재 전역 속도
	// CustomTimeDilation = _localFactor; // 현재 내 커스텀 속도

	
	// 1) 월드 배속 적용
	//    (0에 가까운 값 보호)
	const float ClampedWorld = FMath::Clamp(_worldFactor, 0.001f, 100.f);
	UGameplayStatics::SetGlobalTimeDilation(W, ClampedWorld);

	// 2) 내 캐릭터 절대 배속을 맞추기 위한 CustomTimeDilation 계산
	//    실제 적용 속도 = Global * Custom  =>  Custom = SelfScale / Global
	const float ClampedSelfAbs = FMath::Clamp(_localFactor, 0.001f, 100.f);
	const float CustomForSelf  = ClampedSelfAbs / ClampedWorld;
	CustomTimeDilation = CustomForSelf;

	// 3) 실시간(undilated) 기반 종료 예약
#if ENGINE_MAJOR_VERSION >= 5
	// UE5에선 실시간 타이머 지원. (엔진 버전에 따라 시그니처가 다를 수 있어 아래 기본형 사용)
	W->GetTimerManager().SetTimer(
		BulletTimeTimer,
		this, &AAlice::EndBulletTime,
		RealDuration,
		false /*bLoop*/
	);
#endif
}

void AAlice::EndBulletTime()
{
	UWorld* W = GetWorld();
	if (!W) return;

	// 전역 복원
	UGameplayStatics::SetGlobalTimeDilation(W, SavedGlobalTimeDilation);
	SavedGlobalTimeDilation = 1.f;

	CustomTimeDilation= 1.f;
	
	// 플레이어 보정 해제
	//SetCustomTimeDilation(1.f);
	// 타이머 정리
	if (BulletTimeTimer.IsValid())
	{
		W->GetTimerManager().ClearTimer(BulletTimeTimer);
	}
}

void AAlice::SetAliceAttackState(const EAttackState& _state)
{
	AttackState = _state;
}

void AAlice::SpawnDoro()
{
	DoroArray[HuntCount]->SetDoroActive(true);
	//DoroSpawnPointArray[HuntCount]->SpawnDoro();
}

void AAlice::UpdateHuntUI()
{
	PlayerWidget->ChangeQuestDireciton(HuntCount, MaxHuntCount);
}


bool AAlice::CanTryAttack()
{
	bool b = AttackState == EAttackState::Idle || AttackState == EAttackState::Attacking;
	return b;
}

FVector AAlice::CheckFront(float _length)
{
	FVector maxLocation = FVector::ZeroVector;
	FHitResult Hit;
	// ECollisionChannel Channel;
	FCollisionQueryParams Params(NAME_None, false, this);
	Params.AddIgnoredActor(this);
	bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, GetActorLocation(),
	                                                 GetActorLocation() + (GetActorForwardVector() * _length),
	                                                 ECC_Visibility, Params);
	if (bHit)
	{
		maxLocation = Hit.Location + (Hit.ImpactNormal * 50);
		maxLocation.Z = GetActorLocation().Z;
		DrawDebugSphere(GetWorld(), maxLocation, 32.f, 16, FColor::Green, false, 1.0f);
		return maxLocation;
	}
	DrawDebugLine(GetWorld(), GetActorLocation(), GetActorForwardVector() * _length, FColor::White, false, 1.0f);
	return maxLocation;
}

void AAlice::RandomAttackVocal()
{
	TArray<ESoundName> arr = {
		ESoundName::vocal2, ESoundName::vocal3, ESoundName::vocal4, ESoundName::vocal5, ESoundName::vocal7
	};
	PlayRandomSound(arr, 1.0f);
}

void AAlice::ToggleCamera(bool _isMine, float _blendtime)
{
	//
	if (!AliceController) return;

	if (bOnEnemyView != _isMine) return;
	if (!_isMine) // !bOnEnemyView)
	{
		// 적 뷰로 전환
		if (CurrentTarget)
		{
			AliceController->SetViewTargetWithBlend(CurrentTarget, _blendtime, BlendFunc, BlendExp, /*bLockOutgoing=*/
			                                        true);
			bOnEnemyView = true;
		}
	}
	else
	{
		// 내 뷰로 복귀
		AliceController->SetViewTargetWithBlend(this, _blendtime, BlendFunc, BlendExp, /*bLockOutgoing=*/true);
		bOnEnemyView = false;
	}
}
