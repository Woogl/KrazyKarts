// Fill out your copyright notice in the Description page of Project Settings.


#include "GoKart.h"

// Sets default values
AGoKart::AGoKart()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	MovementComp = CreateDefaultSubobject<UGoKartMovementComponent>(TEXT("MovementComp"));
	MovementReplicator = CreateDefaultSubobject<UGoKartMovementReplicator>(TEXT("MovementReplicator"));
}

// Called when the game starts or when spawned
void AGoKart::BeginPlay()
{
	Super::BeginPlay();

	SetReplicateMovement(false);
}

// Called every frame
void AGoKart::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	ShowAuthority(DeltaTime);
}

// Called to bind functionality to input
void AGoKart::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis(TEXT("MoveForward"), this, &AGoKart::MoveForward);
	PlayerInputComponent->BindAxis(TEXT("MoveRight"), this, &AGoKart::MoveRight);
}

void AGoKart::MoveForward(float Value)
{
	if (MovementComp == nullptr) return;

	MovementComp->SetThrottle(Value);
}

void AGoKart::MoveRight(float Value)
{
	if (MovementComp == nullptr) return;

	MovementComp->SetSteeringThrow(Value);
}

void AGoKart::ShowAuthority(float DeltaTime)
{
	FString EnumString;
	UEnum::GetValueAsString(GetLocalRole(), EnumString);
	DrawDebugString(GetWorld(), FVector(0, 0, 100.f), EnumString, this, FColor::White, DeltaTime);
}
