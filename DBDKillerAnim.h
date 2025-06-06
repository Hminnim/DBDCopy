// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "DBDKiller.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "DBDKillerAnim.generated.h"

class ADBDKiller;

/**
 * 
 */
UCLASS()
class DBDCOPY_API UDBDKillerAnim : public UAnimInstance
{
	GENERATED_BODY()
	
public:
	// Function
	virtual void NativeBeginPlay() override;
	virtual void NativeUpdateAnimation(float DeltaSecond) override;

	// Killer state values
	UPROPERTY(BlueprintReadWrite)
	float KillerSpeed;
	
protected:
	UPROPERTY(BlueprintReadWrite)
	ADBDKiller* Killer;

};
