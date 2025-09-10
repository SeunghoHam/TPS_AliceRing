// Fill out your copyright notice in the Description page of Project Settings.


#include "Anby.h"

#include <MaterialExport.h>

#include "UObject/ConstructorHelpers.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "AnbyAnimInstance.h"
#include "AnbyWeapon.h"
#include "BaseEnemy.h"
#include "Engine/SkeletalMeshSocket.h"
// ----- Componnets -----
#include "CombatComponent.h"
#include "GrappleMoveComponent.h"
#include "CamSettingComponent.h"

#include "PingActor.h"
#include "AnbyController.h"

#include "CableComponent.h"
#include "MovementReplicator.h"
#include "PingSpawnComponent.h"
#include "Animation/AnimSubsystem_NodeRelevancy.h"
#include "Blueprint/UserWidget.h"
#include "Net/UnrealNetwork.h"
#include "ProfilingDebugging/BootProfiling.h"
#include "Widget/PlayerWidget.h"
//#include "MathUtil.h"




// Sets default values
AAnby::AAnby()
{
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
	bUseControllerRotationYaw = false;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	GetCharacterMovement()->bOrientRotationToMovement = true; // Rotate character to moving direction
	GetCharacterMovement()->RotationRate = FRotator(0.f, 500.f, 0.f); //500


	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArmComponent->SetupAttachment(RootComponent);
	SpringArmComponent->TargetArmLength = 300.f;
	SpringArmComponent->bDoCollisionTest = false; // Don't want to pull camera in when it collides with level
	SpringArmComponent->bUsePawnControlRotation = true;


	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	CameraComponent->SetupAttachment(SpringArmComponent, USpringArmComponent::SocketName);
	CameraComponent->bUsePawnControlRotation = false;

	CombatSystem = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatSystem"));
	GrappleSystem = CreateDefaultSubobject<UGrappleMoveComponent>(TEXT("GrappleSystem"));
	Replicator = CreateDefaultSubobject<UMovementReplicator>(TEXT("MovementReplicator"));
	CameraSystem = CreateDefaultSubobject<UCamSettingComponent>(TEXT("CameraSystem"));
	PingSystem=  CreateDefaultSubobject<UPingSpawnComponent>(TEXT("PingSystem"));
	
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true; // 공유되게 하기
	InitGrapple();
}

void AAnby::BeginPlay()
{
	Super::BeginPlay();
	
	AnimInstance = Cast<UAnbyAnimInstance>(GetMesh()->GetAnimInstance());
	
	if (AnimInstance)
	{
		AnimInstance->SetOwner(this);
	}


	//AnbyController = Cast<AAnbyController>(GetController());
	
	// GetController는 로컬환경
	if (HasAuthority()) // PlayerSTate나 NetConnection를 기준으로 할 수도 있음
	{
		AnbyController = Cast<AAnbyController>(UGameplayStatics::GetPlayerController(GetWorld(),0 ));
	}
	else
	{
		AnbyController = Cast<AAnbyController>(GetWorld()->GetFirstPlayerController());
	}
	
	
	FString texts = "Instance is " + GetMesh()->GetName();
	if (AnimInstance == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("The AnimInstance is null"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("The AnimInstance"));
	}

	AttachWeapon(WeaponTemplate);
	ChangeAnbyState(EAnbyState::Normal);
	ViewType = EViewType::ThirdSight;
	if (AnbyController)
	{
		InitControllerValue();
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, TEXT("[Anby] : PlayerController = null"));
	}

	GrappleCable->SetVisibility(false);

	if (PlayerWidgetClass)
	{
		PlayerWidgetInstance = CreateWidget<UPlayerWidget>(GetWorld(), PlayerWidgetClass);
		if (PlayerWidgetInstance)
		{
			PlayerWidgetInstance->AddToViewport();
		}
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, TEXT("[Anby] : PlayerWidgetClass = null"));
	}
}

// Called every frame
void AAnby::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//if (ViewType == EViewType::)
	ControlSpringArmLength(DeltaTime);
}

