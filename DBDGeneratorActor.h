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
	UPROPERTY(Replicated)
	float CurrentRepairRate = 0.0f;
	UPROPERTY(Replicated)
	int32 CurrentRepairingSurvivor = 0;
	UPROPERTY(Replicated)
	bool bIsRepaired = false;
	float RateSpeed[5] = { 0.0f, 1.25f, 2.125f, 2.625f, 2.75f };
	FVector RepairLocation[4];

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* MeshComponent;

};
