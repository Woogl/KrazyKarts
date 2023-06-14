// Fill out your copyright notice in the Description page of Project Settings.


#include "GoKartMovementReplicator.h"
#include <Net/UnrealNetwork.h>
#include <GameFramework/Actor.h>

// Sets default values for this component's properties
UGoKartMovementReplicator::UGoKartMovementReplicator()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicated(true);
}


// Called when the game starts
void UGoKartMovementReplicator::BeginPlay()
{
	Super::BeginPlay();

	MovementComp = GetOwner()->FindComponentByClass<UGoKartMovementComponent>();
}


// Called every frame
void UGoKartMovementReplicator::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (MovementComp == nullptr) return;

	FGoKartMove LastMove = MovementComp->GetLastMove();

	if (GetOwnerRole() == ROLE_AutonomousProxy)
	{
		UnacknowledgedMoves.Emplace(LastMove);
		Server_SendMove(LastMove);
	}

	// We are the server and in control of pawn
	else if (GetOwner()->GetRemoteRole() == ROLE_SimulatedProxy)
	{
		UpdateServerState(LastMove);
	}

	else if (GetOwnerRole() == ROLE_SimulatedProxy)
	{
		ClientTick(DeltaTime);
	}
}

void UGoKartMovementReplicator::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UGoKartMovementReplicator, ServerState);
}

void UGoKartMovementReplicator::ClearAcknowledgeMoves(FGoKartMove LastMove)
{
	TArray<FGoKartMove> NewMoves;

	for (const FGoKartMove& Move : UnacknowledgedMoves)
	{
		if (Move.Time > LastMove.Time)
		{
			NewMoves.Emplace(Move);
		}
	}

	UnacknowledgedMoves = NewMoves;
}

void UGoKartMovementReplicator::UpdateServerState(const FGoKartMove& Move)
{
	ServerState.LastMove = Move;
	ServerState.Transform = GetOwner()->GetActorTransform();
	ServerState.Velocity = MovementComp->GetVelocity();
}

void UGoKartMovementReplicator::ClientTick(float DeltaTime)
{
	ClientTimeSinceUpdate += DeltaTime;

	if (ClientTimeBetweenLastUpdates < KINDA_SMALL_NUMBER) return;
	if (MovementComp == nullptr) return;

	float LerpRatio = ClientTimeSinceUpdate / ClientTimeBetweenLastUpdates;

	FHermitCubicSpline Spline = CreateSpline();
	InterpolateLocation(Spline, LerpRatio);
	InterpolateVelocity(Spline, LerpRatio);
	InterpolateRotation(LerpRatio);
}

FHermitCubicSpline UGoKartMovementReplicator::CreateSpline()
{
	FHermitCubicSpline Spline;
	Spline.StartLocation = ClientStartTransform.GetLocation();
	Spline.TargetLocation = ServerState.Transform.GetLocation();
	Spline.StartDerivative = ClientStartVelocity * ClientTimeBetweenLastUpdates;
	Spline.TargetDerivative = ServerState.Velocity * ClientTimeBetweenLastUpdates;

	return Spline;
}

void UGoKartMovementReplicator::InterpolateLocation(const FHermitCubicSpline& Spline, float LerpRatio)
{
	FVector NewLocation = Spline.IntepolateLocation(LerpRatio);
	if (MeshOffsetRoot != nullptr)
	{
		MeshOffsetRoot->SetWorldLocation(NewLocation);
	}
}

void UGoKartMovementReplicator::InterpolateVelocity(const FHermitCubicSpline& Spline, float LerpRatio)
{
	FVector NewDerivative = Spline.IntepolateDerivative(LerpRatio);
	FVector NewVelocity = NewDerivative / ClientTimeBetweenLastUpdates;
	MovementComp->SetVelocity(NewVelocity);
}

void UGoKartMovementReplicator::InterpolateRotation(float LerpRatio)
{
	FQuat StartRotation = ClientStartTransform.GetRotation();
	FQuat TargetRotation = ServerState.Transform.GetRotation();
	FQuat NewRotation = FQuat::Slerp(StartRotation, TargetRotation, LerpRatio);
	if (MeshOffsetRoot != nullptr)
	{
		MeshOffsetRoot->SetWorldRotation(NewRotation);
	}
}

void UGoKartMovementReplicator::Server_SendMove_Implementation(FGoKartMove Move)
{
	if (MovementComp == nullptr) return;

	ClientSimulatedTime += Move.DeltaTime;
	MovementComp->SimulateMove(Move);

	UpdateServerState(Move);
}

bool UGoKartMovementReplicator::Server_SendMove_Validate(FGoKartMove Move)
{
	float ProposedTime = ClientSimulatedTime + Move.DeltaTime;
	bool CliendNotRunningAhead = ProposedTime < GetWorld()->TimeSeconds;
	if (!CliendNotRunningAhead) 
	{
		WG_LOG("Client is running too fast");
		return false;
	}
	if (!Move.IsValid())
	{
		WG_LOG("Received invalid move");
		return false;
	}
	return true;
}

void UGoKartMovementReplicator::OnRep_ServerState()
{
	if (MovementComp == nullptr) return;

	switch (GetOwnerRole())
	{
	case ROLE_AutonomousProxy:
		AutonomousProxy_OnRep_ServerState();
		break;
	case ROLE_SimulatedProxy:
		SimulatedProxy_OnRep_ServerState();
		break;
	default:
		break;
	}
}

void UGoKartMovementReplicator::SimulatedProxy_OnRep_ServerState()
{
	ClientTimeBetweenLastUpdates = ClientTimeSinceUpdate;
	ClientTimeSinceUpdate = 0.f;

	if (MeshOffsetRoot != nullptr)
	{
		ClientStartTransform.SetLocation(MeshOffsetRoot->GetComponentLocation());
		ClientStartTransform.SetRotation(MeshOffsetRoot->GetComponentQuat());
	}
	ClientStartVelocity = MovementComp->GetVelocity();

	GetOwner()->SetActorTransform(ServerState.Transform);
}

void UGoKartMovementReplicator::AutonomousProxy_OnRep_ServerState()
{
	GetOwner()->SetActorTransform(ServerState.Transform);
	MovementComp->SetVelocity(ServerState.Velocity);

	ClearAcknowledgeMoves(ServerState.LastMove);

	for (const FGoKartMove& Move : UnacknowledgedMoves)
	{
		MovementComp->SimulateMove(Move);
	}
}