// Called to bind functionality to input
void AAnby::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void AAnby::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AAnby, GrappleCable);
}
void AAnby::OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PrevMovementMode, PreviousCustomMode);

	if (GrappleSystem && GrappleSystem->GetIsGrappling())
	{
		if (GetCharacterMovement()->MovementMode != MOVE_Flying)
		{
			GetCharacterMovement()->SetMovementMode(MOVE_Flying);
		}
	}
}
void AAnby::ServerMoveCharacter_Implementation(const FVector& TargetLocation)
{
	SetActorLocation(TargetLocation);
}

void AAnby::PingPreviewStart()
{
	if (PingSystem == nullptr) return;
	AnbyState = EAnbyState::Ping;
	// Viewtype변경도 필요함
	PingSystem->PreviewStart();
}

void AAnby::PingPreviewEnd()
{
	if (PingSystem == nullptr) return;
	//if (AnbyState != EAnbyState::Ping) return;
	//if (ViewType != EViewType::FirstSight) return;
	
	//AnbyState = EAnbyState::Normal;
	
	PingSystem->PreviewEnd();
	//PingComponent->ServerSendPing();
}

void AAnby::ResetAnimState()
{
	ChangeAnbyState(EAnbyState::Normal);
	AnimInstance->ChangeAnbyAnimState(EAnbyAnimState::Locomotion);
	//bCanComboAttackInput = true;
	ComboCount = 0;
	GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Yellow, TEXT("SetIdle"));

	//if (InputQueue.IsEmpty()) return;
	GetWorldTimerManager().SetTimer(
		ProcessInputTimerHandle,
		this,
		&AAnby::ProcessBufferedInput,
		0.2f, // 시간 유동적으로 변경
		false
	);
}

AAnbyWeapon* AAnby::GetAnbyWeapon()
{
	if (AnbyWeapon != nullptr)
		return AnbyWeapon;
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Weapon is Null"));
		return nullptr;
	}
}

void AAnby::HandlePingStart()
{
	if (bIsWheelClicked)
		return;
	bIsWheelClicked= true;
	PingPreviewStart();
}

void AAnby::HandlePingEnd()
{
	bIsWheelClicked = false;
	PingPreviewEnd();
}

void AAnby::HandleGrappleStart()
{
	if (GrappleSystem)
	{
		if (bIsPressShift) return;

		bIsPressShift = true;
		GrappleSystem->StartGrapple();
	}
}

void AAnby::HandleGrappleCancel()
{
	if (GrappleSystem)
	{
		bIsPressShift = false;
		GrappleSystem->CancelGrapple();
	}
}

void AAnby::HandleGrapplePreviewStart()
{
	if (GrappleSystem)
	{
		if (bIsPressRM) return; // Input
		bIsPressRM = true;
		GrappleSystem->StartPreview();
	}
}

void AAnby::HandleGrappleShoot()
{
	if (GrappleSystem)
	{
		bIsPressRM = false;
		GrappleSystem->StopPreview();
		GrappleSystem->FireGrapple();
	}
}

void AAnby::HandleGrappleOff()
{
	if (GrappleSystem)
	{
		GrappleSystem->DestroyGrappleActor();
	}
}

void AAnby::HandleSpaceStart()
{
	if (bIsSpacePressed) return;
	bIsSpacePressed = true;
	LaunchCharacter(GetActorUpVector() * 100.0f,false,false);
	//GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Yellow, TEXT("HandleSpaceStart"));
}

void AAnby::HandleSpaceEnd()
{
	bIsSpacePressed= false;
}

void AAnby::ControlSpringArmLength(float Deltatime)
{
	float pitch = CameraComponent->GetComponentRotation().Pitch;
	//FRotator CurrentRotataion = CameraComponent->GetComponentRotation();
	 //TargetLength;
	if (pitch >= 0 )
	{
		amount = 12.0f;
	}
	else amount = 3.0f;
	
	float TargetLength  = BaseLength - FMath::Abs(pitch * amount);
	
	TargetLength = FMath::Clamp(TargetLength, MinLength,MaxLength);
	float newLength  = FMath::FInterpTo(SpringArmComponent->TargetArmLength,TargetLength,Deltatime , 5.0f);
	//UE_LOG(LogTemp, Log, TEXT("%f"), newLength);
	SpringArmComponent->TargetArmLength = newLength;
}

