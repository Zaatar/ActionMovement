// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "Locomotion.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ACTIONMOVEMENT_API ULocomotion : public USceneComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	ULocomotion();
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	void WallrunJump();
	void WallrunEnd(float WallrunAgainTimerDelay);
	UFUNCTION(BlueprintPure)
	bool GetIsWallrunning() const { return HorizontalWallRunning; }
	UFUNCTION(BlueprintPure)
	bool GetIsWallrunningRight() const { return WallRunningRight; }
	UFUNCTION(BlueprintPure)
	bool GetIsWallrunningLeft() const { return WallRunningLeft; }

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
private:
	UPROPERTY(VisibleAnywhere)
	class AActionMovementCharacter* PlayerCharacter;
	UPROPERTY(VisibleAnywhere)
	class UCharacterMovementComponent* PlayerMovementComponent;
	UPROPERTY(VisibleAnywhere)
	FRotator PlayerRotator;
	UPROPERTY(EditDefaultsOnly, Category = "Collision Check")
	float HorizontalRayCastScope = 75.0f;
	UPROPERTY(EditDefaultsOnly, Category = "Collision Check")
	float VerticalRayCastScope = 20.0f;
	UPROPERTY(EditDefaultsOnly, Category = "Collision Check")
	float ForwardVectorBack = -35.0f;
	UPROPERTY(EditAnywhere, Category = "Collision Check")
	float MinPerpendicularCheckRange = -0.52f;
	UPROPERTY(EditAnywhere, Category = "Collision Check")
	float MaxPerpendicularCheckRange = 0.52f;
	UPROPERTY(EditAnywhere, Category = "Movement")
	float WallrunTargetGravity = 2.0f;
	UPROPERTY(EditAnywhere, Category = "Movement")
	float SupressWallrunTimerDelay = 1.0f;
	UPROPERTY(EditAnywhere, Category = "Movement")
	float WallrunJumpTimerDelay = 0.35f;
	UPROPERTY(EditAnywhere, Category = "Movement")
	float WallrunJumpHeight = 400.0f;
	UPROPERTY(EditAnywhere, Category = "Movement")
	float WallrunJumpOffForce = 800.0f;
	UPROPERTY(VisibleAnywhere, Category = "Movement")
	float DefaultGravity = 0.0f;
	UPROPERTY(VisibleAnywhere, Category = "Movement")
	float WallrunDirection = -1.0f;
	UPROPERTY(EditAnywhere, Category = "Movement")
	float WallrunSpeed = 850.0f;
	UPROPERTY(EditAnywhere, Category = "Movement")
	bool WallrunGravity = true;

	/* Possible State Machine Section*/
	UPROPERTY(VisibleAnywhere, Category = "Movement")
	bool OnWall = false;
	UPROPERTY(VisibleAnywhere, Category = "Movement")
	bool HorizontalWallRunning = false;
	UPROPERTY(VisibleAnywhere, Category = "Movement")
	bool WallRunningRight = false;
	UPROPERTY(VisibleAnywhere, Category = "Movement")
	bool WallRunningLeft = false;
	UPROPERTY(VisibleAnywhere, Category = "Movement")
	bool VerticalWallRunning = false;
	UPROPERTY(VisibleAnywhere, Category = "Movement")
	bool WallrunSupressed = false;
	/* End of Possible State Machine Section*/

	/* Camera Section */
	UPROPERTY(EditDefaultsOnly, Category = "Camera")
	float InterpolationSpeed = 10.0f;
	/* End of Camera Section*/

	FVector PlayerLocation;
	FVector RightRaycastLine;
	FVector LeftRaycastLine;
	FVector WallrunNormal;
	FHitResult Hit;

	FTimerHandle WallrunSuppressHandle;

	// Vertical Wall Run Variables
	FVector VWRRightRaycast;
	FVector VWRMiddleRaycast;
	FVector VWRLeftRaycast;
	float VWRVectorRange = 100.0f;

	void Main();
	void GetRaycastLines();
	void GetHorizontalWallRunRayCast(FVector ForwardVector, FVector RightVector);
	void GetVerticalWallRunRayCast(FVector ForwardVector, FVector RightVector);
	bool IsPerpendicular(const FVector Normal) const;
	bool CheckCollision(bool bRightDirection, FVector RayCastLine);
	void HorizontalWallrun(const FHitResult Hit);
	void VerticalWallrun(const FHitResult Hit);
	void LaunchPlayerIntoWall(FVector PlayerLocation, FVector WallNormal);
	void LaunchPlayer(FVector WallNormal, FVector LaunchDirection, float WallRunSpeed, float WallRunDirection, bool WallRunGravity);
	void SuppressWallrun(float Delay);
	void ResetWallrunSupress();
	void InterpolateGravity();
	void CameraTilt();
	void InterpCameraOffset(float YAxisOffset, float ZAxisOffset);
	void InterpCameraRotation(float CameraRoll);
	//void LaunchPlayerForward(FVector WallNormal, float WallRunSpeed, float WallRunDirection, bool WallRunGravity);

	//void VWRCalculateRaycastLines();
	//bool VWRMovement(FVector RayCast);
	
};
