// Fill out your copyright notice in the Description page of Project Settings.


#include "Alice/AliceBase.h"

#include "Alice.h"
#include "AlicePlayerWidget.h"
#include "Alice/DamageNumberActor.h"
#include "Alice/AliceController.h"
#include "Alice/System/DamageNumberSubsystem.h"
#include "Kismet/GameplayStatics.h"


// Sets default values
AAliceBase::AAliceBase()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SoundManager  = CreateDefaultSubobject<USoundComponent>(TEXT("SoundComponent"));
}

void AAliceBase::BeginPlay()
{
	Super::BeginPlay();
	DamageNumberSubsystem =  GetGameInstance()->GetSubsystem<UDamageNumberSubsystem>();
	AliceController = Cast<AAliceController>(UGameplayStatics::GetPlayerController(GetWorld(),0));
}

void AAliceBase::MoveEmotePointActor()
{
	if (!EmotePointActor)
	{
		GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Red, TEXT("[AliceBase] MoveEmotePoint = null. 월드에서 설정해주셈"));
		return;
	}
	SetActorLocation(EmotePointActor->GetActorLocation());
	SetActorRotation(EmotePointActor->GetActorRotation());
}

void AAliceBase::GetDamaged(float _damage)
{
	if (!bCanDamaged) return;
	if (bIsAvoiding)
	{
		//GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, TEXT("AliceBase Avoid"));
		AvoidAction();
		return;
	}
	Damage = _damage;
	bCanDamaged= false;
	CheckCurrentHP(); // 여기서 자식으로 가서 AddHP하기
	ShowDamage(_damage);
	TWeakObjectPtr<AAliceBase>WeakThis(this);
	GetWorld()->GetTimerManager().ClearTimer(DamageDelayHandle);
	GetWorld()->GetTimerManager().SetTimer(DamageDelayHandle, [WeakThis, this]()
	{
		bCanDamaged = true;
	}, DamageDelayTime, false);
}

void AAliceBase::StatusInitialize(float _maxHP, float _maxMP)
{
	MaxHP = _maxHP;
	MaxMP = _maxMP;
	CurrentHP = _maxHP;
	CurrentMP = _maxMP;
	
	SetCurrentHP(_maxHP);
	SetCurrentMP(_maxHP);
}

void AAliceBase::BossStatusInitialize(float _maxHP, float _maxMP)
{
	MaxHP = _maxHP;
	MaxMP = _maxMP;
	CurrentHP = _maxHP;
	CurrentMP = _maxMP;
	SetBossCurrentHP(_maxHP);
	SetBossCurrentMP(_maxMP);
}

void AAliceBase::SetCurrentHP(float NewHP)
{
	MaxHP = FMath::Max(1.f, MaxHP);
	CurrentHP = FMath::Clamp(NewHP, 0.f, MaxHP);
	if (PlayerWidget) PlayerWidget->OnHPChanged(CurrentHP, MaxHP);
	OnHPChanged.Broadcast(CurrentHP, MaxHP);
}

void AAliceBase::SetCurrentMP(float NewMP)
{
	MaxMP = FMath::Max(1.f, MaxMP);
	CurrentMP = FMath::Clamp(NewMP, 0.f, MaxMP);
	if (PlayerWidget) PlayerWidget->OnMPChanged(CurrentMP, MaxMP);
	OnMPChanged.Broadcast(CurrentMP, MaxMP);
}

void AAliceBase::SetBossCurrentHP(float NewHP)
{
	//GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, TEXT("setcurnretHP"));
	
	MaxHP = FMath::Max(1.f, MaxHP);
	CurrentHP = FMath::Clamp(NewHP, 0.f, MaxHP);
	OnBossHPChanged.Broadcast(CurrentHP, MaxHP);
}

void AAliceBase::SetBossCurrentMP(float NewMP)
{
	MaxMP = FMath::Max(1.f, MaxMP);
	CurrentMP = FMath::Clamp(NewMP, 0.f, MaxMP);
	OnBossMPChanged.Broadcast(CurrentMP, MaxMP);
}

void AAliceBase::AddHP(float Delta)
{
	SetCurrentHP(CurrentHP + Delta);
}

void AAliceBase::AddMP(float Delta)
{
	SetCurrentMP(CurrentMP + Delta);
}

void AAliceBase::AddBossHP(float Delta)
{
	//GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, TEXT("AddBossHP"));
	SetBossCurrentHP(CurrentHP + Delta);
}

void AAliceBase::AddBossMP(float Delta)
{
	SetBossCurrentMP(CurrentMP + Delta);
}

void AAliceBase::PlaySound(ESoundName _name, float _volume)
{
	if (!SoundManager) return; 
	SoundManager->Play2D(_name, _volume);
}

void AAliceBase::PlayRandomSound(TArray<ESoundName> _arr, float _volume)
{
	if (!SoundManager) return; 
	SoundManager->PlayRandom(_arr, nullptr, NAME_None, _volume);
}

void AAliceBase::CheckCurrentHP()
{
	// 여기서는 정의안함 수고링!
}

void AAliceBase::CheckCurrentMP()
{
	// 여기서는 정의안함 수고링!
}

void AAliceBase::AvoidAction()
{
	// 여기서는 정의안함 수고링!
}

void AAliceBase::ShowDamage(float _damage)
{
	//FVector Point  = ((GetActorLocation() - CurrentTarget->GetActorLocation()).GetSafeNormal() * 20)  + CurrentTarget->GetActorLocation();
	FVector Point = GetActorLocation();
	float y = FMath::FRandRange(-40.0f, 40.0f);
	float z = FMath::FRandRange(-40.0f, 40.0f);
	Point.Z += 100;
	Point.Y += y;
	Point.Z += z;
	if (ADamageNumberActor* actor = DamageNumberSubsystem->Acquire(AliceController))
	{
		// GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Green, TEXT("DT Show"));
		actor->Show(_damage, false,Point,FColor::White, 0.8f );
	}
}

