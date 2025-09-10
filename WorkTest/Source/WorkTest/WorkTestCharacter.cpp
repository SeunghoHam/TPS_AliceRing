// Copyright Epic Games, Inc. All Rights Reserved.

#include "WorkTestCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"

#include "DrawDebugHelpers.h"

// Replicated ( DOREPLIFETIME)
#include "Net/UnrealNetwork.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

//////////////////////////////////////////////////////////////////////////
// AWorkTestCharacter


FString GetEnumText(ENetRole Role)
{
	switch (Role)	
	{
	case ROLE_None:
		return "None";
		break;
	case ROLE_SimulatedProxy:
		return "Simiulated";

		break;
	case ROLE_AutonomousProxy:
		return "Autonomous";

		break;
	case ROLE_Authority:
		return "Authority";

		break;
	default:
		return "Default";

		break;
	}
}
void AWorkTestCharacter::Tick(float Deltatime)
{
	Super::Tick(Deltatime);
	
	if (HasAuthority())
	{
		// 인스턴스가 자율 생성된 Autonomous Proxy
		ReplicatedTransform = GetActorLocation();
		//ServerState.Transform = GetActorTransform(); // 구조체 만들어서 관리하면 이런느낌
		//ServerState.LastMoves = 
	}
	/*
	else
	{
		// 인스턴스가 다른  Simulated Proxy
		//SetActorLocation(ReplicatedLocation); // 서버에서 받은 값으로 변경해줌
	}*/

	DrawDebugString(GetWorld(), FVector(0, 0, 100), GetEnumText(GetLocalRole()), this, FColor::White, Deltatime);
}

AWorkTestCharacter::AWorkTestCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
}

void AWorkTestCharacter::QSkill()
{
	UE_LOG(LogTemp, Display, TEXT("Q_Skill"));
}

void AWorkTestCharacter::ESkill()
{
	UE_LOG(LogTemp, Display, TEXT("E_Skill"));
}

void AWorkTestCharacter::TestJump()
{
	// 본인 상태 업데이트
	Server_Jump(); // 상태가 점검된 값으로 서버로 전송
	ReplicatedTransform += FVector::ForwardVector;
}

void AWorkTestCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();


	// Test NetUpdateFrequency
	if (HasAuthority())
	{
		NetUpdateFrequency = 1; // 네트워크 업데이트 스트레스 주기
	}
}

void AWorkTestCharacter::OnRep_ReplicatedTransform()
{
	// 복제 알림은 클라이언트에게만 전달됨
	// SetActorTransform(ReplicatedTransform) // Tick에서 HasAuthority가 아닌경우에 하던 이동방식을 이쪽으로
	UE_LOG(LogTemp, Warning, TEXT("Replicated Locaiton"));
}

// Paremeter Name Change NO!!
void AWorkTestCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(AWorkTestCharacter, ReplicatedTransform);
	DOREPLIFETIME(AWorkTestCharacter, CustomVelocity);
}

void AWorkTestCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{

	// Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
	
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AWorkTestCharacter::Move);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AWorkTestCharacter::Look);

		EnhancedInputComponent->BindAction(QskillAction, ETriggerEvent::Started, this, &AWorkTestCharacter::TestJump);
		EnhancedInputComponent->BindAction(EskillAction, ETriggerEvent::Started, this, &AWorkTestCharacter::ESkill);
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void AWorkTestCharacter::Server_Jump_Implementation()
{
	UE_LOG(LogTemp, Display, TEXT("ServerJump"));
}

void AWorkTestCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	
		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void AWorkTestCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}