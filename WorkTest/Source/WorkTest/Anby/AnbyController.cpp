// Fill out your copyright notice in the Description page of Project Settings.


#include "AnbyController.h"
#include "Anby.h"
#include "AnbyAnimInstance.h"

#include "GameFramework/Pawn.h"

#include "Engine/World.h"

#include "EnhancedInputComponent.h"
#include "InputActionValue.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"

#include "UObject/ConstructorHelpers.h"
#include "Debug/DebugDrawComponent.h"
#include "AnbyAnimInstance.h"
#include "MathUtil.h"
#include "PingSpawnComponent.h"
#include "Camera/CameraComponent.h"
#include "Net/UnrealNetwork.h"


AAnbyController::AAnbyController()
{
	bShowMouseCursor = false;
	DefaultMouseCursor = EMouseCursor::Default;
	bReplicates = true; // 어?
}


void AAnbyController::BeginPlay()
{
	Super::BeginPlay();

	if (Anby == nullptr)
	{
		Anby = Cast<AAnby>(GetPawn());
	}
}

void AAnbyController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	if (Anby == nullptr)
	{
		Anby = Cast<AAnby>(InPawn);
	}
	//Anby = Cast<AAnby>(InPawn);
}


void AAnbyController::SetGameInputMode(EGameInputMode _inputMode)
{
	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Cyan, TEXT("인풋모드 설정하기"));
	this->InputMode = _inputMode;
	if (InputMode == EGameInputMode::GameOnly)
	{
		FInputModeGameOnly gameInputMode;
		SetInputMode(gameInputMode);
		bShowMouseCursor = false;
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Cyan, TEXT("게임상태"));
		return;
	}
	else if (InputMode == EGameInputMode::UIOnly)
	{
		FInputModeUIOnly uiInputMode;
		//uiInputMode.SetWidgetToFocus(_widget); // widget 파라미터로 할당해주기
		uiInputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		SetInputMode(uiInputMode);
		bShowMouseCursor = true;
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Cyan, TEXT("UI상태"));
		return;
	}
	else if (InputMode == EGameInputMode::Mix)
	{
		FInputModeGameAndUI mixInputMode;

		return;
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("AnbyContorller -  InputMode is Default"));
	}
}

void AAnbyController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// Add Input Mapping Context
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(
		GetLocalPlayer()))
	{
		Subsystem->AddMappingContext(IMC_Player, 0);
	}

	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent))
	{
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AAnbyController::OnMove);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AAnbyController::OnLook);
		EnhancedInputComponent->BindAction(WheelAction, ETriggerEvent::Triggered, this, &AAnbyController::OnWheel);

		EnhancedInputComponent->BindAction(ParringAction, ETriggerEvent::Triggered, this, &AAnbyController::OnParring);
		EnhancedInputComponent->BindAction(ParringAction, ETriggerEvent::Completed, this,
		                                   &AAnbyController::OnEndParring);

		EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Started, this, &AAnbyController::OnAttack);
		EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Completed, this, &AAnbyController::OnAttackEnd);

		EnhancedInputComponent->BindAction(WheelClickAction, ETriggerEvent::Started, this, &AAnbyController::OnWheelPressed);
		EnhancedInputComponent->BindAction(WheelClickAction, ETriggerEvent::Completed, this, &AAnbyController::OnWheelReleased);

		EnhancedInputComponent->BindAction(WeaponAction, ETriggerEvent::Triggered, this,
		                                   &AAnbyController::OnChangeWeaponActive);

		EnhancedInputComponent->BindAction(GrappleShootAction, ETriggerEvent::Started, this,
		                                   &AAnbyController::OnRmPressed);
		EnhancedInputComponent->BindAction(GrappleShootAction, ETriggerEvent::Completed, this,
		                                   &AAnbyController::OnRmReleased);

		// Started : 키가 처음 눌렸을 때 한번
		// Triggered : 키를 누르고 있는동안 반복 호출
		// Completed : 입력이 끝났을 때(키 뗌 or Axis 0)
		// CAnceled : 입력이 강제로 취소 (키 땜)
		EnhancedInputComponent->BindAction(GrappleMoveAction, ETriggerEvent::Started, this,
		                                   &AAnbyController::OnShiftPressed);
		EnhancedInputComponent->BindAction(GrappleMoveAction, ETriggerEvent::Completed, this,
		                                   &AAnbyController::OnShiftReleased);

		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &AAnbyController::OnSpaceTrigger);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &AAnbyController::OnSpaceRelease);

		EnhancedInputComponent->BindAction(GrappleOffAction, ETriggerEvent::Triggered, this,
		                                   &AAnbyController::OnGrappleOff);
	}
}

