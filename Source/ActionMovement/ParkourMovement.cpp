#include "ParkourMovement.h"
#include "ActionMovementCharacter.h"
#include "Camera/CameraComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"
#include "TimerManager.h"


#define OUT

// Sets default values for this component's properties
UParkourMovement::UParkourMovement()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
}


// Called when the game starts
void UParkourMovement::BeginPlay()
{
	Super::BeginPlay();
	PlayerCharacter = Cast<AActionMovementCharacter>(GetOwner());
	if (PlayerCharacter)
	{
		PlayerMovementComponent = PlayerCharacter->GetCharacterMovement();
		if (PlayerMovementComponent)
		{
			DefaultGravity = PlayerMovementComponent->GravityScale;
		}
	}
}


// Called every frame
void UParkourMovement::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	CalculateRaycastLines();
	if (WallrunMovement(true))
	{
		WallRunning = true;
		WallRunningLeft = false;
		WallRunningRight = true;
		InterpolateGravity();
	}
	else if (WallRunningRight)
	{
		WallrunEnd(SupressWallrunTimerDelay);
	}
	else if (WallrunMovement(false))
	{
		WallRunning = true;
		WallRunningLeft = true;
		WallRunningRight = false;
		InterpolateGravity();
	}
	else
	{
		WallrunEnd(SupressWallrunTimerDelay);
	}
	CameraTilt();
}

void UParkourMovement::CalculateRaycastLines()
{
	if (PlayerCharacter)
	{
		FVector RightVector = PlayerCharacter->GetActorRightVector();
		FVector ForwardVector = PlayerCharacter->GetActorForwardVector();
		PlayerLocation = PlayerCharacter->GetActorLocation();
		FVector RayCastEnd = RightVector * RayCastLength;
		ForwardVector *= ForwardVectorBack;
		RightRaycastLine = PlayerLocation + RayCastEnd + ForwardVector;
		//In order to get the Left side Raycast we multiply the right vector by the raycast length 
		//in the opposite direction so RightVector * RayCastLength * -1
		RayCastEnd = RightVector * -RayCastLength;
		LeftRaycastLine = PlayerLocation + RayCastEnd + ForwardVector;
	}
}

bool UParkourMovement::WallrunMovement(bool bRightDirection)
{
	FHitResult Hit;
	FCollisionQueryParams Params;
	FVector RayCastLine;
	Params.AddIgnoredActor(GetOwner());
	if (bRightDirection)
	{
		RayCastLine = RightRaycastLine;
		WallrunDirection = -1.0f;
	}
	else
	{
		RayCastLine = LeftRaycastLine;
		WallrunDirection = 1.0f;
	}
	bool bLineTrace = GetWorld()->LineTraceSingleByChannel(OUT Hit, PlayerLocation, RayCastLine,
		ECollisionChannel::ECC_Visibility, Params);
	if (bLineTrace && !WallrunSupressed)
	{
		if (Hit.bBlockingHit && IsPerpendicular(Hit.Normal) && PlayerMovementComponent->IsFalling())
		{
			WallrunNormal = Hit.Normal;
			LaunchPlayerIntoWall(PlayerLocation, WallrunNormal);
			LaunchPlayerForward(WallrunNormal, WallrunSpeed, WallrunDirection, WallrunGravity);
			return true;
		}
		DrawDebugLine(GetWorld(), PlayerLocation, RightRaycastLine, FColor::Red);
		DrawDebugLine(GetWorld(), PlayerLocation, LeftRaycastLine, FColor::Green);
		return false;
	}
	return false;
}

bool UParkourMovement::IsPerpendicular(FVector Normal) const
{
	if (Normal.Z < MinPerpendicularCheckRange || Normal.Z > MaxPerpendicularCheckRange)
	{
		return false;
	}
	return true;
}

void UParkourMovement::InterpolateGravity()
{
	float CurrentGravity = PlayerMovementComponent->GravityScale;
	float DeltaTime = UGameplayStatics::GetWorldDeltaSeconds(this);
	float InterpolatedGravity = FMath::FInterpConstantTo(CurrentGravity, WallrunTargetGravity, DeltaTime, 10.0);
	PlayerMovementComponent->GravityScale = InterpolatedGravity;
}

void UParkourMovement::WallrunEnd(float WallrunAgainTimerDelay)
{
	if (WallRunning)
	{
		WallRunning = false;
		WallRunningLeft = false;
		WallRunningRight = false;
		PlayerMovementComponent->GravityScale = DefaultGravity;
		//SuppressWallrun(WallrunAgainTimerDelay);
	}
}

