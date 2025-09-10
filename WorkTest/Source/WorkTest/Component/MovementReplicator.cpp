// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/MovementReplicator.h"

#include "Net/UnrealNetwork.h"

// Sets default values for this component's properties
UMovementReplicator::UMovementReplicator()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicated(true);
	// ...
}


// Called when the game starts
void UMovementReplicator::BeginPlay()
{
	Super::BeginPlay();

	// ...
	MovementComponent = GetOwner()->FindComponentByClass<UGrappleMoveComponent>();
	if (MovementComponent)
		UE_LOG(LogTemp, Warning, TEXT("[Replicator] Movement Get"));
}

void UMovementReplicator::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UMovementReplicator, ServerState);
}


// Called every frame
void UMovementReplicator::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (MovementComponent == nullptr)return;

	// 네트워크 보간만 사용함
	if (GetOwnerRole() == ROLE_SimulatedProxy)
	{
		ClientTimeSinceUpdate += DeltaTime;
		if (ClientTimeBetweenLastUpdates < KINDA_SMALL_NUMBER) return; // 너무 작은수면 계산이 안되서 반환시킴

		float LerpRatio= ClientTimeSinceUpdate / ClientTimeBetweenLastUpdates; // 시작시간 / 총시간으로 러프 계산
		FHermiteCubicSpline Spline = CreateSpline();

		InterpolateLocation(Spline, LerpRatio);
		InterpolateVelocity(Spline,LerpRatio);
		InterpolateRotation(LerpRatio);
	}
/*
	FGrappleMove LastMove=  MovementComponent->GetLastMove();
	
	if (GetOwnerRole() == ROLE_AutonomousProxy)
	{
		UnacknowledgedMoves.Add(LastMove);
		Server_SendMove(LastMove);
	}
	else if (GetOwner()->GetRemoteRole() == ROLE_SimulatedProxy)
	{
		UpdateServerState(LastMove);
	}

	if (GetOwnerRole() == ROLE_SimulatedProxy) // 서버측?
	{
		ClientTimeSinceUpdate += DeltaTime;
		if (ClientTimeBetweenLastUpdates < KINDA_SMALL_NUMBER) return; // 너무 작은수면 계산이 안되서 반환시킴

		float LerpRatio= ClientTimeSinceUpdate / ClientTimeBetweenLastUpdates; // 시작시간 / 총시간으로 러프 계산
		FHermiteCubicSpline Spline = CreateSpline();

		InterpolateLocation(Spline, LerpRatio);
		InterpolateVelocity(Spline,LerpRatio);
		InterpolateRotation(LerpRatio);
	}*/
/*
	// We are the server and in control of the pawn.
	if (GetOwner()->GetRemoteRole() == ROLE_SimulatedProxy)
	{
		UpdateServerState(LastMove);
	}

	if (GetOwnerRole() == ROLE_SimulatedProxy)
	{
		ClientTick(DeltaTime);
	}*/
}

void UMovementReplicator::SendGrappleMove(const FGrappleMove& Move)
{
	if (MovementComponent == nullptr)
	{
		GEngine->AddOnScreenDebugMessage(-1, 0.2f, FColor::Red, TEXT("[Replicator] : Movement = null"));
		return;
	}
	if (GetOwnerRole() == ROLE_AutonomousProxy)
	{
		UnacknowledgedMoves.Add(Move);
		Server_SendMove(Move);
	}
	else if (GetOwner()->GetRemoteRole() == ROLE_SimulatedProxy)
	{
		UpdateServerState(Move);
	}
}

void UMovementReplicator::ClearAcknowledgeMoves(FGrappleMove LastMove)
{
	TArray<FGrappleMove> NewMoves;

	for (const FGrappleMove& Move : UnacknowledgedMoves)
	{
		if (Move.Time > LastMove.Time)
		{
			NewMoves.Add(Move);
		}
	}

	UnacknowledgedMoves = NewMoves;
}

