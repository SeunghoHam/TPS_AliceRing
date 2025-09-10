// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"

#include "Kismet/GameplayStatics.h"


#include "BaseCharacter.generated.h"

UCLASS()
class WORKTEST_API ABaseCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ABaseCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	//virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	//virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	
	// Die and Hit
	UFUNCTION(BlueprintImplementableEvent, Category=Status)
	void Die();

	UFUNCTION(BlueprintCallable, Category=Status)
	virtual float TakeDamage(float DamageAmount,
		struct FDamageEvent const& DamageEvent,
		class AController* EventInstigator,
		AActor* DamageCauser) override;

protected:
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category=Status)
	float MaxHP;
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category=Status)
	float MaxMP;
	
	float CurrentHP;
	float CurrentMP; // 일정 시간마다 차오르기?
};
