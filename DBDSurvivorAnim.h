// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "DBDSurvivor.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "DBDSurvivorAnim.generated.h"

class ADBDSurvivor;

/**
 * 
 */
UCLASS()
class DBDCOPY_API UDBDSurvivorAnim : public UAnimInstance
{
	GENERATED_BODY()
	
public:
	// Function
	virtual void NativeBeginPlay() override;
	virtual void NativeUpdateAnimation(float DeltaSecond) override;

	// Survivor state values
	UPROPERTY(BlueprintReadWrite)
	float SurvivorSpeed;
	UPROPERTY(BlueprintReadWrite)
	bool bIsCrouched;
	UPROPERTY(BlueprintReadWrite)
	bool bIsInteracting;

protected:
	UPROPERTY(BlueprintReadWrite)
	ADBDSurvivor* Survivor;
};
