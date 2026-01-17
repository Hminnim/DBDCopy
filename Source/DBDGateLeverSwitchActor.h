// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DBDGateLeverSwitchActor.generated.h"

class UStaticMeshComponent;
class UBoxComponent;
class ADBDExitGateActor;

UCLASS()
class DBDCOPY_API ADBDGateLeverSwitchActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ADBDGateLeverSwitchActor();

	// Gate lever value
	UPROPERTY(Replicated)
	float CurrentLeverRate = 0.0f;
	UPROPERTY(Replicated)
	bool bIsOpened = false;
	UPROPERTY(Replicated)
	bool bCanOpen = false;

	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Exit Gate Actor")
	ADBDExitGateActor* MyExitGateActor;

	void OpenExitGate();
	void BeCanOpen();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* LeverBodyStaticMesh;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* LeverStaticMesh;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
	UBoxComponent* LeverCollisionBox;

};
