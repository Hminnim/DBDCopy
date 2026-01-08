// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DBDExitGateActor.generated.h"

class UStaticMeshComponent;
class UBoxComponent;
class USceneComponent;

UCLASS()
class DBDCOPY_API ADBDExitGateActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ADBDExitGateActor();

	UPROPERTY(Replicated)
	bool bIsOpened = false;
	UPROPERTY(Replicated)
	bool bIsOpening = false;

	void StartOpen();
	void Opening(float DeltaTime);
	void StopOpen();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* GateSceneComponent;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* LeftDoorStaticMesh;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* RightDoorStaticMesh;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
	UBoxComponent* ExitOverlapBoxComponent;

	float ElapsedTime = 0.0f;
	float MoveDuration = 2.0f;

	FVector StartLeftLocation = FVector{ -200.0f,0.0f,0.0f };
	FVector StartRightLocation = FVector{ 200.0f,0.0f,0.0f };
	FVector	TargetLeftLocation = FVector{ -800.0f,0.0f,0.0f };
	FVector TargetRightLocation = FVector{ 800.0f,0.0f,0.0f };

private:
	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
		const FHitResult& SweepResult);
	UFUNCTION()
	void OnOverlapEnd(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex
	);
};
