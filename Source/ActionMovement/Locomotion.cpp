// Fill out your copyright notice in the Description page of Project Settings.


#include "Locomotion.h"
#include "ActionMovementCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"


// Sets default values for this component's properties
ULocomotion::ULocomotion()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

}


// Called when the game starts
void ULocomotion::BeginPlay()
{
	Super::BeginPlay();
	PlayerCharacter = Cast<AActionMovementCharacter>(GetOwner());
	if (!PlayerCharacter)
	{
		UE_LOG(LogTemp, Error, TEXT("The Locomotion Component is not attached to a Player Character of the type ActionMovementCharacter"));
		return;
	}
	PlayerMovementComponent = PlayerCharacter->GetCharacterMovement();
	if (!PlayerMovementComponent)
	{
		UE_LOG(LogTemp, Error, TEXT("The ActionMovementCharacter does not have a Movement Component attached to it!"));
	}
	DefaultGravity = PlayerMovementComponent->GravityScale;
}


// Called every frame
void ULocomotion::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	Main();
}

void ULocomotion::WallrunJump()
{
	if (HorizontalWallRunning)
	{
		PlayerCharacter->LaunchCharacter(LaunchIntoWall, false, true);
		WallrunEnd(WallrunJumpTimerDelay);
		FVector LaunchVector = { Hit.Normal.X * WallrunJumpOffForce, Hit.Normal.Y * WallrunJumpOffForce,
			WallrunJumpHeight };
		PlayerCharacter->LaunchCharacter(LaunchVector, false, true);
	}
}

void ULocomotion::WallrunEnd(float WallrunAgainTimerDelay)
{
	if (HorizontalWallRunning)
	{
		HorizontalWallRunning = false;
		WallRunningLeft = false;
		WallRunningRight = false;
		PlayerMovementComponent->GravityScale = DefaultGravity;
		//SuppressWallrun(WallrunAgainTimerDelay);
	}
	if (VerticalWallRunning)
	{
		VerticalWallRunning = false;
		PlayerMovementComponent->GravityScale = DefaultGravity;
		//SuppressWallrun(WallrunAgainTimerDelay);
	}
}

void ULocomotion::Main()
{
	GetRaycastLines();
	HorizontalWallrunMainLoop();
	VerticalWallrunMainLoop();
}

void ULocomotion::HorizontalWallrunMainLoop()
{
	if (CheckCollision(true, RightRaycastLine))
	{
		HorizontalWallRunning = true;
		WallRunningLeft = false;
		WallRunningRight = true;
		HorizontalWallrun(Hit);
		InterpolateGravity();
	}
	else if (WallRunningRight)
	{
		WallrunEnd(SupressWallrunTimerDelay);
	}
	else if (CheckCollision(false, LeftRaycastLine))
	{
		HorizontalWallRunning = true;
		WallRunningLeft = true;
		WallRunningRight = false;
		HorizontalWallrun(Hit);
		InterpolateGravity();
	}
	else
	{
		WallrunEnd(SupressWallrunTimerDelay);
	}
	CameraTilt();
}

void ULocomotion::VerticalWallrunMainLoop()
{
	if (CheckCollision(false, VWRMiddleRaycast))
	{
		VerticalWallRunning = true;
		VerticalWallrun(Hit);
		InterpolateGravity();
	}
	else if (CheckCollision(false, VWRMiddleRaycast))
	{
		VerticalWallRunning = true;
		VerticalWallrun(Hit);
		InterpolateGravity();
	}
}

void ULocomotion::GetRaycastLines()
{
	if (!PlayerCharacter)
	{
		UE_LOG(LogTemp, Error, TEXT("The ActionMovementCharacter component is not found by the Locomotion class"));
		return;
	}
	FVector RightVector = PlayerCharacter->GetActorRightVector();
	FVector ForwardVector = PlayerCharacter->GetActorForwardVector();
	PlayerLocation = PlayerCharacter->GetActorLocation();
	
	GetHorizontalWallRunRayCast(ForwardVector, RightVector);
	GetVerticalWallRunRayCast(ForwardVector, RightVector);
}