void AAnby::SetWheelAmount(float value)
{
	//float value = WheelAmount;
	//FMathf::Clamp(value, minWheelAmount, maxWheelAmount);
	if (ViewType != EViewType::ThirdSight) return; // 다른시점이면 동작 안하도록
	
	WheelAmount = value;
	BaseLength+= WheelAmount;
}

void AAnby::TryParring()
{
	if (AnimInstance == nullptr)
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::Red, TEXT("AnimInstance = null"));
	}
	else
	{
		AnimInstance->ChangeAnbyAnimState(EAnbyAnimState::Parring);
		ChangeAnbyState(EAnbyState::Working);
		BP_Parring(); // 패링 연출효과
	}
}

void AAnby::EndParring()
{
	AnimInstance->ChangeAnbyAnimState(EAnbyAnimState::Locomotion);
	ChangeAnbyState(EAnbyState::Normal);
}

void AAnby::TryAttack()
{
	bCanComboAttackInput = false;
	// 예약되거나 실행되거나 할겨
	ClearInputQueue();
	//GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Red, TEXT("TryChain"));
	ComboCount++;
	switch (ComboCount)
	{
	case 0:
		UE_LOG(LogTemp, Error, TEXT("Chain ComboCount Error"));
		break;
	case 1:
		ChangeAnbyState(EAnbyState::ChainAttacking);
		AnimInstance->ChangeAnbyAnimState(EAnbyAnimState::Chain1);
		SetTarget();
		break;
	case 2:
		AnimInstance->ChangeAnbyAnimState(EAnbyAnimState::Chain2);
		break;
	case 3:
		AnimInstance->ChangeAnbyAnimState(EAnbyAnimState::Chain3);
		break;
	case 4:
		AnimInstance->ChangeAnbyAnimState(EAnbyAnimState::Chain4);
		break;
	default:
		UE_LOG(LogTemp, Error, TEXT("Chain ComboCount Error"));
		break;
	}
}

void AAnby::SetTarget()
{
	if (CombatSystem == nullptr)
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Red, TEXT("CombatComponent =null"));
		return;
	}
	if (CombatSystem != nullptr)
	{
		//GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Red, TEXT("Combatsystem::FindEnemy"));
		MyTarget = CombatSystem->FindFrontEnemy(800.f);
		if (MyTarget)
		{
			AnimInstance->SetLookAtTarget(MyTarget);
		}
		//CombatSystem->ServerAttack(10);
	}
}

void AAnby::ResetTarget()
{
	GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Yellow, TEXT("ResetTarget"));
	if (MyTarget == nullptr) return;
	BP_ResetTarget();
	MyTarget = nullptr;
}

void AAnby::RightClickAction()
{
}

void AAnby::CheckCombo()
{
	// animnotify 애니메이션 종료시점에서 실행
	ProcessBufferedInput();
}

void AAnby::LookAtEnemy()
{
	if (!MyTarget)
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Red, TEXT("Target = null"));
		return;
	}

	FVector CameraLocation = CameraComponent->GetComponentLocation();
	FVector TargetLocation = MyTarget->GetActorLocation();
	FVector LookDir = (TargetLocation - CameraLocation).GetSafeNormal();
	FRotator LookAtRot = LookDir.Rotation();

	//GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Green, TEXT("LookAtSetting"));
	SpringArmComponent->SetWorldRotation(LookAtRot);
}

void AAnby::ActiveWeapon()
{
	if (IsWorking()) return;
	if (IsAttacking()) return;
	GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::White, TEXT("Active Weapon"));
	//AnimInstance->ChangeAnbyAnimState(EAnbyAnimState::EnableWeapon);
	ChangeAnbyState(EAnbyState::WeaponStateChange);
	AnbyWeapon->WeaponEnable();
	bIsWeaponActive = true;
	BP_ActiveWeapon();
}

