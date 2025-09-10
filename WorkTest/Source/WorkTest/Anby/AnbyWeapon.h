// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AnbyWeapon.generated.h"

class UStaticMeshComponent;
UCLASS(BlueprintType)
class WORKTEST_API AAnbyWeapon : public AActor
{
	GENERATED_BODY()

	protected:
	virtual void BeginPlay() override;
	//virtual void Tick(float DeltaSeconds) override;
public:	
	// Sets default values for this actor's properties
	AAnbyWeapon();
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* WeaponMesh;
public:
	
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category ="Weapon")
	void Show();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category ="Weapon")
	void Hide();

	void SetWeaponVisiblity(bool _value);
	
	void WeaponEnable();
	void WeaponDisable();
	void ChangeValue(float value);
	void UpdateScalarParameter();
	UFUNCTION(BlueprintCallable, BlueprintPure,Category= "Combat")
	UStaticMeshComponent* GetMesh() { return WeaponMesh; }

private:
	float CurrentDissolve = 0.0f;
	float TargetDissolve = 0.0f;
	float LerpSpeed = 0.2f;
	float ToLerance = 0.01f; // 종료오차

	FTimerHandle ScalarUpdateTimerHandle;
	UMaterialInstanceDynamic* DM;
	
};
