#include "ParkourMovement.h"
#include "ActionMovementCharacter.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/Actor.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
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
	else if (WallrunMovement(false))
	{
		WallRunning = true;
		WallRunningLeft = true;
		WallRunningRight = false;
		InterpolateGravity();
	}
	else
	{
		WallrunEnd();
		SuppressWallrun(SupressWallrunTimer);
	}
}

void UParkourMovement::CalculateRaycastLines()
{
	if (PlayerCharacter)
	{
		RightVector = PlayerCharacter->GetActorRightVector();
		ForwardVector = PlayerCharacter->GetActorForwardVector();
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
	bool bLineTrace = GetWorld()->LineTraceSingleByChannel(Hit, PlayerLocation, RayCastLine,
		ECollisionChannel::ECC_Visibility, Params);
	if (bLineTrace)
	{
		if (Hit.bBlockingHit && IsPerpendicular(Hit.Normal) && PlayerMovementComponent->IsFalling())
		{
			WallrunNormal = Hit.Normal;
			FVector DistanceToWall = WallrunNormal - PlayerLocation;
			float DistanceToWallSize = DistanceToWall.Size();
			FVector LaunchVector = Hit.Normal * DistanceToWallSize;
			PlayerCharacter->LaunchCharacter(-LaunchVector, false, false);


			//TO BE REFACTORED INTO ITS' OWN FUNCTION
			//Launch Player forward
			FVector ForwardDirection = WallrunNormal.CrossProduct(WallrunNormal, { 0.0, 0.0, 1.0 });
			ForwardDirection = ForwardDirection * WallrunSpeed * WallrunDirection;
			UE_LOG(LogTemp, Warning, TEXT("Forward Direction Params: X: %f, Y: %f, Z: %f"), ForwardDirection.X,
				ForwardDirection.Y, ForwardDirection.Z);
			
			PlayerCharacter->LaunchCharacter(ForwardDirection, true, WallrunGravity);
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
	UE_LOG(LogTemp, Warning, TEXT("Logging Updates in Gravity: %f"), PlayerMovementComponent->GravityScale);
}

void UParkourMovement::WallrunEnd()
{
	WallRunning = false;
	WallRunningLeft = false;
	WallRunningRight = false;
	PlayerMovementComponent->GravityScale = DefaultGravity;
}

void UParkourMovement::SuppressWallrun(float Delay)
{
	WallrunSupressed = true;
	UE_LOG(LogTemp, Error, TEXT("Wallrun Suppressed"));
	PlayerCharacter->GetWorldTimerManager().SetTimer(WallrunSuppressHandle, this, &UParkourMovement::ResetWallrunSupress, 
		SupressWallrunTimer, true);
}

void UParkourMovement::ResetWallrunSupress()
{
	WallrunSupressed = false;
	UE_LOG(LogTemp, Error, TEXT("Wallrun no longer suppressed"));
	PlayerCharacter->GetWorldTimerManager().ClearTimer(WallrunSuppressHandle);
}