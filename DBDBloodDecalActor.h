// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DecalComponent.h"
#include "DBDBloodDecalActor.generated.h"

UCLASS()
class DBDCOPY_API ADBDBloodDecalActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ADBDBloodDecalActor();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Decal")
	UDecalComponent* DecalComponent;
};
