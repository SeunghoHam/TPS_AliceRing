// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/GrappleMoveComponent.h"


#include "TextureAssetActions.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "MovementReplicator.h"
#include "PlayerWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h" // GetLifeTime~
#include "ProfilingDebugging/BootProfiling.h"


// Sets default values for this component's properties
UGrappleMoveComponent::UGrappleMoveComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicated(true);
	//bReplicates = true;
	// ...
}


// Called when the game starts
void UGrappleMoveComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	Owner = Cast<AAnby>(GetOwner());
	PC = Cast<APlayerController>(Owner->GetController());
	Replicator = GetOwner()->FindComponentByClass<UMovementReplicator>();
	
	//StartPoint = Owner->GetMesh()->GetSocketLocation("as"); // 시작 위치 확인
	EMovementMode mode  = Owner->GetCharacterMovement()->MovementMode;
	FString ModeName;
	switch (mode)
	{
	case MOVE_Walking:        ModeName = TEXT("Walking"); break;
	case MOVE_NavWalking:     ModeName = TEXT("NavWalking"); break;
	case MOVE_Falling:        ModeName = TEXT("Falling"); break;
	case MOVE_Swimming:       ModeName = TEXT("Swimming"); break;
	case MOVE_Flying:         ModeName = TEXT("Flying"); break;
	case MOVE_Custom:         ModeName = TEXT("Custom"); break;
	case MOVE_None:           ModeName = TEXT("None"); break;
	default:                  ModeName = TEXT("Unknown"); break;
	}
	
	GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green,
		FString::Printf(TEXT("MovementMode: %s"), *ModeName));
}


// Called every frame
void UGrappleMoveComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                          FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);


	if (PC== nullptr) return;
	if (!PC->IsLocalController()) return;
	if (!GetOwner()->HasAuthority())
	{
		TickDebug();	
	}
	
	if (bIsHooked && bIsGrappling)
	{
		if (bCancelGrapple)
		{
			SetIsGrappling(false);
			return;
		}
		//ApplyGrappleMovementMode();
		if (!IsValid(Owner)) return;
	
		GEngine->AddOnScreenDebugMessage(1, 0.2f, FColor::White,TEXT("Moving"));
		LastMove = CreateMove(DeltaTime); // CreateMove에서 방향처리 다 함
		
		// AutonomouseProxy인 경우 서버로 Move전송
		if (Owner->GetLocalRole() == ROLE_AutonomousProxy)
		{
			SimulateMove(LastMove); // 로컬에서 이동
			Replicator->SendGrappleMove(LastMove); // 서버로 Move 전송
		}
		else
		{
			// 서버나 리플리케이터 없다면 직접 처리
			SimulateMove(LastMove);
		}
	}
}


void UGrappleMoveComponent::CheckGrappleHitPoint()
{
	if (bIsHooked) return;
	int32 ViewportX = 0, ViewportY = 0;
	PC->GetViewportSize(ViewportX, ViewportY);
	FVector WorldLocation, WorldDirection;
	if (PC->DeprojectScreenPositionToWorld(ViewportX / 2, ViewportY / 2, WorldLocation, WorldDirection))
	{
		FVector Start = WorldLocation;
		FVector End = Start + (WorldDirection * MaxGrappleDistance);

		FHitResult Hit;
		FCollisionQueryParams Params;
		Params.AddIgnoredActor(Owner); // 자기 자신은 제외시키기

		bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECollisionChannel::ECC_Visibility, Params);
		if (bHit)
		{
			DrawDebugSphere(GetWorld(), Hit.ImpactPoint, 16.0f, 16, FColor::Green, false, inRate);
			DrawDebugLine(GetWorld(), Start, Hit.ImpactPoint, FColor::Green, false, inRate);
		}
		else
		{
			DrawDebugLine(GetWorld(), Start, End, FColor::White, false, inRate);
		}
	}
}


void UGrappleMoveComponent::ServerFireGrapple_Implementation(const FVector& Start, const FVector& End)
{
}

