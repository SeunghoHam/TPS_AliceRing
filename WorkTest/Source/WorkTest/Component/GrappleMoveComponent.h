// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CableComponent.h"
#include "Anby.h"
#include "GrappleMoveComponent.generated.h"

USTRUCT()
struct FGrappleMove
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	FVector Direction; // 내 위치에서 타겟까지의 방향
	//UPROPERTY()
	//float SteeringThrow;

	UPROPERTY()
	float DeltaTime;
	UPROPERTY()
	float Time;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class WORKTEST_API UGrappleMoveComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UGrappleMoveComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	
public:

	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(Server, Reliable)
	void ServerFireGrapple(const FVector& Start, const FVector& End);

	UFUNCTION(BlueprintCallable)
	void FireGrapple();
	

	UPROPERTY(Editanywhere, BlueprintReadWrite, Category=Grapple)
	TSubclassOf<class AActor> GrapplePointObject;
	
	
	UPROPERTY(EditAnywhere, Category=Grapple)
	float MaxGrappleDistance = 2000.0f;

	UPROPERTY(EditAnywhere, Category=Grapple)
	FName StartSocketName = "LeftHandSocket"; // 스켈레탈에 따라 바꿔줘야함

	
	// 우클릭 지속동안은 이 범위로 보이고 클릭 해제시 그 위치로 전달하기
	UFUNCTION()
	void StartPreview();
	UFUNCTION()
	void StopPreview();
	
	void CheckGrappleHitPoint();
	
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	

	UFUNCTION()
	void SpawnGrappleActor(const FVector& _location);

	UFUNCTION(Server,Reliable)
	void Server_RequestSpawnGrappleActor(const FVector& Location);
	
	UFUNCTION(Category= Grapple)
	void DestroyGrappleActor();
	
	UFUNCTION(Server,Reliable)
	void Server_RequestDestroyGrappleActor();
	
	UFUNCTION(BlueprintCallable, Category=Grappple)
	void StartGrapple();
	
	UFUNCTION()
	void CancelGrapple();

	//UFUNCTION(Server, Reliable)
	//void Server_CancelGrapple();
	
	UPROPERTY(ReplicatedUsing = OnRep_GrappleLocation)
	FVector ReplicatedLocation; // 이 위치로 캐릭터 위치 변경?
	
	UFUNCTION()
	void OnRep_GrappleLocation();

	void SetGravity(bool _enable, float _amount = 1.f);
	void ApplyGrappleMovementMode();
	AActor* GetGrappleActor();

	UFUNCTION(Server,Reliable)
	void Server_SetGrappleActor(AActor* _actor);
	void SetGrappleActor(AActor* _actor);

	UFUNCTION(Server, Reliable)
	void Server_SetIsHooked(bool bNewHooked);

	void SetIsHooked(bool bNewHooked);

	UFUNCTION(Server,Reliable)
	void Server_SetIsGrappling(bool bNewGrappling);

	void SetIsGrappling(bool isGrappling);
	bool GetIsGrappling() { return bIsGrappling;}
	
	UFUNCTION(Server,Reliable)
	void Server_CancelGrappling(bool isCancelGrappling);
	void SetCancelGrappling(bool isCancelGrappling);
	
	
private:
	void TickDebug();
	UPROPERTY(Replicated)
	FVector GrappleTargetLocation; // 타겟지점

	UPROPERTY(Replicated)
	FVector GrappleCreatePoint; // 액터 생성 지점
	
	UPROPERTY(Replicated)
	bool bIsGrappling =  false;
	
	bool bCancelGrapple = false;
	float GrappleSpeed = 1000.0f;
	float ElapsedTime = 0.0f;
	
	UPROPERTY(Replicated)
	AActor* GrappleActor = nullptr;

	UPROPERTY(Replicated)
	bool bIsHooked = false;
	
	AAnby* Owner;
	APlayerController* PC;
	FTimerHandle GrapplePreviewHandle;
	float inRate =0.02f;
	void GrappleCreateAndSetting();


public:
	void SimulateMove(const FGrappleMove& Move);
	
	
	//FVector GetVelocity() { return Velocity; };
	//void SetVelocity(FVector Val) { Velocity = Val; };
	void SetDirection(FVector Val) { GrappleDirection = Val; };

	FGrappleMove GetLastMove() { return LastMove; }

	UFUNCTION(Server,Reliable)
	void Server_SetTargetLocation(FVector newTarget);
	
	void SetTargetLocation(FVector _target);
	void SetGrappleSpeed(float _speed) { GrappleSpeed = _speed;}
	float GetGrappleSpeed() { return GrappleSpeed; }
	FVector GrappleDirection;


private:
	FGrappleMove CreateMove(float DeltaTime);
	
	//void ApplyRotation(float DeltaTime, float SteeringThrow);

	//void UpdateLocationFromVelocity(float DeltaTime);
/*
	// The mass of the car (kg).
	UPROPERTY(EditAnywhere)
	float Mass = 10;

	// The force applied to the car when the throttle is fully down (N).
	UPROPERTY(EditAnywhere)
	float MaxDrivingForce = 10000;

	// Minimum radius of the car turning circle at full lock (m).
	UPROPERTY(EditAnywhere)
	float MinTurningRadius = 10;

	// Higher means more drag.
	UPROPERTY(EditAnywhere)
	float DragCoefficient = 16;

	// Higher means more rolling resistance.
	UPROPERTY(EditAnywhere)
	float RollingResistanceCoefficient = 0.015;

	FVector Velocity;
*/

	FGrappleMove LastMove;

	UPROPERTY()
	class UMovementReplicator* Replicator;
};
