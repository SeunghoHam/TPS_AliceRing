// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GrappleMoveComponent.h"
#include "MovementReplicator.generated.h"


USTRUCT()
struct FGrappleMoveState
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	FTransform Tranform;

	UPROPERTY()
	FVector Velocity; // 미분 도함수 보간때문에 사용함

	UPROPERTY()
	FGrappleMove LastMove;
};

// 3차원 곡선으로 LastMove(클라의 현재위치) ServerState.위치(서버가 계산한 위치)보간시키기
struct FHermiteCubicSpline
{
	FVector StartLocation, StartDerivative, TargetLocation, TargetDerivative;

	FVector InterpolateLocation(float LerpRatio) const
	{
		return FMath::CubicInterp(StartLocation, StartDerivative, TargetLocation, TargetDerivative, LerpRatio);
	}
	FVector InterpolateDerivative(float LerpRatio) const
	{
		return FMath::CubicInterpDerivative(StartLocation, StartDerivative, TargetLocation, TargetDerivative, LerpRatio);
	}
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class WORKTEST_API UMovementReplicator : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UMovementReplicator();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// 서버로 GrappleMove전송해서 전달하기
	void SendGrappleMove(const FGrappleMove& Move);
	
	
private:
	void ClearAcknowledgeMoves(FGrappleMove LastMove);

	void UpdateServerState(const FGrappleMove& Move);

	//void ClientTick(float DeltaTime);
	
	FHermiteCubicSpline CreateSpline();
	
	void InterpolateLocation(const FHermiteCubicSpline &Spline, float LerpRatio);
	void InterpolateVelocity(const FHermiteCubicSpline &Spline, float LerpRatio);
	void InterpolateRotation(float LerpRatio);
	float VelocityToDerivative();

	UFUNCTION(Server, Reliable)
	void Server_SendMove(FGrappleMove Move);
	

	UPROPERTY(ReplicatedUsing = OnRep_ServerState)
	FGrappleMoveState ServerState;

	UFUNCTION()
	void OnRep_ServerState();
	void AutonomousProxy_OnRep_ServerState();
	void SimulatedProxy_OnRep_ServerState();

	TArray<FGrappleMove> UnacknowledgedMoves;

	float ClientTimeSinceUpdate;
	float ClientTimeBetweenLastUpdates;
	FTransform ClientStartTransform;
	FVector ClientStartVelocity;

	float ClientSimulatedTime;

	UPROPERTY()
	UGrappleMoveComponent* MovementComponent;
/*
	UPROPERTY()
	USceneComponent* MeshOffsetRoot;
	UFUNCTION(BlueprintCallable)
	void SetMeshOffsetRoot(USceneComponent* Root) { MeshOffsetRoot = Root; }*/
};