void ULocomotion::GetHorizontalWallRunRayCast(FVector ForwardVector, FVector RightVector)
{
	FVector RayCastEnd = RightVector * HorizontalRayCastScope;
	ForwardVector *= ForwardVectorBack;
	RightRaycastLine = PlayerLocation + RayCastEnd + ForwardVector;
	//In order to get the Left side Raycast we multiply the right vector by the raycast length 
	//in the opposite direction so RightVector * RayCastLength * -1
	RayCastEnd = RightVector * -HorizontalRayCastScope;
	LeftRaycastLine = PlayerLocation + RayCastEnd + ForwardVector;
}

void ULocomotion::GetVerticalWallRunRayCast(FVector ForwardVector, FVector RightVector)
{
	FVector RayCastEnd = RightVector * VerticalRayCastScope;
	ForwardVector *= VWRVectorRange;
	VWRRightRaycast = PlayerLocation + RayCastEnd + ForwardVector;
	VWRMiddleRaycast = PlayerLocation + ForwardVector;
	RayCastEnd = RightVector * -VerticalRayCastScope;
	VWRLeftRaycast = PlayerLocation + RayCastEnd + ForwardVector;
}

bool ULocomotion::IsPerpendicular(const FVector Normal) const
{
	if (Normal.Z < MinPerpendicularCheckRange || Normal.Z > MaxPerpendicularCheckRange)
	{
		return false;
	}
	return true;
}

bool ULocomotion::CheckCollision(bool bRightDirection, FVector RayCastLine)
{
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(GetOwner());
	if (bRightDirection)
	{
		WallrunDirection = -1.0f;
	}
	else
	{
		WallrunDirection = 1.0f;
	}
	return GetWorld()->LineTraceSingleByChannel(OUT Hit, PlayerLocation, RayCastLine, ECollisionChannel::ECC_Visibility, 
		Params);
}

void ULocomotion::HorizontalWallrun(const FHitResult HitResult)
{
	if (HitResult.bBlockingHit && IsPerpendicular(HitResult.Normal) && PlayerMovementComponent->IsFalling())
	{
		LaunchPlayerIntoWall(PlayerLocation, HitResult.Normal);
		LaunchPlayer(HitResult.Normal, FVector::ZAxisVector, WallrunSpeed, WallrunDirection, true);
	}
}

void ULocomotion::VerticalWallrun(const FHitResult HitResult)
{
	if (HitResult.bBlockingHit && IsPerpendicular(HitResult.Normal) && PlayerMovementComponent->IsFalling())
	{
		LaunchPlayerIntoWall(PlayerLocation, HitResult.Normal);
		//UE_LOG(LogTemp, Error, TEXT("The Hit Result Normal is : X: %f, Y: %f, Z: %f"), HitResult.Normal.X, HitResult.Normal.Y, HitResult.Normal.Z);
		//TODO: Think about refactoring this cuz you're passing the absolute value of the Y
		LaunchPlayer(-FVector::YAxisVector, FVector::XAxisVector, WallrunSpeed, WallrunDirection, true);
	}
}

void ULocomotion::LaunchPlayerIntoWall(FVector PlayerLocationVector, FVector WallNormal)
{
	FVector DistanceToWall = WallrunNormal - PlayerLocationVector;
	float DistanceToWallSize = DistanceToWall.Size();
	LaunchIntoWall = WallNormal * DistanceToWallSize;
	if (PlayerCharacter)
	{
		PlayerCharacter->LaunchCharacter(-LaunchIntoWall, false, false);
	}
}

