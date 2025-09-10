// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DamageNumberActor.generated.h"

class UWidgetComponent;
class UDamageNumberWidget;
class UDamageNumberSubsystem; // Owner풀 가져와서 Life종료시 호출함
UCLASS()
class WORKTEST_API ADamageNumberActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ADamageNumberActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	UFUNCTION()
	void Show(float Value, bool bCrit, const FVector& WorldPos, const FLinearColor& Color, float Lifetime = 0.8f);
	//void SetCameraRef(APlayerCameraManager* Cam) { CamMgr = Cam; }

	// 풀링용
	bool bInUse = false;

	void ActivePooled(APlayerCameraManager* Cam, UDamageNumberSubsystem* DNSystem);
	void DeactivePooled();
	

	UPROPERTY(EditDefaultsOnly, Category=DamageNumber)
	TSubclassOf<UDamageNumberWidget> DamageWidgetClass;
	
	
	UPROPERTY(EditAnywhere, Category=Widget)
	UWidgetComponent* WidgetComp;

	UPROPERTY(Editanywhere)
	UStaticMeshComponent* SM;
private:
	UDamageNumberWidget* WidgetInstance;

	void RefreshVisibility(bool bVisible);
	void ForceRegisterAndRefresh();
	void ActiveActor();
	//void DeactiveActor();
	TWeakObjectPtr<UDamageNumberSubsystem> OwnerPool;

	UPROPERTY() UDamageNumberWidget* UI;

	APlayerCameraManager* CamMgr = nullptr;
	float Time = 0.f, Life = 1.f;
	FVector Velocity = FVector::ZeroVector; // 살짝 떠오르는 속도(월드)

	// 거리 스케일
	float DistanceScale(const FVector& CamLoc, const FVector& Me) const;

};
