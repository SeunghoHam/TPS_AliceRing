// Fill out your copyright notice in the Description page of Project Settings.


#include "Alice/Doro.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Alice.h"
#include "Alice/DoroWidget.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ADoro::ADoro()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	auto* SlimeMove = GetCharacterMovement();
	SlimeMove->GravityScale = 1.6;
	SlimeMove->GroundFriction = 8.f;
	SlimeMove->BrakingDecelerationWalking = 2800.f; // 1500~2800
	SlimeMove->bOrientRotationToMovement = true;
	SlimeMove->RotationRate = FRotator(0, 540, 0);

	DoroWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("DoroWidget"));

	static ConstructorHelpers::FClassFinder<UDoroWidget> DoroWidgetBPClass (TEXT("/Game/A_ProjectAlice/Widgets/WBP_DoroWidget.WBP_DoroWidget_C"));
	if (DoroWidgetBPClass.Class != nullptr)
	{
		DoroWidgetClass = DoroWidgetBPClass.Class;
		UE_LOG(LogTemp, Warning, TEXT("DoroWidgetClass : %s"), *DoroWidgetClass->GetName());
		DoroWidget->SetWidgetClass(DoroWidgetClass);
		//DoroWidgetInstance = Cast<UDoroWidget>(DoroWidgetBPClass.Class.Get());
	}
}

// Called when the game starts or when spawnd
void ADoro::BeginPlay()
{
	Super::BeginPlay();
	
	//GEngine->AddOnScreenDebugMessage(-1, 2.0f,FColor::Green, TEXT("DoroBeginplay"));
	Alice = Cast<AAlice> (UGameplayStatics::GetPlayerCharacter(GetWorld(),0));
	SetMoveDir(false);

	if (DoroWidgetClass)
	{
		DoroWidgetInstance = Cast<UDoroWidget>(DoroWidget->GetUserWidgetObject());
		
		float hp = 25.f;
		CurrentHP = hp;
		MaxHP=  hp;
		if (DoroWidgetInstance)
		{
			//UE_LOG(LogTemp, Warning, TEXT("DoroWidgetInstance : %s"), *DoroWidgetInstance->GetName());
			DoroWidgetInstance->HPChange(CurrentHP, MaxHP);
		}
		else
		{
			GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Red,TEXT("[Doro] WidgetInstance=  null"));
		}
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Red,TEXT("[Doro] DoroWidgetClass=  null"));
	}

	//HopStart();

	//Collision = FindComponentByClass<UCapsuleComponent>();
	SetDoroActive(false);
	SetOutLineEnable(false);
}


// Called every frame
void ADoro::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//GEngine->AddOnScreenDebugMessage(3, 0.2f, FColor::White, FString::Printf(TEXT("Is Landed? %f"), bIsLanded));
	DoroWidget->SetWorldLocation(GetActorLocation() + FVector(0.f ,0.f, 80.f));
	if (!Target) return;
	FVector dir  = (Target->GetActorLocation() - GetActorLocation()).GetSafeNormal();
	FRotator rot = dir.Rotation();
	float yawOnly = rot.Yaw;
	DoroWidget->SetWorldRotation(FRotator(0.f,yawOnly, 0.f));
	
	TimeSinceLastHop += DeltaTime;
	//alpha += DeltaTime;
	// 이동 입력 (지상에서약간 끌림)
	if (!GetCharacterMovement()->IsFalling() && !MoveDir.IsNearlyZero())
	{
		AddMovementInput(MoveDir, 1.0f);
	}


	if (bIsAttack)
	{
		AttackAlpha += DeltaTime;
		float Alpha = FMath::Clamp(AttackAlpha / 0.5f, 0.f, 1.f);
		float Smooth = Ease01(Alpha, 2.0f); // EaseEXp 2.0 ~ 2.4 권장
		const FVector cur = FMath::Lerp(StartLoc,TargetLoc, Smooth);
		SetActorLocation(cur);
		if (Smooth >= 1.0f)
		{
			bIsAttack = false;
		}
	}

	
	// 스프링 적분
	IntergrateWobble(DeltaTime);
	ApplySquashScale(Wobble);

	if (TimeSinceLastHop >= 1)
	{
		bIsRot = false;
	}

	if (!bIsRot) return;
	const FQuat InterpQuat = FQuat::Slerp(StartRot, TargetRot, FMath::Clamp(TimeSinceLastHop, 0.f, 1.f)).
		GetNormalized();
	const float YawOnly = InterpQuat.Rotator().Yaw;
	SetActorRotation(FRotator(0.f, YawOnly, 0.f), ETeleportType::None);

	
	//FQuat rot =  FMath::Lerp(StartRot, TargetRot,TimeSinceLastHop);
}