void ULocomotion::LaunchPlayer(FVector WallNormal, FVector LaunchDirection, float WallRunSpeed, float WallRunDirection, bool WallRunGravity)
{
	FVector Direction = FVector::CrossProduct(WallNormal, LaunchDirection);
	Direction = Direction * WallRunSpeed * WallRunDirection;
	if (PlayerCharacter)
	{
		PlayerCharacter->LaunchCharacter(Direction, true, WallRunGravity);
	}
}

void ULocomotion::SuppressWallrun(float Delay)
{
	WallrunSupressed = true;
	PlayerCharacter->GetWorldTimerManager().SetTimer(WallrunSuppressHandle, this, &ULocomotion::ResetWallrunSupress,
		SupressWallrunTimerDelay, true);
}

void ULocomotion::ResetWallrunSupress()
{
	PlayerCharacter->GetWorldTimerManager().ClearTimer(WallrunSuppressHandle);
	WallrunSupressed = false;
}

void ULocomotion::InterpolateGravity()
{
	float CurrentGravity = PlayerMovementComponent->GravityScale;
	float DeltaTime = UGameplayStatics::GetWorldDeltaSeconds(this);
	float InterpolatedGravity = FMath::FInterpConstantTo(CurrentGravity, WallrunTargetGravity, DeltaTime, 10.0);
	PlayerMovementComponent->GravityScale = InterpolatedGravity;
}

void ULocomotion::CameraTilt()
{
	//TODO Parametrize these values and expose them in BP edits
	//EDIT: Parametrizing these values will lead to complications in the TickComponent where we'd need to set values based on boolean flag, left it as is for now
	if (WallRunningLeft)
	{
		//CameraXRoll = 15.0f;
		InterpCameraRotation(15.0f);
		InterpCameraOffset(-150.0f, -100.0f);
	}
	else if (WallRunningRight)
	{
		//CameraXRoll = -15.0f;
		InterpCameraRotation(-15.0f);
		InterpCameraOffset(150.0f, 100.0f);
	}
	else
	{
		//CameraXRoll = 0.0f;
		InterpCameraRotation(0.0f);
		InterpCameraOffset(0.0f, 0.0f);
	}
}

void ULocomotion::InterpCameraOffset(float YAxisOffset, float ZAxisOffset)
{
	USpringArmComponent* PlayerSpringArm = PlayerCharacter->FindComponentByClass<USpringArmComponent>();
	if (!PlayerSpringArm)
	{
		UE_LOG(LogTemp, Error, TEXT("The Player Spring Arm Component is not found by the Locomotion class!"));
		return;
	}
	FVector CameraOffset;
	CameraOffset = PlayerSpringArm->GetTargetOffset();
	float DeltaTime = UGameplayStatics::GetWorldDeltaSeconds(this);
	CameraOffset.Y = FMath::FInterpConstantTo(CameraOffset.Y, YAxisOffset, DeltaTime, 10.0);
	CameraOffset.Z = FMath::FInterpConstantTo(CameraOffset.Z, ZAxisOffset, DeltaTime, 10.0);
	PlayerSpringArm->SetTargetOffset(CameraOffset);
}

void ULocomotion::InterpCameraRotation(float CameraRoll)
{
	UCameraComponent* PlayerCameraComponent = PlayerCharacter->FindComponentByClass<UCameraComponent>();
	if (!PlayerCameraComponent)
	{
		UE_LOG(LogTemp, Error, TEXT("The Player Camera Component is not found by the Locomotion class!"));
		return;
	}
	PlayerRotator = PlayerCameraComponent->GetRelativeRotation();
	FRotator TargetRotation = { CameraRoll, PlayerRotator.Pitch, PlayerRotator.Yaw };
	float DeltaTime = UGameplayStatics::GetWorldDeltaSeconds(this);
	FRotator InterpolatedRotation = FMath::RInterpTo(PlayerRotator, TargetRotation, DeltaTime, InterpolationSpeed);
	PlayerCameraComponent->SetRelativeRotation(InterpolatedRotation);
}
