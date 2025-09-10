// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/CombatComponent.h"

#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "BaseEnemy.h"
#include "Engine/OverlapResult.h"
#include "Anby.h"
#include "Elements/Framework/TypedElementMetaData.h"


UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}
void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();
	//Super::BeginPlay();

	anby = Cast<AAnby>(GetOwner());
}
bool UCombatComponent::RegisterAttack(const FInComingAttackToken& token)
{
	// 이미 등록되어있는 공격이 있다면 교체하거나 무시함.
	// 현재상태 : 교체
	CurrentAttack = token;
	return true;
}

bool UCombatComponent::CheckParringSuccess()
{
	if (bCanParring)
	{
		CurrentAttack.Reset();
		return true;
	}
	else
	{
		CurrentAttack.Reset();
		return false;
	}
	/*
	if (bCanParring)
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Green, TEXT("패링성공!"));
		CurrentAttack.Reset();
		// 공격 모션 도중에 멈추게하기 테스트
		//if (Enemy)
		{
			//Enemy->GetMesh()->GetAnimInstance()->Montage_Pause(0.1f);
		}
		return true;	
	}
	GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Red, TEXT("패링실패!"));
	CurrentAttack.Reset();
	return false;
	*/
}

ABaseEnemy* UCombatComponent::FindFrontEnemy(float radius)
{
	//DrawDebugSphere(GetWorld(),GetOwner()->GetActorLocation(),100, 16, FColor::White, false, 1.f);
	//GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::White, TEXT("FindEnemy1"));
	// 범위 내의 적 수집하기( SphereOverlap)
	TArray<FOverlapResult> Overlaps;
	FCollisionQueryParams Params(NAME_None);
	
	bool bHit = GetOwner()->GetWorld()->OverlapMultiByObjectType(
	  Overlaps,
	  GetOwner()->GetActorLocation(),
	  FQuat::Identity,
	  // 오브젝트 타입이 Pawn인것만 체크함. Enemy의 Collision 다 폰으로 바꿔잇
	  FCollisionObjectQueryParams(ECC_Pawn), // Pawn 채널
	  FCollisionShape::MakeSphere(radius),
	  Params);
	
	//DrawDebugSphere(GetWorld(),GetOwner()->GetActorLocation(),radius, 16, FColor::Green, false, 1.f);
		//UE_LOG(LogTemp, Error, TEXT("CombatSystem is NULL in %s"), *GetName());
		//UE_LOG(LogGeometry, Warning, TEXT("OverlapActor : %s"), *Overlap.GetActor()->GetName());
		//GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::White, TEXT("OverlapActor : %s"), *Overlap.GetActor()->GetName());
	
	if (!bHit) return nullptr;
	//GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::White, TEXT("FindEnemy2"));
	// 전방 확인 + 최고근접 적 찾기
	// 이게 아니라 카메라의 방향을 가져와야할듯인데
	//const FVector Forward = GetOwner()->GetActorForwardVector(); 
	const FVector Forward  = anby->GetCameraForwardVector();
	float   BestDistSq   = 9999999;
	ABaseEnemy* BestEnemy = nullptr;

	for (const FOverlapResult& Hit : Overlaps)
	{
		//GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::White, TEXT("OverlapActor : %s"), *Hit.GetActor()->GetName());
		ABaseEnemy* Enemy = Cast<ABaseEnemy>(Hit.GetActor());

		if (Enemy != nullptr)
		{
			DrawDebugSphere(GetWorld(),Enemy->GetActorLocation(), 64
			, 16, FColor::Red, false, 1.f);
		}	


		//if (!Enemy || Enemy->IsPendingKill()) continue;
		if (!Enemy) continue;
		FVector ToEnemy = (Enemy->GetActorLocation() - GetOwner()->GetActorLocation()).GetSafeNormal();
		float Dot = FVector::DotProduct(Forward, ToEnemy); // 코사인 값 나옴
		// 두 벡터 사이의 각도 세터. Dot = cos(세타)
		float DotThreshold = FMath::Cos(FMath::DegreesToRadians(ViewAngleDegree));

		// Dot값 1 : 완전히 정면
		// 0 : 90도, -1 : 완전히 반대
		if (Dot >= DotThreshold)                // “정면”에 있고
		{
			float DistSq = FVector::DistSquared(Enemy->GetActorLocation(),GetOwner()->GetActorLocation());
			if (DistSq < BestDistSq)            // 가장 가까운 적 선택
			{
				BestDistSq = DistSq;
				BestEnemy  = Enemy;
				DrawDebugSphere(GetWorld(),BestEnemy->GetActorLocation(), 50
					, 16, FColor::Red, false, 1.f);
			}
		}
	}
	/*
	DrawDebugCone(
	GetWorld(),
	GetOwner()->GetActorLocation(),
	anby->GetCameraForwardVector(),
	radius,
	FMath::DegreesToRadians(ViewAngleDegree), // Half Angle
	FMath::DegreesToRadians(ViewAngleDegree),
	12,
	FColor::Blue,
	false, 1.0f);
	*/
	return BestEnemy;
}

