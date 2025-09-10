// Fill out your copyright notice in the Description page of Project Settings.


#include "Alice/AliceController.h"
#include "GameFramework/Pawn.h"
#include "UObject/ConstructorHelpers.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"
#include "EnhancedInputSubsystems.h"
#include "MathUtil.h"
#include "GameFramework/Character.h" // GetCharcter
#include "Kismet/GameplayStatics.h"
#include "Alice/Alice.h"

void AAliceController::BeginPlay()
{
	Super::BeginPlay();
	Alice=  Cast<AAlice>(UGameplayStatics::GetPlayerCharacter(GetWorld(),0));
}

void AAliceController::SetupInputComponent()
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
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AAliceController::OnMove);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AAliceController::OnLook);
		EnhancedInputComponent->BindAction(LMAction, ETriggerEvent::Started	, this, &AAliceController::OnAttack_Press);
		EnhancedInputComponent->BindAction(LMAction, ETriggerEvent::Completed, this, &AAliceController::OnAttack_Release);
		EnhancedInputComponent->BindAction(RMAction, ETriggerEvent::Started, this, &AAliceController::OnRM_Press);
		EnhancedInputComponent->BindAction(RMAction, ETriggerEvent::Completed, this, &AAliceController::OnRM_Release);
		EnhancedInputComponent->BindAction(ShiftAction, ETriggerEvent::Started, this, &AAliceController::OnShift_Press);
		EnhancedInputComponent->BindAction(ShiftAction, ETriggerEvent::Completed, this, &AAliceController::OnShift_Release);
		
		EnhancedInputComponent->BindAction(EAction, ETriggerEvent::Triggered, this, &AAliceController::OnE_Press);
		EnhancedInputComponent->BindAction(RAction, ETriggerEvent::Triggered, this, &AAliceController::OnR_Press);
	}
}

AAliceController::AAliceController()
{
	
}


void AAliceController::OnAttack_Press()
{
	if (!Alice)
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.0,FColor::Red, TEXT("Alice is Null"));
	}

	Alice->TryAttack();
}

void AAliceController::OnAttack_Release()
{
	
}

void AAliceController::OnRM_Press()
{
	if (!Alice)
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.0,FColor::Red, TEXT("Alice is Null"));
	}
	Alice->Rm_Press();
}

void AAliceController::OnRM_Release()
{
	Alice->RM_Release();
}

void AAliceController::OnShift_Press()
{
	if (bShiftPressed) return;
	bShiftPressed = true;
	if (!Alice)
{
	GEngine->AddOnScreenDebugMessage(-1, 1.0,FColor::Red, TEXT("Alice is Null"));
}
	
	Alice->Shift_Press();
}

void AAliceController::OnShift_Release()
{
	bShiftPressed = false;
	if (!Alice)
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.0,FColor::Red, TEXT("Alice is Null"));
	}
	Alice->Shift_Release();
}

void AAliceController::OnE_Press()
{
	Alice->TryParring();
}

void AAliceController::OnR_Press()
{
	Alice->R_Press();
}


void AAliceController::SetOnLookActive(bool _active)
{
	bIsOnLookActive = _active;
}

void AAliceController::OnMove(const FInputActionValue& Value)
{
	if (!bIsOnLookActive) return;
	if (Alice->IsActionState()) return;
	
	FVector2D MovementVector = Value.Get<FVector2D>();
	
	const FRotator Rotation = GetControlRotation();
	const FRotator YawRotation(0, Rotation.Yaw, 0);

	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);


	const FVector MoveDir = ForwardDirection * MovementVector.Y + RightDirection * MovementVector.X;
	GetCharacter()->AddMovementInput(MoveDir.GetSafeNormal(), 1.0f);
}

void AAliceController::OnLook(const FInputActionValue& Value)
{
	if (!bIsOnLookActive) return;
	FVector2D LookAxisVector = Value.Get<FVector2D>();
	float mul = 2.0f;
	float YawInput = LookAxisVector.X; // 좌 우 회전
	float PitchInput = LookAxisVector.Y; // 상 하 회전
	
	FRotator CurrentControlRotation = GetControlRotation();
	FRotator NewRot = CurrentControlRotation;

	float newPitch = CurrentControlRotation.Pitch + PitchInput;
	newPitch = FMath::Clamp(newPitch, upperMaxPitch, underMaxPitch);

	// Pitch 값에 따라서 SpringArm의 값 변경하기
	Alice->SetSpringArmLength(newPitch);
	
	//UE_LOG(LogTemp, Log, TEXT("yaw : %f"), YawInput);
	//UE_LOG(LogTemp, Log, TEXT("pitch : %f"), newPitch);
	NewRot.Pitch = newPitch;
	NewRot.Yaw += YawInput *mul;
	SetControlRotation(NewRot); // 서버에서 회전 적용시킴 - 클라이언트는 값이 변경되면 OnRep_Rotation이 호출될거
}
