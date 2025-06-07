// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DBDGeneratorActor.generated.h"

UCLASS()
class DBDCOPY_API ADBDGeneratorActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ADBDGeneratorActor();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Generator value
	float CurrentRepairRate = 0.0f;
	int8 CurrentRepairingSurvivor = 0;
	float RateSpeed[5] = { 0.0f, 1.25f, 2.125f, 2.625f, 2.75f };

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
private:
	void Repairing(float DeltaTime);
};
