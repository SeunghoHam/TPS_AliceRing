// Fill out your copyright notice in the Description page of Project Settings.


#include "Alice/System/DamageNumberSubsystem.h"

#include "DamageNumberActor.h"

UDamageNumberSubsystem::UDamageNumberSubsystem()
{
	if (DamageActorClass == nullptr)
	{
		// 경로 뒤에 _C (클래스를 의미)
		static ConstructorHelpers::FClassFinder<ADamageNumberActor> BPClass(TEXT("/Game/A_ProjectAlice/Actor/DamageNumber/BP_DamageNumber.BP_DamageNumber_C"));
		if (BPClass.Succeeded())
		{
			DamageActorClass = (BPClass.Class); // BP 할당
		}
	}
}

void UDamageNumberSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	//auto* Sub =  GetGameInstance()->GetSubsystem<UDamageNumberSubsystem>();
	
	UE_LOG(LogTemp, Warning, TEXT("[DamageNumberSystem] Create Pooling DamageActor"));
	FVector SpawnLocation = FVector(0,0,100);
	FRotator SpawnRotation = FRotator::ZeroRotator;
	FActorSpawnParameters Params;
	//Params.Owner = this;                // 이 액터가 스폰의 소유자
	//Params.Instigator = GetInstigator(); // 공격자/시작자 정보
	Params.SpawnCollisionHandlingOverride =
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn; // 충돌 무시하고 스폰
	// 미리 30개 생성해두기
	UWorld* World = GetWorld();
	if (World)
	{
		UClass* cls = DamageActorClass.Get();
		for (int i =0; i <30; ++i)
		{
			//ADamageNumberActor* actor = World->SpawnActor<ADamageNumberActor>(DamageActorClass,FVector::ZeroVector,FRotator::ZeroRotator,Params);
			auto* actor = Cast<ADamageNumberActor>(World->SpawnActor(cls));
			actor->SetActorHiddenInGame(true);
			actor->SetActorTickEnabled(false);
			Pool.Add(actor);
		}
	}
}

void UDamageNumberSubsystem::Deinitialize()
{
	if (UWorld* World = GetWorld())
	{
		for (auto* actor : Pool) if (IsValid(actor)) actor->Destroy();
	}
	Pool.Empty();
	UE_LOG(LogTemp, Warning, TEXT("[DamageNumberSystem] Destroy Pooling DamageActor"));
	
	Super::Deinitialize();
}

ADamageNumberActor* UDamageNumberSubsystem::Acquire(APlayerController* _pc)
{
	if (!_pc) return nullptr;
	if (!CachedCam.IsValid()) CachedCam = _pc->PlayerCameraManager;

	for (ADamageNumberActor* actor : Pool)
	{
		if (actor && !actor->bInUse)
		{
			actor->ActivePooled(CachedCam.Get(), this);
			return actor;
		}
	}

	// 만약 부족하면 추가되도록 30개로도 될거같긴함
	if (UWorld* world =GetWorld())
	{
		ADamageNumberActor* actor = world->SpawnActor<ADamageNumberActor>();
		actor->ActivePooled(CachedCam.Get(),this);
		//actor->SetCameraRef(CachedCam.Get());
		Pool.Add(actor); // 추가생성
		return actor;
	}
	return nullptr;
}

void UDamageNumberSubsystem::Release(ADamageNumberActor* _actor)
{
	// 숨김처리하고 풀에 push
	if (!_actor) return;
	_actor->DeactivePooled();
}



/*
 *
 *멀티플레이 계산을 한다면
// Enemy or CombatComponent
UFUNCTION(NetMulticast, Unreliable)
void Multicast_ShowDamage(float Damage, bool bCrit, FVector HitLocation);

void AEnemy::Multicast_ShowDamage_Implementation(float Damage, bool bCrit, FVector HitLocation)
{
	if (auto* PC = GetWorld()->GetFirstPlayerController())
	{
		if (auto* Sub = PC->GetGameInstance()->GetSubsystem<UDamageNumberSubsystem>())
		{
			if (auto* A = Sub->Acquire(PC))
			{
				const FLinearColor Col = bCrit ? FLinearColor(1,0.85,0.2) : FLinearColor(0.95,0.95,1);
				A->Show(Damage, bCrit, HitLocation + FVector(0,0,60), Col, 0.8f);
			}
		}
	}
}
*/