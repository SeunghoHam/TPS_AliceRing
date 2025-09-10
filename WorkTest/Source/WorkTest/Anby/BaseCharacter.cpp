// Fill out your copyright notice in the Description page of Project Settings.


#include "Anby/BaseCharacter.h"

// Sets default values
ABaseCharacter::ABaseCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ABaseCharacter::BeginPlay()
{
	Super::BeginPlay();
	//CurrentHP= MaxHP;
	//CurrentMP = MaxMP;
}

// Called every frame
/*
void ABaseCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ABaseCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}
*/
float ABaseCharacter::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
                                 class AController* EventInstigator, AActor* DamageCauser)
{
	// DamageEvent : 데미지 종류
	// EventInstigator : 누가 공격했냐? ( Controller)
	// DamageCauser : 공격 근원(무기 or 캐릭터)
	CurrentHP -= DamageAmount;

	if (CurrentHP <= 0.f)
	{
		Die();
	}
	return DamageAmount;
}

