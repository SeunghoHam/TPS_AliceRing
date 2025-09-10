// Fill out your copyright notice in the Description page of Project Settings.


#include "Anby/AnbyAnimInstance.h"
#include "AnbyController.h"
#include "Anby.h"
#include "Kismet/GameplayStatics.h"
#include "Math/RotationMatrix.h"
#include "Math/Matrix.h"

void UAnbyAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	
	//Controller  = Cast<AAnbyController>(UGameplayStatics::GetPlayerController(GetWorld(),0));
	/*
	Owner = Cast<ACharacter>(GetOwningActor());
	if (Owner)
	{
		MyMesh =  Owner->GetMesh();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[AnbyAnimInstance] owner = null"));
	}*/
}

void UAnbyAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
	//캐릭터가 적 바라보게 하기
	
	if (LookAtTarget == nullptr) return;
	//LookTarget();
	LookTarget();
}

void UAnbyAnimInstance::LookTarget()
{

	//.if (!TryGetPawnOwner()) return;
	
	//APawn* OwnerPawn = TryGetPawnOwner();
	
	//USkeletalMeshComponent* MeshComp = GetSkelMeshComponent();
	//if (!MeshComp) return;

	// 1. 대상 위치 계산
	FVector ActorLocation = Owner->GetActorLocation();
	FVector TargetLocation = LookAtTarget->GetActorLocation();
	FVector ToTargetWorld = (TargetLocation - ActorLocation).GetSafeNormal();

	// 2. 월드 → 로컬 변환 (컴포넌트 공간 기준)
	FTransform ComponentToWorld = MyMesh->GetComponentTransform();
	FVector ToTargetLocal = ComponentToWorld.InverseTransformVectorNoScale(ToTargetWorld);

	// 3. 방향 기준 회전 행렬 만들기
	FRotationMatrix LookAtMatrix = FRotationMatrix::MakeFromX(ToTargetLocal).Rotator();
	FRotator LookAtRotator = LookAtMatrix.Rotator(); // 변수화시킴

	// 4. 예: 정면 기준이 다르다면 보정 (필요 시 조정)
	// 기본 = 왼쪽으로 90도
	LookAtRotator.Yaw -= 90.f;
	LookAtRotator.Roll += 90.f;
	
	//LookAtRotator.Pitch += 90.f;
	//DrawDebugLine(GetWorld(), ActorLocation, TargetLocation,FColor::White,false, 0.5f);
	//DrawDebugCoordinateSystem(GetWorld(), ActorLocation + FVector::UpVector * 100, LookAtRotator, 20.f, false, 0.5f, 0, 2.0f);
	
	GEngine->AddOnScreenDebugMessage(1,0.1f, FColor::Green, TEXT("[AnbyAnim] LookAt"));
	// 5. 본에 적용할 회전 저장 (Component Space)
	SpineRotation = FTransform(LookAtRotator); // 이 값은 AnimGraph에서 ModifyBone에 연결
	if (bUseLookAt == false) bUseLookAt = true;
	// 디버그 로그
	// UE_LOG(LogTemp, Warning, TEXT("LookAtRotator: %s"), *LookAtRotator.ToString());
}

void UAnbyAnimInstance::SetLookAtTarget(AActor* _target)
{
	LookAtTarget = _target;
}

void UAnbyAnimInstance::SetLocomotion(const EAnbyAnimState& state)
{
	AnimState= EAnbyAnimState::Locomotion;
}

void UAnbyAnimInstance::ChangeAnbyAnimState(const EAnbyAnimState& state)
{
	this->AnimState = state;
	//UE_LOG(LogHAL, Log, TEXT("ChangeAnimState"));
}

void UAnbyAnimInstance::SetOwner(ACharacter* _owner)
{
	Owner = _owner;
	MyMesh = _owner->GetMesh();
	UE_LOG(LogTemp,Warning,TEXT("AnimInstnace SetOwner"));

	UE_LOG(LogTemp,Warning,TEXT("MyMesh : %s"), *MyMesh->GetName());
	
}