void UGrappleMoveComponent::FireGrapple()
{
	if (!Owner || !PC) return;
	if (bIsHooked) return;
	//if (!Owner || !Owner->IsLocallyControlled()) return;
	//FVector Start = Owner->GetActorLocation() + Owner->GetActorForwardVector() * 100.f;
	//FVector End = Start+Owner->GetActorForwardVector() * 3000.0f;

	//ServerFireGrapple(Start, End);

	// Deproject 화면 중앙 -> 월드방향
	int32 ViewportX = 0, ViewportY = 0;
	PC->GetViewportSize(ViewportX, ViewportY);
	FVector WorldLocation, WorldDirection;
	if (PC->DeprojectScreenPositionToWorld(ViewportX / 2, ViewportY / 2, WorldLocation, WorldDirection))
	{
		FVector Start = WorldLocation;
		FVector End = Start + (WorldDirection * MaxGrappleDistance);

		FHitResult Hit;
		FCollisionQueryParams Params;
		Params.AddIgnoredActor(Owner); // 자기 자신은 제외시키기

		bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECollisionChannel::ECC_Visibility, Params);
		if (bHit)
		{
			//if (Owner == nullptr) return;
			if (!Owner || !Owner->IsLocallyControlled()) return;
			//FVector NormalDirection = ( Owner->GetActorLocation() -  Hit.ImpactPoint).GetSafeNormal();

			SetTargetLocation(Hit.Location + (Hit.ImpactNormal * 30));
			// GrappleTargetLocation = Hit.Location + (Hit.ImpactNormal * 30);

			GrappleCreatePoint = Hit.Location + (Hit.ImpactNormal * 10);
			GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::White, TEXT("Grapple Hit"));
			DrawDebugSphere(GetWorld(), GrappleTargetLocation, 64.0f, 16, FColor::Red, false, 2.0f);
			DrawDebugSphere(GetWorld(), GrappleCreatePoint, 32.0f, 16, FColor::Blue, false, 3.0f);

			
			//if (bIsHooked) return;
			if (GetOwner()->HasAuthority())
			{
				SpawnGrappleActor(GrappleCreatePoint);
				bIsHooked = true;
			}
			else
			{
				// bIsHooked를 Replicated 타입으로 했지만,
				Server_RequestSpawnGrappleActor(GrappleCreatePoint);
				// 서버RPC 요청과 같이 로컬값도 바꿔주면 즉시 적용됨
				bIsHooked = true;
			}
			
			
			//SpawnGrappleActor(GrappleCreatePoint);
			//bIsHooked = true;
		}
		else
		{
			GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::White, TEXT("Grapple NoHit"));
			// 그래플 발사 일단 방향대로 쏨
		}
	}
}

void UGrappleMoveComponent::StartPreview()
{
	if (bIsHooked) return;
	GetWorld()->GetTimerManager().SetTimer(
		GrapplePreviewHandle,
		this,
		&UGrappleMoveComponent::CheckGrappleHitPoint,
		inRate,
		true
	);

	//Owner->ChangeView();

	//CallSpringArmStateChange(EViewType::FirstSight);
	//Owner->ChangeViewType_First();
	//Owner->changeview
	//if (Owner!= nullptr) Owner->ChangeViewType(EViewType::FirstSight);
}

void UGrappleMoveComponent::StopPreview()
{
	GetWorld()->GetTimerManager().ClearTimer(GrapplePreviewHandle);
}


void UGrappleMoveComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UGrappleMoveComponent, bIsGrappling); // 해당 변수를 Replcation 대상으로 지정
	DOREPLIFETIME(UGrappleMoveComponent, bIsHooked);
	
	DOREPLIFETIME(UGrappleMoveComponent, GrappleTargetLocation); // 언리얼이 내부적으로 서버에서 클라로 값을 전송함
	DOREPLIFETIME(UGrappleMoveComponent, GrappleCreatePoint);
	DOREPLIFETIME(UGrappleMoveComponent, ReplicatedLocation);
	
	DOREPLIFETIME(UGrappleMoveComponent, GrappleActor);
}

void UGrappleMoveComponent::Server_RequestSpawnGrappleActor_Implementation(const FVector& Location)
{
	if (bIsHooked) return; // 재생성 방지

	SpawnGrappleActor(Location);
	bIsHooked = true;
}


void UGrappleMoveComponent::SpawnGrappleActor(const FVector& _location)
{
	if (GrapplePointObject == nullptr)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("GrapplePointActor is Null"));
		return;
	}

	//bIsHooked = true;

	// SpawnActor 시 서버에서만 실행되도록 보장.
	// 복제되는 액터는 서버에서만 생성되도록. 
	//if (!Owner->HasAuthority()) return;


	FRotator CreateRot = FRotationMatrix::MakeFromX((GrappleCreatePoint - Owner->GetActorLocation()).GetSafeNormal()).
		Rotator();

	AActor* SpawnActor = GetWorld()->SpawnActor<AActor>(GrapplePointObject, _location, CreateRot); // FRotator::ZeroRotator

	SetGrappleActor(SpawnActor);
}

void UGrappleMoveComponent::Server_RequestDestroyGrappleActor_Implementation()
{
	Owner->GrappleCable->SetVisibility(false);
	SetGravity(true, 2.0f);
	SetIsHooked(false);
	
	GrappleActor->Destroy();
	GrappleActor = nullptr;
	GrappleTargetLocation = FVector::ZeroVector;
	Owner->PlayerWidgetInstance->ShowPinIcon(bIsHooked);
}

