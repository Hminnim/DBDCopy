// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Components/BoxComponent.h"
#include "DBDPalletActor.generated.h"

UCLASS()
class DBDCOPY_API ADBDPalletActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ADBDPalletActor();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	// Components
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* RootScene;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* PalletScene;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* PalletStaticMesh;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
	class UBoxComponent* TriggerBox;

	// Values
	bool bIsDropping = false;
	bool bIsDropped = false;
	FVector StartLocation[2];

	// Functions
	void StartDrop();
	void DropPallet(float DeltaTime);
	void EndDrop();

private:
	// Overlap
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

	// Drop pallet values
	float DropElapsedTime = 0.0f;
	float DropDuration = 0.2f;
	float DropStartPitch = 0.0f;
	float DropEndPitch = -70.0f;
};
