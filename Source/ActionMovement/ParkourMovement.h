// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ParkourMovement.generated.h"

class AActionMovementCharacter;
class UCharacterMovementComponent;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ACTIONMOVEMENT_API UParkourMovement : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UParkourMovement();
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	void WallrunJump();
	void WallrunEnd(float WallrunAgainTimerDelay);

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

private:
	UPROPERTY(VisibleAnywhere)
	AActionMovementCharacter* PlayerCharacter;
	UPROPERTY(VisibleAnywhere)
	UCharacterMovementComponent* PlayerMovementComponent;
	UPROPERTY(VisibleAnywhere)
	FRotator PlayerRotator;

	UPROPERTY(VisibleAnywhere)
	float DefaultGravity = 0.0f;
	UPROPERTY(EditDefaultsOnly)
	float RayCastLength = 75.0f;
	UPROPERTY(EditDefaultsOnly)
	float ForwardVectorBack = -35.0f;
	UPROPERTY(VisibleAnywhere)
	float WallrunDirection = -1.0f;
	UPROPERTY(EditAnywhere)
	float WallrunSpeed = 850.0f;
	UPROPERTY(EditAnywhere)
	float MinPerpendicularCheckRange = -0.52f;
	UPROPERTY(EditAnywhere)
	float MaxPerpendicularCheckRange = 0.52f;
	UPROPERTY(EditAnywhere)
	bool WallrunGravity = true;
	UPROPERTY(EditAnywhere)
	float WallrunTargetGravity = 2.0f;
	UPROPERTY(EditAnywhere)
	float SupressWallrunTimerDelay = 1.0f;
	UPROPERTY(EditAnywhere)
	float WallrunJumpTimerDelay = 0.35f;
	UPROPERTY(EditAnywhere)
	float WallrunJumpHeight = 400.0f;
	UPROPERTY(EditAnywhere)
	float WallrunJumpOffForce = 800.0f;

	UPROPERTY(VisibleAnywhere)
	bool OnWall = false;
	UPROPERTY(VisibleAnywhere)
	bool WallRunning = false;
	UPROPERTY(VisibleAnywhere)
	bool WallRunningRight = false;
	UPROPERTY(VisibleAnywhere)
	bool WallRunningLeft = false;
	UPROPERTY(VisibleAnywhere)
	bool WallrunSupressed = false;

	/*UPROPERTY(EditDefaultsOnly, Category = "Camera")
	float CameraXRoll;*/
	UPROPERTY(EditDefaultsOnly, Category = "Camera")
	float InterpolationSpeed = 10.0f;;

	FVector RightVector;
	FVector ForwardVector;
	FVector PlayerLocation;
	FVector RightRaycastLine;
	FVector LeftRaycastLine;
	FVector WallrunNormal;

	FTimerHandle WallrunSuppressHandle;
	
	void CalculateRaycastLines();
	bool WallrunMovement(bool bRightDirection);
	bool IsPerpendicular(FVector Normal) const;
	void InterpolateGravity();
	void SuppressWallrun(float Delay);
	void ResetWallrunSupress();
	void InterpCamRotation(float CameraRoll, float YAxisOffset,float ZAxisOffset);
	void CameraTilt();
};
