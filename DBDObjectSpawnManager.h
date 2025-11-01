// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/TargetPoint.h"
#include "DBDObjectSpawnManager.generated.h"

UCLASS()
class DBDCOPY_API ADBDObjectSpawnManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ADBDObjectSpawnManager();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pallet")
	TSubclassOf<AActor> PalletActorClass;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pallet")
	TArray<ATargetPoint*> PalletTargetPoints;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generator")
	TSubclassOf<AActor> GeneratorActorClass;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generator")
	TArray<ATargetPoint*> GeneratorTargetPoints;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hook")
	TSubclassOf<AActor> HookActorClass;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hook")
	TArray<ATargetPoint*> HookTargetPoints;

	// Functions
	void SpawnAllObjects();

private:
	// Spawn pallets
	void SpawnObjects(TSubclassOf<AActor> SpawnObjectClass, TArray<ATargetPoint*> SpawnTargetPoints, int SpawnCount);
};
