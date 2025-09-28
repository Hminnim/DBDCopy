// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "DBDPalletActor.h"
#include "DBDPalletAnim.generated.h"

/**
 * 
 */
UCLASS()
class DBDCOPY_API UDBDPalletAnim : public UAnimInstance
{
	GENERATED_BODY()
	
public:
	// Function
	virtual void NativeBeginPlay() override;
	virtual void NativeUpdateAnimation(float DeltaSecond) override;

	UPROPERTY(BlueprintReadOnly)
	bool bIsDropped;

protected:
	UPROPERTY(BlueprintReadOnly)
	ADBDPalletActor* Pallet;
};