void AAnby::DisActiveWeapon()
{
	if (IsWorking()) return;
	if (IsAttacking()) return;
	GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::White, TEXT("DisActive Weapon"));
	//AnimInstance->ChangeAnbyAnimState(EAnbyAnimState::DisableWeapon);
	ChangeAnbyState(EAnbyState::WeaponStateChange);
	AnbyWeapon->WeaponDisable();
	bIsWeaponActive = false;
	BP_DisActiveWeapon();
}

void AAnby::HandleAttackInput(const EAttackInput& inputType)
{
	if (IsWorking()) return;
	if (IsWeaponChanging()) return;

	if (ViewType == EViewType::ThirdSight)
	{
		if (bCanComboAttackInput)
		{
			bCanComboAttackInput = false;
			// 바로 실행시킴
			PerformAttack(); // 
		}
		else
		{
			// 예약
			//GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::White, TEXT("[예약추가]"));
			InputQueue.Enqueue(inputType);
		}
	}
}
void AAnby::HandleLeftMouseEnd()
{

}


FVector AAnby::GetCameraForwardVector()
{
	return CameraComponent->GetForwardVector();
}

UAnbyAnimInstance* AAnby::GetAnbyAnimInstance()
{
	if (AnimInstance == nullptr)
		return nullptr;
	return AnimInstance;
}

void AAnby::SetMovementState(const EMovementState& value)
{
	MovementState = value;
}

void AAnby::SetTargetMinArmLength()
{
	if (ViewType == EViewType::ThirdSight)
	{
		// 1인칭으로
		MinLength = 0;
		AnbyWeapon->SetWeaponVisiblity(false);
	}
	else if (ViewType  == EViewType::FirstSight)
	{
		// 3인칭으로
		MinLength= 300;
		AnbyWeapon->SetWeaponVisiblity(true);
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Red, TEXT("[Anby::SetTargetMinArmLength] - 잘못된 ViewType입력"));

	}
}

void AAnby::ChangeViewType(EViewType _type)
{
	ViewType = _type;
}

void AAnby::PerformAttack()
{
	// 바로 실행되는 공격
	//GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::White, TEXT("[PerformAttack]"));
	TryAttack();
}

void AAnby::ProcessBufferedInput()
{
	if (InputQueue.IsEmpty())
	{
		// 입력 가능상태 변환
		bCanComboAttackInput = true;
		return;
	}

	//GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Red, TEXT("ProcessBufferInput"));
	EAttackInput Input;
	if (InputQueue.Dequeue(Input))
	{
		switch (Input)
		{
		case EAttackInput::Chain:
			PerformAttack();
			break;
		case EAttackInput::Grapple:
			GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Red, TEXT("Grapple 미구현"));
			break;
		}
	}
	// 입력 처리 후 비우기
	ClearInputQueue();
}

void AAnby::ClearInputQueue()
{
	// 입력 큐 전체초기화
	//GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Red, TEXT("ClearInputQueue"));
	while (InputQueue.Dequeue(AttackInput))
	{
	}
}

void AAnby::InitControllerValue()
{
	upperMaxPitch = AnbyController->upperMaxPitch;
	underMaxPitch = AnbyController->underMaxPitch;
	amount = AnbyController->amount;
	minWheelAmount = AnbyController->minWheelAmount;
	maxWheelAmount = AnbyController->maxWheelAmount;

	BaseLength = AnbyController->BaseLength;
	MinLength = AnbyController->MinLength;
	MaxLength = AnbyController->MaxLength;
}