void UGrappleMoveComponent::DestroyGrappleActor()
{
	if (GrappleActor == nullptr)
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::White, TEXT("GrappleActor = null"));
		return;
	}

	
	if (GetOwnerRole() < ROLE_Authority)
	{
		Server_RequestDestroyGrappleActor();
	}
	else
	{
		Owner->GrappleCable->SetVisibility(false);
		SetGravity(true, 2.0f);
		SetIsHooked(false);

		GrappleActor->Destroy();
		GrappleActor = nullptr;
		GrappleTargetLocation = FVector::ZeroVector;
		Owner->PlayerWidgetInstance->ShowPinIcon(bIsHooked);
	}
	if (bIsGrappling)
	{
		if (PC->IsLocalController())
		{
			GEngine->AddOnScreenDebugMessage(-1,2.0f, FColor::Blue, TEXT("LaunchCharcter"));
			// Shift가 눌리고 있는지 검사해서 함
			FVector outFec;
			FRotator outRot;
			Owner->GetController()->GetPlayerViewPoint(outFec, outRot);
			FVector dir = outRot.Vector();
			Owner->GrappleCancelLaunch(dir, GrappleSpeed);
		}
	}
}

void UGrappleMoveComponent::Server_SetIsGrappling_Implementation(bool bNewGrappling)
{
	bIsGrappling = bNewGrappling;
	//SetCancelGrappling(false);
	//bCancelGrapple = false;
}
void UGrappleMoveComponent::SetIsGrappling(bool isGrappling)
{
	SetCancelGrappling(false);
	if (GetOwnerRole() < ROLE_Authority)
	{
		bIsGrappling = isGrappling;
		Server_SetIsGrappling(isGrappling);
	}
	else
	{
		bIsGrappling = isGrappling;
	}
}

void UGrappleMoveComponent::Server_CancelGrappling_Implementation(bool isCancelGrappling)
{
	bCancelGrapple = isCancelGrappling;
}

void UGrappleMoveComponent::SetCancelGrappling(bool isCancelGrappling)
{
	if (GetOwnerRole() < ROLE_Authority)
	{
		bCancelGrapple= isCancelGrappling;
		Server_CancelGrappling(isCancelGrappling);
	}
	else
	{
		bCancelGrapple= isCancelGrappling;
	}
}

void UGrappleMoveComponent::TickDebug()
{
	UWorld* engine = GetWorld();
	if (engine)
	{
		EMovementMode mode  = Owner->GetCharacterMovement()->MovementMode;
		FString ModeName;
		switch (mode)
		{
		case MOVE_Walking:        ModeName = TEXT("Walking"); break;
		case MOVE_NavWalking:     ModeName = TEXT("NavWalking"); break;
		case MOVE_Falling:        ModeName = TEXT("Falling"); break;
		case MOVE_Swimming:       ModeName = TEXT("Swimming"); break;
		case MOVE_Flying:         ModeName = TEXT("Flying"); break;
		case MOVE_Custom:         ModeName = TEXT("Custom"); break;
		case MOVE_None:           ModeName = TEXT("None"); break;
		default:                  ModeName = TEXT("Unknown"); break;
		}
	
		GEngine->AddOnScreenDebugMessage(8, 0.5f, FColor::Green,
			FString::Printf(TEXT("MovementMode: %s"), *ModeName));
		GEngine->AddOnScreenDebugMessage(5, 0.5f, FColor::Green, FString::Printf(TEXT("IsHooked ? : %d"), bIsHooked));
		GEngine->AddOnScreenDebugMessage(6, 0.5f, FColor::Green, FString::Printf(TEXT("IsGrappling ? : %d"), bIsGrappling));
		GEngine->AddOnScreenDebugMessage(7, 0.5f, FColor::Green, FString::Printf(TEXT("GravityScale ? : %f"),
			Owner->GetCharacterMovement()->GravityScale));
	}
}

void UGrappleMoveComponent::StartGrapple()
{
	SetIsGrappling(true);
	SetCancelGrappling(false);
	SetGravity(false);
}

void UGrappleMoveComponent::CancelGrapple()
{
	ElapsedTime = 0.0f;
	//SetIsGrappling(false);
	SetIsGrappling(false);
	SetCancelGrappling(true);
	//bCancelGrapple = true;
	SetGravity(true, 2.0f);
}

