// Fill out your copyright notice in the Description page of Project Settings.


#include "Alice/ZZZEnemy.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "System/EnemyRegistrySubsystem.h"
#include "Alice.h"
// Sets default values
AZZZEnemy::AZZZEnemy()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	/*
	auto* SlimeMove = GetCharacterMovement();
	SlimeMove->GravityScale = 1.3;
	SlimeMove->GroundFriction = 8.f;
	SlimeMove->BrakingDecelerationWalking = 2800.f; // 1500~2800
	SlimeMove->bOrientRotationToMovement = true;
	SlimeMove->RotationRate = FRotator(0, 540, 0);*/
}

/*
FText AZZZEnemy::GetBossName()
{
	FText name = FText::FromString(TEXT("::None::"));
	return name;
}
*/
// Called when the game starts or when spawned
void AZZZEnemy::BeginPlay()
{
	Super::BeginPlay();
	Target = Cast<AAlice>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	RS = GetGameInstance()->GetSubsystem<UEnemyRegistrySubsystem>();
	RegistryEnemy();
}

void AZZZEnemy::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (RS)
	{
		//GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green,TEXT("EndPlay"));
		RS->UnregisterEnemy(this);
	}
	Super::EndPlay(EndPlayReason);
}

void AZZZEnemy::SetCharacter(AActor* _actor)
{
	Target = Cast<AAlice>(_actor);
}


void AZZZEnemy::RegistryEnemy()
{
	
	UGameInstance* GI = GetGameInstance();
	UE_LOG(LogTemp, Warning, TEXT("[Enemy] GI=%s"), GI ? *GI->GetName() : TEXT("nullptr"));
	if (GI)
	{
		if (auto* R = GI->GetSubsystem<UEnemyRegistrySubsystem>())
		{
			R->RegisterEnemy(this);
			//GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, TEXT("Registry OK"));
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("[Enemy] Registry NOT found, retry next frame"));
			FTimerHandle Th;
			GetWorld()->GetTimerManager().SetTimer(Th, [this]()
			{
				if (auto* GI2 = GetGameInstance())
					if (auto* R2 = GI2->GetSubsystem<UEnemyRegistrySubsystem>())
					{
						R2->RegisterEnemy(this);
						UE_LOG(LogTemp, Warning, TEXT("[Enemy][Retry] Registry OK"));
					}
			}, 0.f, false);
		}
	}
	RS->RegisterEnemy(this);
	/*
	GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow,
	                                 FString::Printf(TEXT("Registry Enemy : %s"), *GetName()));*/
}

void AZZZEnemy::UnregisterEnemy()
{
	RS->UnregisterEnemy(this);
}