FVector UCombatComponent::GetMoveDestination(const FVector& target)
{
	// 1. 적 위치를 받고, 그 위치와 플레이어 방향으로 보간
	FVector normal = (target - GetOwner()->GetActorLocation()).GetSafeNormal();
	FVector Dest = target - (normal * BlankAmount);
	return Dest;
}

void UCombatComponent::GetHalfCirclePosition(const FVector& owner, const FVector& target, float t)
{
	// t : 0~1 진행률
	// r : 반지름 ( 시작점과 목적지의 거리)
	// c : 목적지 좌표 (Center)
	// ∂ : 각도 진행률
	FVector TargetLocation = target;
	FVector SelfLocation = owner; // 타겟 주위에 원 만들기
	//float Radius = FVector::Dist(Circle,Center);
	
	// Circle = Start
	FVector StartDirection = (SelfLocation - TargetLocation).GetSafeNormal();
	float Radius = FVector::Distance(SelfLocation,TargetLocation);

	// 회전축 (월드 Up 방향 기준)
	FVector RotationAxis = FVector::UpVector;

	// 현재 시간값 t (0.0 ~ 1.0)
	float Angle = PI * t; // 180도 회전

	// 반시계 방향 회전 쿼터니언
	FQuat Rotation = FQuat(RotationAxis, Angle);

	// 회전된 방향 계산
	FVector NewOffset = Rotation.RotateVector(StartDirection) * Radius;

	// 새 위치 = 타겟 위치 + 회전된 벡터
	FVector NewLocation = TargetLocation + NewOffset;
	
	GetOwner()->SetActorLocation((NewLocation));
}

void UCombatComponent::CameraViewChange(const FQuat& StartQuat, const FQuat& TargetQuat, float t)
{
	FQuat newQuat = FQuat::Slerp(StartQuat, TargetQuat, t);
	GetOwner()->SetActorRotation(newQuat);
}

void UCombatComponent::ServerAttack_Implementation(ABaseEnemy* _enemy, float _damage)
{
	if (anby == nullptr) return;
	if (_enemy== nullptr)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red,TEXT("Target = null"));
		return;
	}

	if (!GetOwner()->HasAuthority()) return;
	// 서버일때만 실행되도록
	
	_enemy->TakeDamageFromCombat(_damage);
}

void UCombatComponent::DoAttack(FVector _halfSize, float _end, float _damage)
{
	CheckAttackCollisionRange(_halfSize, _end,_damage);
}

void UCombatComponent::CheckAttackCollisionRange(FVector _halfSize, float _end, float _damage)
{
	
	FVector Forward = GetOwner()->GetActorForwardVector();
	FVector Location=  GetOwner()->GetActorLocation();
	CollisionStart = (Forward * 50) + Location;
	CollisionEnd = (Forward * _end) + Location;
	

	TArray<FHitResult> OutHits;
	FRotator Orient = GetOwner()->GetActorRotation();
	FQuat Rot = Orient.Quaternion();
	FCollisionShape BoxShape = FCollisionShape::MakeBox(HalfSize);
	// SweepMultiByChannel에서 Shape 만들어넣기
	bool bHit = GetWorld()->SweepMultiByChannel(
		OutHits,
		 CollisionStart,
		CollisionEnd,
		Rot,
		ECC_Visibility,
		BoxShape
		);

	FVector Center = (CollisionStart + CollisionEnd) * 0.5f;
	FVector BoxExtend = _halfSize;
	
	DrawDebugBox(
	GetWorld(),
	Center,
	BoxExtend,
	Rot,
	FColor::Red,
	false,
	0.5f
);
	if (bHit)
	{
		//GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Cyan,TEXT("공격성공!!"));
			for (const FHitResult& HitActor : OutHits)
			{
				if (HitActor.GetActor()->ActorHasTag("Enemy"))
				{
					ABaseEnemy* HitEnemy = Cast<ABaseEnemy>(HitActor.GetActor());
					anby->GetCombatComponent()->ServerAttack(HitEnemy,_damage);
				}
			}
	}
	else
	{
		//GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Cyan,TEXT("공격범위 외부"));
	}
	
}