void ADoro::Landed(const FHitResult& Hit)
{
	bIsLanded = true;
	Super::Landed(Hit);
	
	// 착지 스쿼시 킥
	// 킥 방향이 음인게 납작->복원에 좋음
	WobbleVel -= LandKick;
	SetMoveDir(false);
	//GEngine->AddOnScreenDebugMessage(-1, 3.0f,FColor::Cyan,TEXT("Doro Landed"));
}

void ADoro::CheckCurrentHP()
{
	// Super::CheckCurrentHP();
	CurrentHP -= Damage;
	
	if (DoroWidgetInstance)
	{
		DoroWidgetInstance->HPChange(CurrentHP, MaxHP);
	}

	if (CurrentHP <= 0.f)
	{
		if (Alice)
		{
			Alice->HuntCountUp();
		}
		Destroy();
	}
		
	//AddHP(-Damage);
	//DoroWidgetInstance->HPStatusChange(CurrentHP,MaxHP);
}

bool ADoro::ActiveCheck()
{
	return bIsActive;
}

void ADoro::SetDoroActive(bool _isActive)
{
	bIsActive= _isActive;
	GetMesh()->SetHiddenInGame(!_isActive);

	if (_isActive)
	{
		DoroWidgetInstance->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		DoroWidgetInstance->SetVisibility(ESlateVisibility::Hidden);
	}
	
}

void ADoro::CreateAttackCollision(FVector _halfSize, float _end)
{
	//FVector Forward = GetActorForwardVector();
	const FVector F = GetActorForwardVector().GetSafeNormal();
	const FVector CollisionStart = GetActorLocation() + F * 50.f;
	const FVector CollisionEnd = GetActorLocation() + F * _end;

	//DrawDebugSphere(GetWorld(), CollisionStart, 32.0f, 16, FColor::Blue, false, 1.0f);
	//DrawDebugSphere(GetWorld(), CollisionEnd, 32.0f, 16, FColor::Green, false, 1.0f);

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

	const float Travel = FMath::Max(_end - 50.f, 0.f);
	const float TotalLen = Travel + 2.f * _halfSize.X; // = Length
	const float HalfTotalLen = TotalLen * 0.5f; // = HalfX + Travel/2
	const FVector CoverCenter = CollisionStart + F * HalfTotalLen;
	const FVector CoverExtent = FVector(HalfTotalLen, _halfSize.Y, _halfSize.Z);

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
				alice->GetDamaged(20.0f);

				FVector dir =(Target->GetActorLocation() - GetActorLocation()).GetSafeNormal() * 1000.0f;
				alice->LaunchCharacter(dir,true,false);
			}
		}
	}
}

float ADoro::Ease01(float Alpha, float Exp) const
{
	Alpha = FMath::Clamp(Alpha, 0.f, 1.f);
	return FMath::InterpEaseInOut(0.f, 1.0f, Alpha, Exp);
}