void AAnbyController::OnMove(const FInputActionValue& Value)
{
	
	if (Anby->IsWorking()) return;
	if (Anby->IsAttacking()) return;

	FVector2D MovementVector = Value.Get<FVector2D>();

	const FRotator Rotation = GetControlRotation();
	const FRotator YawRotation(0, Rotation.Yaw, 0);

	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
	if (Anby->GetMovementState() != EMovementState::Moving || Anby->GetMovementState() != EMovementState::Falling)
	{
		GetCharacter()->AddMovementInput(ForwardDirection, MovementVector.Y);
		GetCharacter()->AddMovementInput(RightDirection, MovementVector.X);
	}
	DirectionRight = FMath::Clamp(MovementVector.X, -1.0f, 1.0f);
	DirectionFront = FMath::Clamp(MovementVector.Y, -1.0f, 1.0f);
}

void AAnbyController::OnLook(const FInputActionValue& Value)
{
	const ENetMode NetMode = GetNetMode();
	FVector2D LookAxisVector = Value.Get<FVector2D>(); // LookAxisVector를 로컬에서 계산
	// 리슨서버에서 로컬 플레이어라면'
	/*
	if (HasAuthority() && IsLocalController() && NetMode == NM_ListenServer)
	{
		// 리슨서버를 가진 로컬 플레이어 : 직접처리
		//GEngine->AddOnScreenDebugMessage(-1, 0.5f, FColor::Cyan, TEXT("직접처리"));
		ApplyLookRotation(LookAxisVector); // 즉각적인 반응주기 
		OnLook_Server(Value);
		return;
	}
	// 리슨서버를 가지지 않는 일반 플레이어
	if (!HasAuthority() && IsLocalController())
	{
		//	GEngine->AddOnScreenDebugMessage(-1, 0.5f, FColor::Cyan, TEXT("로컬 플레이어"));
		ApplyLookRotation(LookAxisVector); // 즉각적인 반응주기 
		OnLook_Server(Value); // 서버 RPC
		return;
	}*/
	ApplyLookRotation(LookAxisVector); // 즉각적인 반응주기 
	OnLook_Server(Value); // 서버 RPC
	//GEngine->AddOnScreenDebugMessage(-1, 0.5f, FColor::Cyan, TEXT("AnbyContorlller : OnLook : 서버 설정 없이 킴"));
}

void AAnbyController::OnLook_Server_Implementation(const FInputActionValue& Value)
{
	//FVector2D 변수화 시킴. 서버 공유를 위해서
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	// 서버에서는 직접 회전을 적용
	ApplyLookRotation(LookAxisVector); // 서버에서도 ApplyLookRotation 호출
	ReplicatedRotation = GetControlRotation(); // 서버의 최종 회전을 복제용 값으로 설정

	//GetCharacter()->AddControllerYawInput(LookAxisVector.X); 원래 이거로함 
	//GetCharacter()->AddControllerPitchInput(LookAxisVector.Y);
}

void AAnbyController::OnRep_Rotation()
{
	// 클라이언트에서는 OnRep_Rotation() 함수를 통해서 회전을 적용시킴
	// 클라이언트 예측을 사용했음(ApplyRotation) 서버에서 보낸 복제값이 현재 클라이언트값과 다른경우에 보정시킴
	if (!HasAuthority())
	{
		//if (GetControlRotation() != ReplicatedRotation)
		{
			//SetControlRotation(ReplicatedRotation);
		}
	}
}

void AAnbyController::OnWheel(const FInputActionValue& Value)
{
	float w = Value.Get<float>();
	WheelAmount += w * 2;

	WheelAmount = FMath::Clamp(WheelAmount, -40.0f, 40.0f);
	Anby->SetWheelAmount(WheelAmount);

	GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::White, FString::Printf(TEXT(" %f"), WheelAmount));
}

void AAnbyController::OnWheelPressed()
{
	Anby->HandlePingStart();
	
}

