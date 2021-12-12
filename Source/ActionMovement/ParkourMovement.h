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

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	UPROPERTY(VisibleAnywhere)
	AActionMovementCharacter* PlayerCharacter;
	UPROPERTY(VisibleAnywhere)
	UCharacterMovementComponent* PlayerMovementComponent;
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

	FVector RightVector;
	FVector ForwardVector;
	FVector PlayerLocation;
	FVector RightRaycastLine;
	FVector LeftRaycastLine;
	FVector WallrunNormal;
	
	void CalculateRaycastLines();
	void WallrunMovement();
	bool IsPerpendicular(FVector Normal) const;
};
