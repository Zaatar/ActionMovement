// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SpringArmComponent.h"
#include "ActionMovementCameraSpringArm.generated.h"

/**
 * 
 */
UCLASS()
class ACTIONMOVEMENT_API UActionMovementCameraSpringArm : public USpringArmComponent
{
	GENERATED_BODY()
public:
	inline FVector GetTargetOffset() const { return TargetOffset; }

	void SetTargetOffset(FVector Input) { TargetOffset = Input; }
};
