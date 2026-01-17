// Fill out your copyright notice in the Description page of Project Settings.


#include "DBDObjectSpawnManager.h"

// Sets default values
ADBDObjectSpawnManager::ADBDObjectSpawnManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

}

void ADBDObjectSpawnManager::SpawnAllObjects()
{
	if (PalletActorClass && PalletTargetPoints.Num() > 0)
	{
		SpawnObjects(PalletActorClass, PalletTargetPoints, 10);
	}
	if (GeneratorActorClass && GeneratorTargetPoints.Num() > 0)
	{
		SpawnObjects(GeneratorActorClass, GeneratorTargetPoints, 7);
	}
	if (HookActorClass && HookTargetPoints.Num() > 0)
	{
		SpawnObjects(HookActorClass, HookTargetPoints, 7);
	}
}

void ADBDObjectSpawnManager::SpawnObjects(TSubclassOf<AActor> SpawnObjectClass, TArray<ATargetPoint*> SpawnTargetPoints, int SpawnCount)
{
	TArray<ATargetPoint*> ShuffledPoints = SpawnTargetPoints;
	for (int32 i = 0; i < ShuffledPoints.Num(); i++) {
		int32 RandIndex = FMath::RandRange(i, ShuffledPoints.Num() - 1);
		if (i != RandIndex)
		{
			ShuffledPoints.Swap(i, RandIndex);
		}
	}

	for (int32 i = 0; i < SpawnCount; i++) 
	{
		if (ShuffledPoints[i])
		{
			FVector SpawnLocation = ShuffledPoints[i]->GetActorLocation();
			FRotator SpawnRotation = ShuffledPoints[i]->GetActorRotation();

			GetWorld()->SpawnActor<AActor>(SpawnObjectClass, SpawnLocation, SpawnRotation);
		}
	}
}