FVector ADoro::CheckFront(float _length)
{
	FVector maxLocation = FVector::ZeroVector;
	FHitResult Hit;
	// ECollisionChannel Channel;
	FCollisionQueryParams Params(NAME_None,false, this);
	Params.AddIgnoredActor(this);
	if (Target) Params.AddIgnoredActor(Target);
	bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, GetActorLocation(),GetActorLocation() +(GetActorForwardVector() * _length),
		ECC_Visibility, Params);
	if (bHit)
	{
		//GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, FString::Printf(TEXT("Hitactor: %s"), * Hit.GetActor()->GetName()));
		maxLocation =  Hit.Location + (Hit.ImpactNormal*50);
		maxLocation.Z = GetActorLocation().Z;
		//DrawDebugSphere(GetWorld(),maxLocation,32.f,16,FColor::Green,false,1.0f);
		return maxLocation;
	}
	//DrawDebugLine(GetWorld(),GetActorLocation(),GetActorForwardVector() * _length,FColor::White,false, 1.0f);
	return maxLocation;
}

void ADoro::HopStart()
{
	GetWorld()->GetTimerManager().ClearTimer(JumpTimerHandle);
	GetWorld()->GetTimerManager().SetTimer(JumpTimerHandle,this,&ADoro::TryHop, 2.5f, true, 0.3f);
}

void ADoro::TryHop()
{
	bIsLanded = false;
	//SetMoveDir(true);
	SetMoveDir(false); // 포폴 영상찍기용으로 false로 해둠
	LookTarget();
	if (!GetCharacterMovement()->IsFalling() && TimeSinceLastHop >= MinHopInterval)
	{
		// 이륙 XY + Z 입펄스
		const FVector XY = MoveDir * HopXY;
		LaunchCharacter(FVector(XY.X, XY.Y, HopZ), false, true); // 점프는 Z override

		// 이륙 스트레치 킥
		// TakeOfKick 을 0.8~1.1 로 만들어서 이륙시 길쭉하게 만듬
		// 마찬가지로 착지시 landKick 1.2 ~ 1.6
		WobbleVel += TAkeoffKick;

		//Wobble     = FMath::Clamp(Wobble + 0.15f, -2.f, 2.f); // 선택: 즉시 약간 길쭉해 보이는 프리-스트레치
		TimeSinceLastHop = 0.f;

		//SetOutLineEnable(true);
	}
}
void ADoro::AttackPrev()
{
	DoroAngry(true);
	StartLoc = GetActorLocation();
	if (Target)
	{
		FRotator dir =(Target->GetActorLocation() - GetActorLocation()).GetSafeNormal().Rotation();
		const float yawOnly = dir.Yaw;
		SetActorRotation(FRotator(0.f, yawOnly, 0.f));
	}
			

	AttackAlpha= 0.f;
	//float Length= 1500.0f;
	FVector loc = CheckFront(end);
	if (loc == FVector::ZeroVector)
	{
		// 맞은 적이 없음 = 전방 length만큼
		TargetLoc = GetActorLocation()  + (GetActorForwardVector()* end);
	}
	else
	{
		TargetLoc = loc;
	}
	SetOutLineEnable(true);
}

void ADoro::TryAttack()
{
	bIsAttack= true;
	bIsAngry = true;

	//FVector HalfSize = FVector(100.f,50.f,50.f);
	//float end = 1500.f;
	InRateWithDistance = 0.1f;
	if (Target)
	{
		float dist = FVector::Dist(GetActorLocation(),Target->GetActorLocation());
		//UE_LOG(LogTemp, Warning,TEXT("dist : %f, Dist/1600 : %f"), dist, dist/1600);
		/*;
		GEngine->AddOnScreenDebugMessage(-1, 3.0f,
			FColor::Magenta,FString::Printf(TEXT("dist : %f, Dist/16000 : %f"), dist, dist/16000));*/
		
		//InRateWithDistance = FMath::Clamp(dist / 1600, 0.0f,0.45f); // 이계산식 점검 필요하긴 혀
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.0f,FColor::Blue,TEXT("Target is null"));
	}
	
	TWeakObjectPtr<AZZZEnemy> WeakThis(this);
	GetWorld()->GetTimerManager().SetTimer(DoroAttackTimerHandle, [WeakThis, this]()
	{
		if (!WeakThis.IsValid()) return;
		CreateAttackCollision(halfSize,end);
		
	}, InRateWithDistance, false);
	
}

