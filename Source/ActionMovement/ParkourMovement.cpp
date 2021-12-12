#include "ParkourMovement.h"
#include "ActionMovementCharacter.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/CharacterMovementComponent.h"


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
			DefaultGravity = PlayerMovementComponent->GetGravityZ();
		}
	}
	
}


// Called every frame
void UParkourMovement::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	CalculateRaycastLines();
	WallrunMovement();
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

void UParkourMovement::WallrunMovement()
{
	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(GetOwner());
	bool bLineTrace = GetWorld()->LineTraceSingleByChannel(Hit, PlayerLocation, RightRaycastLine, 
		ECollisionChannel::ECC_Visibility, Params);
	if (bLineTrace)
	{
		UE_LOG(LogTemp, Warning, TEXT("Hit.Normal Params: X: %f, Y: %f, Z: %f"), Hit.Normal.X, Hit.Normal.Y, Hit.Normal.Z);
		if (Hit.bBlockingHit && IsPerpendicular(Hit.Normal) && PlayerMovementComponent->IsFalling())
		{
			WallrunNormal = Hit.Normal;
			FVector DistanceToWall = WallrunNormal - PlayerLocation;
			float DistanceToWallSize = DistanceToWall.Size();
			FVector LaunchVector = Hit.Normal * DistanceToWallSize;
			UE_LOG(LogTemp, Warning, TEXT("Launch Vector Params: X: %f, Y: %f, Z: %f"), LaunchVector.X,
				LaunchVector.Y, LaunchVector.Z);
			PlayerCharacter->LaunchCharacter(-LaunchVector, false, false);

			//Launch Player forward
			FVector ForwardDirection = WallrunNormal.CrossProduct(WallrunNormal, { 0.0, 0.0, 1.0 });
			ForwardDirection = ForwardDirection * WallrunSpeed * WallrunDirection;
			UE_LOG(LogTemp, Warning, TEXT("Forward Direction Params: X: %f, Y: %f, Z: %f"), ForwardDirection.X,
				ForwardDirection.Y, ForwardDirection.Z);
			
			PlayerCharacter->LaunchCharacter(ForwardDirection, true, WallrunGravity);
		}
		DrawDebugLine(GetWorld(), PlayerLocation, RightRaycastLine, FColor::Red);
		DrawDebugLine(GetWorld(), PlayerLocation, LeftRaycastLine, FColor::Green);
	}
}

bool UParkourMovement::IsPerpendicular(FVector Normal) const
{
	if (Normal.Z < MinPerpendicularCheckRange || Normal.Z > MaxPerpendicularCheckRange)
	{
		return false;
	}
	return true;
}