void AAnby::AttachWeapon(TSubclassOf<AAnbyWeapon> Weapon)
{
	if (Weapon) // WeaponTemplate is valid?
	{
		//UE_LOG(LogTemp, Log, TEXT("Try Attach1"));
		AActor* SpawnWeapon = GetWorld()->SpawnActor<AAnbyWeapon>(Weapon);

		AnbyWeapon = Cast<AAnbyWeapon>(SpawnWeapon);

		const USkeletalMeshSocket* WeaponSocket = GetMesh()->GetSocketByName("WeaponPoint");

		if (WeaponSocket && SpawnWeapon)
		{
			UE_LOG(LogTemp, Log, TEXT("Try Attach2"));

			WeaponSocket->AttachActor(SpawnWeapon, GetMesh());
		}
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Red, TEXT("[Anby] WeaponTemplate Null!!!"));
		UE_LOG(LogTemp, Warning, TEXT("[Anby] WeaponTemplate Null!!!"));
	}
}


void AAnby::ChangeAnbyState(const EAnbyState& state)
{
	AnbyState = state;
}

EMovementState AAnby::GetMovementState()
{
	return MovementState;
}

bool AAnby::GetWeaponState()
{
	return bIsWeaponActive;
}

void AAnby::CallCurrentState()
{
	if (AnbyState == EAnbyState::Normal) UE_LOG(LogTemp, Log, TEXT("Curent State : Normal"));
	if (AnbyState == EAnbyState::Working) UE_LOG(LogTemp, Log, TEXT("Curent State : Working"));
}

void AAnby::AttackLaunchCharacter()
{
	FVector Direction;
	if (MyTarget)
	{
		Direction = (MyTarget->GetActorLocation() - GetActorLocation()).GetSafeNormal() * MoveAmount;
		// TODO : 회전추가
		FQuat quat = Direction.ToOrientationQuat();
		SetActorRotation(quat);
	}
	else
	{
		Direction = GetActorForwardVector() * MoveAmount;
	}
	LaunchCharacter(Direction, false, false);
}

void AAnby::ServerSendPing_Implementation(const FVector& Location)
{
	/*
	ClientShowPing(Location); // 서버에는 직접 생성
	
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		if (AAnbyController* PC =Cast<AAnbyController>(*It))
		{
			//if (PC !=  getcongro)
			{
				// 만약 본인(ServerPing을 호출한 컨트롤러가 아닐때만)
			//	PC->ClientShowPing(Location);	
			}
			
		}
	}*/
	MulticastSpawnPing(Location);
}

void AAnby::ClientShowPing_Implementation(const FVector& Location)
{
	// Ping Actor 및 widget 생성
	// 일정 시간 후 Destory 까지
	GetWorld()->SpawnActor<APingActor>(TemplatePingActor, Location, FRotator::ZeroRotator);
}

void AAnby::MulticastSpawnPing_Implementation(const FVector& Location)
{
	GetWorld()->SpawnActor<APingActor>(TemplatePingActor, Location, FRotator::ZeroRotator);
}

void AAnby::InitGrapple()
{
	GrappleCable = CreateDefaultSubobject<UCableComponent>(TEXT("GrappleComponent"));
	GrappleCable->SetupAttachment(GetMesh(), "LeftHandSocket");
	GrappleCable->CableLength = 0.0f;
	GrappleCable->NumSegments = 1;
	GrappleCable->CableWidth = 2;
	GrappleCable->bEnableCollision = false;
	GrappleCable->bAttachEnd = false;
}

void AAnby::GrappleCancelLaunch(const FVector& _direction, float _amount)
{
	FVector Shoot = _direction * _amount;
	LaunchCharacter(Shoot,false,false);
}

void AAnby::HideGrapple_Implementation()
{
	GrappleCable->SetVisibility(false);
}

void AAnby::ShowGrapple_Implementation()
{
	GrappleCable->SetVisibility(true);
}

void AAnby::CableAttachToMovePoint_Implementation(AActor* actor)
{
	GrappleCable->SetVisibility(true);
	GrappleCable->bAttachEnd = true;
	FName ComponentProperty = "grapplinghook_pTorus2"; // 훅 엑터에 있는 Mesh의 이름임
	FName SocketName = "GrappleSocket";
	GrappleCable->SetAttachEndTo(GrappleSystem->GetGrappleActor(), ComponentProperty,SocketName);
}
