// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PingWidget.h" // EPingTypes
#include "PingSpawnComponent.generated.h"

class UUserWidget;
class APingActor;
class AAnby;
class AMultiPingActor;
class AAnbyController;
class APlayerState;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class WORKTEST_API UPingSpawnComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UPingSpawnComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;


	void PreviewStart();
	void PreviewEnd();
	void PreviewShow();
	bool bIsShowPreview = false;
	
	//void Ping(); // 실질적으로 핑 생성은 애가함

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category=Ping)
	FRotator WidgetRot;

	//void SpawnPing();
	/*
	UFUNCTION(Server,Reliable, BlueprintCallable, Category=Ping)
	void ServerSendPing(const FVector& Location, APlayerState* OwnerState);
	*/
	UFUNCTION(NetMulticast, Reliable)
	void MultiCastSpawnMultiPing(const FVector& Location);
	
	// 서버에다가 멀티핑 요청하기
	UFUNCTION(Server,Reliable)
	void ServerSpawnMultiPing(const FVector& Location);

	// 로컬환경에서 보일 핑 생성하기
	void SpawnLocalPreviewPing();
	UPROPERTY(Replicated)
	FVector PingLocation;
	
	UPROPERTY(EditAnywhere)
	TSubclassOf<AMultiPingActor> MultiPingActorClass;
	UPROPERTY(EditAnywhere)
	TSubclassOf<APingActor> PingActorClass;

	UPROPERTY(Replicated)
	APingActor* PingActorInstance = nullptr;
	
	//void SpawnMultiPing();
	void DisablePing();

	UFUNCTION()
	void GetDirection(float _yaw, float _pitch);

	
private:
	// 핑 선택하기
	FVector2D MouseStartPos;
	FVector2D MouseCurrentPos;
	FVector2D MouseEndPos;
	
	bool bIsCheckMouseDirection =false;
	
	class UPingWidget* PingWidget;
	//UPROPERTY()
	EPingTypes PingType;
	//void LookAtPlayer();
	//void SpawnPing();
	
	
	
	AAnby* anby;
	UPROPERTY()
	AAnbyController* Controller;

	//FVector SpawnLocation;


	// Direction
	float YawValue;
	float PitchValue;
	
	FTimerHandle PreviewHandle;
	FTimerHandle MouseDirectionTimerHandle;
	float inRate = 0.02f;
};
