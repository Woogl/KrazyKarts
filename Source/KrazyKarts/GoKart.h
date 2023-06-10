// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "InputActionValue.h"
#include "GoKart.generated.h"

UCLASS()
class KRAZYKARTS_API AGoKart : public APawn
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputMappingContext* DefaultMappingContext;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* MoveForwardAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* MoveRightAction;

public:
	// Sets default values for this pawn's properties
	AGoKart();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

private:
	// Mass of the car (kg)
	UPROPERTY(EditAnywhere)
	float Mass = 1000.f;

	// The force applied to the car when the throttle is fully down (N)
	UPROPERTY(EditAnywhere)
	float MaxDrivingForce = 100000.f;

	// Minimum radius of the car turning circle at full lock (m)
	UPROPERTY(EditAnywhere)
	float MinTurningRadius = 10.f;

	// Higher means more drag (kg/cm)
	UPROPERTY(EditAnywhere)
	float DragCoefficient = 180.f;

	// Higher means more rolling resistance (kg/cm)
	UPROPERTY(EditAnywhere)
	float RollingResistanceCoefficient = 0.015f;

	FVector GetAirResistance();
	FVector GetRollingResistance();

	void MoveForward(const FInputActionValue& Value);

	void StopMoveForward(const FInputActionValue& Value);

	void MoveRight(const FInputActionValue& Value);

	void StopMoveRight(const FInputActionValue& Value);

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_MoveForward(const FInputActionValue& Value);

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_StopMoveForward(const FInputActionValue& Value);
	
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_MoveRight(const FInputActionValue& Value);

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_StopMoveRight(const FInputActionValue& Value);

	void UpdateLocationFromVelocity(float DeltaTime);
	void ApplyRotation(float DeltaTime);

	FVector Velocity;

	float Throttle;

	float SteeringThrow;
};