void UMovementReplicator::UpdateServerState(const FGrappleMove& Move)
{
	ServerState.LastMove = Move;
	ServerState.Tranform = GetOwner()->GetActorTransform();
	ServerState.Velocity = Move.Direction * MovementComponent->GetGrappleSpeed(); // 수치 설정해줘야함
}
/*
void UMovementReplicator::ClientTick(float DeltaTime)
{
	ClientTimeSinceUpdate += DeltaTime;

	if (ClientTimeBetweenLastUpdates < KINDA_SMALL_NUMBER) return;
	if (MovementComponent == nullptr) return;

	float LerpRatio = ClientTimeSinceUpdate / ClientTimeBetweenLastUpdates;
	FHermiteCubicSpline Spline = CreateSpline();

	InterpolateLocation(Spline, LerpRatio);

	InterpolateVelocity(Spline, LerpRatio);

	InterpolateRotation(LerpRatio);
}
*/

FHermiteCubicSpline UMovementReplicator::CreateSpline()
{
	FHermiteCubicSpline Spline;
	Spline.TargetLocation = ServerState.Tranform.GetLocation();
	Spline.StartLocation = ClientStartTransform.GetLocation();
	Spline.StartDerivative = ClientStartVelocity * VelocityToDerivative();
	Spline.TargetDerivative = ServerState.Velocity * VelocityToDerivative();
	return Spline;
}

void UMovementReplicator::InterpolateLocation(const FHermiteCubicSpline& Spline, float LerpRatio)
{
	FVector NewLocation = Spline.InterpolateLocation(LerpRatio);
	GetOwner()->SetActorLocation(NewLocation,true); // 위치 튀는거 막아줌
	/*
	if (MeshOffsetRoot != nullptr)
	{
		MeshOffsetRoot->SetWorldLocation(NewLocation);
	}*/
}

void UMovementReplicator::InterpolateVelocity(const FHermiteCubicSpline& Spline, float LerpRatio)
{
	FVector NewDerivative = Spline.InterpolateDerivative(LerpRatio);
	FVector NewVelocity = NewDerivative / VelocityToDerivative();
	ClientStartVelocity = NewVelocity;
	//MovementComponent->SetVelocity(NewVelocity);
}

void UMovementReplicator::InterpolateRotation(float LerpRatio)
{
	FQuat TargetRotation = ServerState.Tranform.GetRotation();
	FQuat StartRotation = ClientStartTransform.GetRotation();

	FQuat NewRotation = FQuat::Slerp(StartRotation, TargetRotation, LerpRatio);
/*
	if (MeshOffsetRoot != nullptr)
	{
		MeshOffsetRoot->SetWorldRotation(NewRotation);
	}*/
}

float UMovementReplicator::VelocityToDerivative()
{
	return ClientTimeBetweenLastUpdates * 100;

}


void UMovementReplicator::OnRep_ServerState()
{
	/*
	if (!MovementComponent )return;
	GetOwner()->SetActorTransform(ServerState.Tranform);
	MovementComponent->SimulateMove(ServerState.LastMove);
	*/
	switch (GetOwnerRole())
	{
	case ROLE_AutonomousProxy: // 클라측
		AutonomousProxy_OnRep_ServerState();
		break;
	case ROLE_SimulatedProxy: // 서버측
		SimulatedProxy_OnRep_ServerState();
		break;
	default:
		break;
	}
}


void UMovementReplicator::AutonomousProxy_OnRep_ServerState()
{
	if (MovementComponent == nullptr) return;

	GetOwner()->SetActorTransform(ServerState.Tranform);
	//MovementComponent->SetVelocity(ServerState.Velocity);

	ClearAcknowledgeMoves(ServerState.LastMove);

	for (const FGrappleMove& Move : UnacknowledgedMoves)
	{
		MovementComponent->SimulateMove(Move);
	}
}

void UMovementReplicator::SimulatedProxy_OnRep_ServerState()
{
	if (MovementComponent == nullptr) return;

	ClientTimeBetweenLastUpdates = ClientTimeSinceUpdate;
	ClientTimeSinceUpdate = 0;
/*
	if (MeshOffsetRoot != nullptr)
	{
		ClientStartTransform.SetLocation(MeshOffsetRoot->GetComponentLocation());
		ClientStartTransform.SetRotation(MeshOffsetRoot->GetComponentQuat());
	}*/
	ClientStartTransform = GetOwner()->GetActorTransform();
	ClientStartVelocity = ServerState.Velocity;

	GetOwner()->SetActorTransform(ServerState.Tranform);
}

void UMovementReplicator::Server_SendMove_Implementation(FGrappleMove Move)
{
	if (MovementComponent == nullptr) return;

	MovementComponent->SimulateMove(Move); // 서버에서도 실행해줘야 반영됨.
	UpdateServerState(Move);
}