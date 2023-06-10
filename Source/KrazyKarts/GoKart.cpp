// Fill out your copyright notice in the Description page of Project Settings.


#include "GoKart.h"
#include <../Plugins/EnhancedInput/Source/EnhancedInput/Public/EnhancedInputComponent.h>
#include <../Plugins/EnhancedInput/Source/EnhancedInput/Public/EnhancedInputSubsystems.h>

// Sets default values
AGoKart::AGoKart()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AGoKart::BeginPlay()
{
	Super::BeginPlay();

	//Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}

// Called every frame
void AGoKart::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	FVector Force = GetActorForwardVector() * MaxDrivingForce * Throttle;

	Force += GetAirResistance();
	Force += GetRollingResistance();

	FVector Acceleration = Force / Mass;

	Velocity = Velocity + Acceleration * DeltaTime;

	WG_VECTOR(Velocity);

	ApplyRotation(DeltaTime);
	UpdateLocationFromVelocity(DeltaTime);

	FString EnumString;
	UEnum::GetValueAsString(GetLocalRole(), EnumString);
	DrawDebugString(GetWorld(), FVector(0,0,100.f), EnumString, this, FColor::White, DeltaTime);
}

// Called to bind functionality to input
void AGoKart::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent)) {

		//Moving
		EnhancedInputComponent->BindAction(MoveForwardAction, ETriggerEvent::Triggered, this, &AGoKart::MoveForward);
		EnhancedInputComponent->BindAction(MoveForwardAction, ETriggerEvent::Completed, this, &AGoKart::StopMoveForward);
		EnhancedInputComponent->BindAction(MoveRightAction, ETriggerEvent::Triggered, this, &AGoKart::MoveRight);
		EnhancedInputComponent->BindAction(MoveRightAction, ETriggerEvent::Completed, this, &AGoKart::StopMoveRight);
	}
}

FVector AGoKart::GetAirResistance()
{
	return -Velocity.GetSafeNormal() * Velocity.SizeSquared() * DragCoefficient;
}

FVector AGoKart::GetRollingResistance()
{
	float AccelerationDueToGravity = -GetWorld()->GetGravityZ();
	float NormalForce = Mass * AccelerationDueToGravity;
	return -Velocity.GetSafeNormal() * RollingResistanceCoefficient * NormalForce;
}

void AGoKart::MoveForward(const FInputActionValue& Value)
{
	Throttle = Value.GetMagnitude();
	Server_MoveForward(Value);
}

void AGoKart::StopMoveForward(const FInputActionValue& Value)
{
	Throttle = 0.f;
	Server_StopMoveForward(Value);
}

void AGoKart::MoveRight(const FInputActionValue& Value)
{
	SteeringThrow = Value.GetMagnitude();
	Server_MoveRight(Value);
}

void AGoKart::StopMoveRight(const FInputActionValue& Value)
{
	SteeringThrow = 0.f;
	Server_StopMoveRight(Value);
}

void AGoKart::Server_MoveForward_Implementation(const FInputActionValue& Value)
{
	Throttle = Value.GetMagnitude();
}

bool AGoKart::Server_MoveForward_Validate(const FInputActionValue& Value)
{
	return Value.GetMagnitude() <= 1 && Value.GetMagnitude() >= -1;
}

void AGoKart::Server_StopMoveForward_Implementation(const FInputActionValue& Value)
{
	Throttle = 0.f;
}

bool AGoKart::Server_StopMoveForward_Validate(const FInputActionValue& Value)
{
	return true;
}

void AGoKart::Server_MoveRight_Implementation(const FInputActionValue& Value)
{
	SteeringThrow = Value.GetMagnitude();
}

bool AGoKart::Server_MoveRight_Validate(const FInputActionValue& Value)
{
	return Value.GetMagnitude() <= 1 && Value.GetMagnitude() >= -1;
}

void AGoKart::Server_StopMoveRight_Implementation(const FInputActionValue& Value)
{
	SteeringThrow = 0.f;
}

bool AGoKart::Server_StopMoveRight_Validate(const FInputActionValue& Value)
{
	return true;
}

void AGoKart::UpdateLocationFromVelocity(float DeltaTime)
{
	FVector Translation = Velocity * 100 * DeltaTime;

	FHitResult Hit;
	AddActorWorldOffset(Translation, true, &Hit);
	if (Hit.IsValidBlockingHit())
	{
		Velocity = FVector::ZeroVector;
	}
}

void AGoKart::ApplyRotation(float DeltaTime)
{
	float DeltaLocation = FVector::DotProduct(GetActorForwardVector(), Velocity) * DeltaTime;
	float RotationAngle = DeltaLocation / MinTurningRadius * SteeringThrow;
	FQuat RotationDelta(GetActorUpVector(), RotationAngle);
	Velocity = RotationDelta.RotateVector(Velocity);

	AddActorWorldRotation(RotationDelta);
}
