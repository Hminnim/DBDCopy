// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "DBDGeneratorActor.h"
#include "DBDGeneratorAnim.generated.h"

/**
 * 
 */
UCLASS()
class DBDCOPY_API UDBDGeneratorAnim : public UAnimInstance
{
	GENERATED_BODY()
	

public:
	// Function
	virtual void NativeBeginPlay() override;
	virtual void NativeUpdateAnimation(float DeltaSecond) override;

	UPROPERTY(BlueprintReadOnly)
	float CurrentRepairRate;

protected:
	UPROPERTY(BlueprintReadOnly)
	ADBDGeneratorActor* Generator;
};