void ADoro::EndAttack()
{
	DoroAngry(false);
	SetOutLineEnable(false);
	bIsAngry = false;
	bIsAttack= false;
}

void ADoro::IntergrateWobble(float _deltaTime)
{
	// C =WobbleStiffness, K = WobbleDampling 이 두개값 낮추면 말랑함 추가?
	// 추천 Stiffness 14~18, Dampoing 5~6
	// 2차 시스템: W'' + c*W' + k*W = 0  (외력은 킥으로 반영)
	const float Acc = -WobbleStiffness * Wobble - WobbleDAmping * WobbleVel;
	WobbleVel += Acc * _deltaTime;
	Wobble += WobbleVel * _deltaTime;

	// 안정화(과도 발산 방지)
	Wobble = FMath::Clamp(Wobble, -2.f, 2.f);
	WobbleVel = FMath::Clamp(WobbleVel, -10.f, 10.f);
}

void ADoro::ApplySquashScale(float _wobble)
{
	// XY는 납작, Z는 길게 (질량보존 느낌)
	//
	// sx/sy = 1 - 0.25 * wobble, sz = 1 + 0.5 * w
	// 기존값이므로 w수치들을 조금씩 키워줌
	/*
	const float sx = 1.f - 0.25f * _wobble;
	const float sy = 1.f - 0.25f * _wobble;
	const float sz = 1.f + 0.5f  * _wobble;
	*/
	const float sx = 1.f - 0.35f * _wobble;
	const float sy = 1.f - 0.35f * _wobble;
	const float sz = 1.f + 0.70f * _wobble;


	SetActorScale3D(FVector(sx, sy, sz));
}

void ADoro::LookTarget()
{
	if (Target == nullptr) return;

	alpha = 0.0f;
	bIsRot = true;
	StartRot = GetActorRotation().Quaternion();
	TargetRot = MoveDir.Rotation().Quaternion();
}

void ADoro::SetMoveDir(bool _isOnTarget)
{
	if (_isOnTarget) // 타겟이 있는경우 = 0 이아님
	{
		if (FVector::Dist(GetActorLocation(), Target->GetActorLocation()) <= 1500.f)
		{
			MoveDir = (Target->GetActorLocation() - GetActorLocation()).GetSafeNormal();
		}
		else
		{
			MoveDir = GetActorForwardVector();
		}
	}
	else
	{
		MoveDir = FVector::ZeroVector;
	}
}

void ADoro::SetMeshSize(float _delta)
{
	const float w = FMath::Clamp(Wobble, -2.f, 2.f);
	const float wPos = FMath::Max(w, 0.f); // 스트레치(이륙)
	const float wNeg = FMath::Max(-w, 0.f); // 스쿼시(착지)

	// 이륙(+)일 때 Z를 더 키우고 XY는 살짝만 가늘게
	const float zFromStretch = 0.85f * wPos; // ↑ 더 과감
	const float xyFromStretch = 0.20f * wPos; // ↑ 너무 가늘어지지 않게

	// 착지(-)일 때는 기존보다 살짝 납작
	const float zFromSquash = -0.45f * wNeg;
	const float xyFromSquash = 0.40f * wNeg;

	const float sz = 1.f + zFromStretch + zFromSquash;
	const float sxy = 1.f - (xyFromStretch + xyFromSquash);

	const FVector targetScale(
		FMath::Clamp(sxy, 0.35f, 1.6f),
		FMath::Clamp(sxy, 0.35f, 1.6f),
		FMath::Clamp(sz, 0.35f, 1.8f)
	);

	// 살짝 보간해서 튐 방지
	CurrentScale = FMath::VInterpTo(CurrentScale, targetScale, _delta, 12.f);
	GetMesh()->SetRelativeScale3D(CurrentScale);
}
