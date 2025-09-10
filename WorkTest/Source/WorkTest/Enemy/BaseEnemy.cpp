// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/BaseEnemy.h"
#include "TimerManager.h"
#include "Engine/World.h"

// Socket
#include "Anby.h"
#include "CombatComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"
//#include "StatusComponent.h"

#include "EnemyAnimInstance.h"
#include "Net/UnrealNetwork.h"

#include "AnbyGameState.h"
#include "MathUtil.h"

ABaseEnemy::ABaseEnemy()
{
	//StatusSystem = CreateDefaultSubobject<UStatusComponent>(TEXT("StatusSystem"));
	//EnemyAnimInstance = Cast<UEnemyAnimInstance>(GetMesh()->GetAnimInstance());
	bReplicates= true;
}

void ABaseEnemy::BeginPlay()
{
	Super::BeginPlay();

	AttachWeapon(WeaponTemplate);
	//TestAttack();

	OnRep_SetEnemy();
	_bossCurrentHP=  100;
	_bossMaxHP = 100;
	//OnRep_CurrnetHP();

	if (HasAuthority())
	{
		AAnbyGameState* GS = Cast<AAnbyGameState>(GetWorld()->GetGameState());
		GS->BossCurrnetHP= _bossCurrentHP;
		GS->BossMaxHP = _bossMaxHP;
	}

}

void ABaseEnemy::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}

FText ABaseEnemy::GetBossName()
{
	FText drake = FText::FromString(TEXT("::Angry Chu::"));
	return drake;
}

void ABaseEnemy::OnRep_CurrnetHP()
{
	// 피격 시 UI, HitReaction 여기서실행
	if (IsLocallyControlled())
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::White, TEXT("Locally OnRep_CurrentHP"));
	}
	
	AAnbyGameState* GS = Cast<AAnbyGameState>(GetWorld()->GetGameState());
	if (GS)
	{
		GS->BossCurrnetHP = _bossCurrentHP;
		//GS->BossMaxHP = MaxHP;
		if (!HasAuthority()) // 클라에서는 직접실행
		{
			GS->OnRep_BossHP();
			//GS->OnBossHPChanged.Broadcast(_bossCurrentHP,_bossMaxHP);
		}
		UE_LOG(LogTemp, Warning, TEXT("CurrentHP: %f"), GS->BossCurrnetHP);
		UE_LOG(LogTemp, Warning, TEXT("NaxHP: %f"), GS->BossMaxHP);
	}
}

void ABaseEnemy::ServerHitEnemy_Implementation(float _damage)
{
	TestDamaged(_damage);
}


void ABaseEnemy::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ABaseEnemy, _bossCurrentHP);
	//DOREPLIFETIME(ABaseEnemy, _bossMaxHP);
	//DOREPLIFETIME(ABaseEnemy, BossMaxHP);
}

void ABaseEnemy::StopTimer()
{
	GetWorldTimerManager().ClearTimer(TimerHandle);
}

void ABaseEnemy::TakeDamageFromCombat(float _damage)
{
	//GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::White,TEXT("TakeDamageFromCombat"));

	AAnbyGameState* GS = Cast<AAnbyGameState>(GetWorld()->GetGameState());
	if (GS)
	{
		GS->ApplyBossDamage(_damage);
	}
}

void ABaseEnemy::TestDamaged(float _damage)
{
	//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::White,TEXT("1. TestDamage 함수 실행"));

	// 서버에서만 체력 감소하도록 클라에서 발생 시 무시시킴
	// TestDamaged 호출 자체를 서버에서만 함
	if (!HasAuthority()) return;

	_bossCurrentHP = FMathf::Clamp(_bossCurrentHP -_damage, 0.0f, _bossMaxHP);
	/*
	_bossCurrentHP -= _damage;
	if (!HasAuthority())
	{
		OnRep_CurrnetHP();
	}*/
}

void ABaseEnemy::OnRep_SetEnemy()
{
	AAnbyGameState* GS = Cast<AAnbyGameState>(GetWorld()->GetGameState());
	if (GS != nullptr)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green,TEXT("BaseEnmey_ Onrep"));
		//GS->CurrentTargetEnemy = this; // TODO : 이거 사용할거면 해줘야함!
	}
}
void ABaseEnemy::AttachWeapon(TSubclassOf<AActor> weapon)
{
	if (weapon)
	{
		AActor* SpawnActor = GetWorld()->SpawnActor<AActor>(weapon);

		const  USkeletalMeshSocket* WeaponSocket = GetMesh()->GetSocketByName("WeaponPoint");

		if (WeaponSocket && SpawnActor)
		{
			WeaponSocket->AttachActor(SpawnActor, GetMesh());
		}
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("WeaponTemplate None!"));
	}
}

void ABaseEnemy::SetTargetCharacter(AAnby* anby)
{
	// 지금은 그냥 설정하는데, 가장 근접 or 도발을 사용한 캐릭터로 타겟 옮겨가도록 설정해야함
	this->TargetCharacter = anby;
}

AAnby* ABaseEnemy::GetTargetCharacter()
{
	if (TargetCharacter != nullptr)
	{
		return TargetCharacter;
	}
	else return nullptr;
}

UEnemyAnimInstance* ABaseEnemy::GetEnemyAnimInstance()
{
	// 안씀
	if (EnemyAnimInstance == nullptr)
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Red, TEXT("EnmeyAnimInstance is null"));
		return nullptr;
	}
		return EnemyAnimInstance;
}

UEnemyAnimInstance* ABaseEnemy::SetAnimInstance(USkeletalMeshComponent* meshComp)
{
	EnemyAnimInstance = Cast<UEnemyAnimInstance>(meshComp->GetAnimInstance());
	return EnemyAnimInstance;
}