void AAnbyController::OnWheelReleased()
{
	Anby->HandlePingEnd();
}

void AAnbyController::ApplyLookRotation(FVector2D LookAxisVector)
{
	float YawInput = LookAxisVector.X; // 좌 우 회전
	float PitchInput = LookAxisVector.Y; // 상 하 회전
	
	if (Anby == nullptr)
	{
		return;
	}
	if (Anby->bIsWheelClicked)
	{
		Anby->GetPingComponent()->GetDirection(YawInput, PitchInput);
		return;
	}
	/*
	if (Anby->PingPreviewCheck())
	{
		GEngine->AddOnScreenDebugMessage(5, 0.02f, FColor::White, TEXT("PingPreviewCheck = true"));
		return;
	}*/
	
	FRotator CurrentControlRotation = GetControlRotation();
	FRotator NewRot = CurrentControlRotation;
	if (Anby->ViewType == EViewType::ThirdSight)
	{
		//NewRot.Yaw += YawInput;

		float newPitch = CurrentControlRotation.Pitch += PitchInput;
		newPitch = FMath::Clamp(newPitch, upperMaxPitch, underMaxPitch);
		NewRot.Pitch = newPitch;
	}
	else if (Anby->ViewType == EViewType::FirstSight)
	{
		//NewRot.Yaw -= YawInput;
		NewRot.Pitch -= PitchInput; // 상 하 전환이 반대임
	}
	NewRot.Yaw += YawInput;
	SetControlRotation(NewRot); // 서버에서 회전 적용시킴 - 클라이언트는 값이 변경되면 OnRep_Rotation이 호출될거
}

void AAnbyController::OnParring()
{
	if (Anby == nullptr) return;
	if (Anby->IsWorking()) return;
	if (Anby->IsWeaponChanging()) return;
	//UE_LOG(LogTemp, Display, TEXT("Try Parring"));
	//Anby->TryParring();
	//GEngine->AddOnScreenDebugMessage(-1, 0.5f, FColor::Yellow, TEXT("StartParring"));
}

void AAnbyController::OnEndParring()
{
	if (Anby == nullptr) return;
	//GEngine->AddOnScreenDebugMessage(-1, 0.5f, FColor::Green, TEXT("CancelParring"));
	//Anby->EndParring();
}

void AAnbyController::OnAttack()
{
	if (Anby == nullptr) return;
	if (Anby->IsWorking()) return;
	//Anby->TryAttack();
	Anby->HandleAttackInput(EAttackInput::Chain);
}

void AAnbyController::OnAttackEnd()
{
	if (Anby == nullptr) return;
	if (Anby->IsWorking()) return;
	Anby->HandleLeftMouseEnd();
}


void AAnbyController::OnRMaction()
{
	if (Anby == nullptr) return;
	Anby->ResetTarget();
}

void AAnbyController::OnSpaceTrigger()
{
	if (Anby==nullptr) return;
	Anby->HandleSpaceStart();
}

void AAnbyController::OnSpaceRelease()
{
	if (Anby==nullptr) return;
	Anby->HandleSpaceEnd();
}

void AAnbyController::OnGrappleOff()
{
	Anby->HandleGrappleOff();
}

void AAnbyController::OnShiftPressed()
{
	Anby->HandleGrappleStart();
}

void AAnbyController::OnShiftReleased()
{
	Anby->HandleGrappleCancel();
}

void AAnbyController::OnRmPressed()
{
	Anby->HandleGrapplePreviewStart();
}

void AAnbyController::OnRmReleased()
{
	Anby->HandleGrappleShoot();
}

void AAnbyController::OnChangeWeaponActive()
{
	Anby->ResetAnimState();
	if (Anby->GetWeaponState()) // 활성화 상태
	{
		// 비활성화 시키기
		GEngine->AddOnScreenDebugMessage(-1, 0.5f, FColor::Green, TEXT("Disable"));
		Anby->DisActiveWeapon();
	}
	else // 비활성화 상태
	{
		// 활성화 시키기
		GEngine->AddOnScreenDebugMessage(-1, 0.5f, FColor::Green, TEXT("Enable"));
		Anby->ActiveWeapon();
	}
}


void AAnbyController::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AAnbyController, ReplicatedRotation);
}