void UParkourMovement::SuppressWallrun(float Delay)
{
	WallrunSupressed = true;
	PlayerCharacter->GetWorldTimerManager().SetTimer(WallrunSuppressHandle, this, &UParkourMovement::ResetWallrunSupress, 
		SupressWallrunTimerDelay, true);
}

void UParkourMovement::ResetWallrunSupress()
{
	PlayerCharacter->GetWorldTimerManager().ClearTimer(WallrunSuppressHandle);
	WallrunSupressed = false;
}

void UParkourMovement::InterpCameraRotation(float RollValue)
{
	UCameraComponent* PlayerCameraComponent = PlayerCharacter->FindComponentByClass<UCameraComponent>();
	if (!PlayerCameraComponent)
	{
		UE_LOG(LogTemp, Error, TEXT("The Player Camera Component is not initialized correclty!"));
		return;
	}
	PlayerRotator = PlayerCameraComponent->GetRelativeRotation();
	FRotator TargetRotation = { RollValue, PlayerRotator.Pitch, PlayerRotator.Yaw };
	float DeltaTime = UGameplayStatics::GetWorldDeltaSeconds(this);
	FRotator InterpolatedRotation = FMath::RInterpTo(PlayerRotator, TargetRotation, DeltaTime, InterpolationSpeed);
	PlayerCameraComponent->SetRelativeRotation(InterpolatedRotation);
}

void UParkourMovement::CameraTilt()
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

void UParkourMovement::WallrunJump()
{
	if (WallRunning)
	{
		WallrunEnd(WallrunJumpTimerDelay);
		FVector LaunchVector = { WallrunNormal.X * WallrunJumpOffForce, WallrunNormal.Y * WallrunJumpOffForce, 
			WallrunJumpHeight };
		PlayerCharacter->LaunchCharacter(LaunchVector, false, true);
	}
}

/// <summary>
/// This function gives the player character a feeling of "sticking to the wall" which is needed for wallrunning
/// </summary>
/// <param name="PlayerPosition">The location of the player character on which this component is placed</param>
/// <param name="Hit">A hit occuring from a ray cast against a wall and matching the necessary conditions</param>
void UParkourMovement::LaunchPlayerIntoWall(FVector PlayerPosition, FVector WallNormal)
{
	FVector DistanceToWall = WallrunNormal - PlayerLocation;
	float DistanceToWallSize = DistanceToWall.Size();
	FVector LaunchVector = WallNormal * DistanceToWallSize;
	if (PlayerCharacter)
	{
		PlayerCharacter->LaunchCharacter(-LaunchVector, false, false);
	}
}

/// <summary>
/// This function launches the player forward while stuck to the wall from the previous function, thus mimicking movement
/// while still running on the wall, hence, a wall run
/// </summary>
/// <param name="WallNormal">The normal of the wall being collided with, used here in a cross product with the Z axis unit
/// vector to get the forward direction of the jump</param>
/// <param name="WallRunSpeed">The speed at which the player is moving when wallrunning</param>
/// <param name="WallRunDirection">The direction of the wall run, -1.0 for right direction wall runs, 1 for left direction</param>
/// <param name="WallRunGravity">A boolean value that determines whether gravity should be ignored or not</param>

void UParkourMovement::LaunchPlayerForward(FVector WallNormal, float WallRunSpeed, float WallRunDirection, bool WallRunGravity)
{
	FVector ForwardDirection = FVector::CrossProduct(WallNormal, { 0.0, 0.0, 1.0 });
	ForwardDirection = ForwardDirection * WallRunSpeed * WallRunDirection;
	if (PlayerCharacter)
	{
		PlayerCharacter->LaunchCharacter(ForwardDirection, true, WallRunGravity);
	}
}

void UParkourMovement::InterpCameraOffset(float YAxisOffset, float ZAxisOffset)
{
	FVector CameraOffset;
	USpringArmComponent* PlayerSpringArm = PlayerCharacter->FindComponentByClass<USpringArmComponent>();
	if (!PlayerSpringArm)
	{
		UE_LOG(LogTemp, Error, TEXT("The Player Spring Arm Component is not initialized correctly!"));
		return;
	}
	CameraOffset = PlayerSpringArm->GetTargetOffset();
	float DeltaTime = UGameplayStatics::GetWorldDeltaSeconds(this);
	CameraOffset.Y = FMath::FInterpConstantTo(CameraOffset.Y, YAxisOffset, DeltaTime, 10.0);
	CameraOffset.Z = FMath::FInterpConstantTo(CameraOffset.Z, ZAxisOffset, DeltaTime, 10.0);
	PlayerSpringArm->SetTargetOffset(CameraOffset);
}