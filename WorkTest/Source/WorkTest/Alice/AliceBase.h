// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Alice/System/SoundComponent.h"
#include "AliceBase.generated.h"

//class USoundComponent;
class UDamageNumberSubsystem;
class AAliceController;
class UAlicePlayerWidget;
class AAlice;
// (현재값, 최대값) 전달용 델리게이트
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnHPChanged, float /*Current*/, float /*Max*/);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnMPChanged, float /*Current*/, float /*Max*/);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnBossHPChanged, float /*Current*/, float /*Max*/);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnBossMPChanged, float /*Current*/, float /*Max*/);
UCLASS()
class WORKTEST_API AAliceBase : public ACharacter
{
	GENERATED_BODY()


	bool CheckIsDead(){ return IsDead;}
public:
	virtual void BeginPlay() override;

	
	// Sets default values for this character's properties
	AAliceBase();

	UPROPERTY(BlueprintInternalUseOnly, EditInstanceOnly, Category=Emote)
	TObjectPtr<AActor> EmotePointActor;

	UFUNCTION(BlueprintCallable)
	void MoveEmotePointActor();

	UFUNCTION(BlueprintCallable)
	void GetDamaged(float _damage);
	UFUNCTION()
	void StatusInitialize(float _maxHP= 100, float _maxMP =100);
	void BossStatusInitialize(float _maxHP= 100, float _maxMP =100);
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite,Category=Status)	
	float MaxHP;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite,Category=Status)	
	float MaxMP;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite,Category=Status)	
	float CurrentHP;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite,Category=Status)	
	float CurrentMP;

	FOnHPChanged OnHPChanged;
	FOnMPChanged OnMPChanged;
	FOnBossHPChanged OnBossHPChanged;
	FOnBossMPChanged OnBossMPChanged;

	// Getter
	UFUNCTION(BlueprintCallable, BlueprintPure) float GetMaxHP() const { return MaxHP; }
	UFUNCTION(BlueprintCallable, BlueprintPure) float GetMaxMP() const { return MaxMP; }
	UFUNCTION(BlueprintCallable, BlueprintPure) float GetCurrentHP() const { return CurrentHP; }
	UFUNCTION(BlueprintCallable, BlueprintPure) float GetCurrentMP() const { return CurrentMP; }

	// Setter (브로드캐스트 포함)
	UFUNCTION(BlueprintCallable) void SetCurrentHP(float NewHP);
	UFUNCTION(BlueprintCallable) void SetCurrentMP(float NewMP);

	UFUNCTION(BlueprintCallable) void SetBossCurrentHP(float NewHP);
	UFUNCTION(BlueprintCallable) void SetBossCurrentMP(float NewMP);

	void AddHP(float Delta);
	void AddMP(float Delta);

	void AddBossHP(float Delta);
	void AddBossMP(float Delta);
protected:
	AAlice* Alice;
	AAliceController* AliceController;
	USoundComponent* SoundManager;
	UAlicePlayerWidget * PlayerWidget;

	void SetPlayerWidget(UAlicePlayerWidget* _widget){ PlayerWidget = _widget;}
	void PlaySound(ESoundName _name, float _volume);
	void PlayRandomSound(TArray<ESoundName> _arr, float _volume);


	
	bool IsDead = false;

	bool bIsAvoiding= false;
	virtual void CheckCurrentHP();
	virtual void CheckCurrentMP();
	float Damage = 0.0f;
	virtual void AvoidAction();
	void ShowDamage(float _damage);

	
private:
	
FTimerHandle DamageDelayHandle;
	float DamageDelayTime = 0.3f;
	bool bCanDamaged  = true;
	
	UDamageNumberSubsystem* DamageNumberSubsystem;
};