// 이 함수에서 위치 조정시킴
void UGrappleMoveComponent::OnRep_GrappleLocation()
{
	if (Owner->HasAuthority()) return;

	// 새로 생긴부분
	//서버의 위치를 따라가는식으로 함
	FVector CurrentLocation = Owner->GetActorLocation();
	FVector Interpolated = FMath::VInterpTo(CurrentLocation, ReplicatedLocation, GetWorld()->GetDeltaSeconds(), 12.f);
	Owner->SetActorLocation(Interpolated);
	//
	//Owner->SetActorLocation(ReplicatedLocation); 
}

void UGrappleMoveComponent::SetGravity(bool _enable, float _amount)
{
	if (_enable)
	{
		// 중력 활성화
		Owner->GetCharacterMovement()->GravityScale = _amount;
		Owner->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
		//Owner->GetCharacterMovement()->bIgnoreClientMovementErrorChecksAndCorrection = false;
	}
	else
	{
		// 중력 비활성화
		Owner->GetCharacterMovement()->GravityScale = 0.0f;
		Owner->GetCharacterMovement()->SetMovementMode(MOVE_Flying);
		//Owner->GetCharacterMovement()->bIgnoreClientMovementErrorChecksAndCorrection = true;
		// bIgnoreClientMovementErrorChecksAndCorrection 옵션을 키면
		// 클라이언트 - 서버가 불일치해도 위치보정,속도보정,모드보정을 하지 않음을 의미
	}
}

void UGrappleMoveComponent::ApplyGrappleMovementMode()
{
		Owner->GetCharacterMovement()->GravityScale = 0.0f;
		Owner->GetCharacterMovement()->SetMovementMode(MOVE_Flying);
}

AActor* UGrappleMoveComponent::GetGrappleActor()
{
	if (GrappleActor == nullptr) return nullptr;
	return GrappleActor;
}

void UGrappleMoveComponent::SetGrappleActor(AActor* _actor)
{
	if (GetOwnerRole() < ROLE_Authority)
	{
		Server_SetGrappleActor(_actor);
		//GrappleActor = SpawnActor;
		//if (Owner) Owner->CableAttachToMovePoint(GrappleActor);
	}
	else
	{
		GrappleActor = _actor;
		if (Owner) Owner->CableAttachToMovePoint(GrappleActor);
		Owner->PlayerWidgetInstance->ShowPinIcon(bIsHooked);
	}
	// GrappleCalbe도 동기화 작업 필요함
}

void UGrappleMoveComponent::Server_SetGrappleActor_Implementation(AActor* _actor)
{
	GrappleActor= _actor;
	if (Owner)
	{
		Owner->CableAttachToMovePoint(GrappleActor);
		Owner->PlayerWidgetInstance->ShowPinIcon(bIsHooked);
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.0f ,FColor::Red, TEXT("[GrappleMove] - Server_SetGrappleActor. Owenr = null"));
	}
	
}

void UGrappleMoveComponent::SetIsHooked(bool bNewHooked)
{
	if (GetOwnerRole() < ROLE_Authority)
	{
		Server_SetIsHooked(bNewHooked);
	}
	else
	{
		bIsHooked = bNewHooked;
	}
}




void UGrappleMoveComponent::Server_SetIsHooked_Implementation(bool bNewHooked)
{
	bIsHooked = bNewHooked;
}

void UGrappleMoveComponent::SimulateMove(const FGrappleMove& Move)
{
	// Grapple이동 로직을 여기서
	FVector CurrentLocation = Owner->GetActorLocation();
	FVector Velocity = Move.Direction * GrappleSpeed;
	FVector NewLocation = CurrentLocation + (Velocity * Move.DeltaTime);


	// 도착 체크
	if (FVector::Dist(NewLocation, GrappleTargetLocation) < 100.0f)
	{
		// 도착 처리
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Blue,TEXT("도착~!"));
	}
	else
	{
		Owner->SetActorLocation(NewLocation, true);
	}
}

void UGrappleMoveComponent::Server_SetTargetLocation_Implementation(FVector newTarget)
{
	GrappleTargetLocation = newTarget;
}

void UGrappleMoveComponent::SetTargetLocation(FVector _target)
{
	if (GetOwnerRole() < ROLE_Authority)
	{
		Server_SetTargetLocation(_target);
	}
	else
	{
		GrappleTargetLocation = _target;
	}
}

FGrappleMove UGrappleMoveComponent::CreateMove(float DeltaTime)
{
	// CreateMove에서 매 프레임당 Direction 계산

	FGrappleMove Move;
	Move.DeltaTime = DeltaTime;
	//DrawDebugLine(GetWorld(), Owner->GetActorLocation(), GrappleTargetLocation, FColor::Green, false, 0.2f);
	Move.Direction = (GrappleTargetLocation - Owner->GetActorLocation()).GetSafeNormal();
	Move.Time = GetWorld()->TimeSeconds;
	return Move;
}
