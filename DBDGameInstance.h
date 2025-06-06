// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "DBDGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class DBDCOPY_API UDBDGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Information")
	bool bIsKiller;
};
