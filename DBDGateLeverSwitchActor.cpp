// Fill out your copyright notice in the Description page of Project Settings.


#include "DBDGateLeverSwitchActor.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Net/UnrealNetwork.h"
#include "DBDExitGateActor.h"

// Sets default values
ADBDGateLeverSwitchActor::ADBDGateLeverSwitchActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	// Lever static component
	LeverBodyStaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Lever Body Mesh"));
	LeverStaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Lever Mesh"));
	LeverBodyStaticMesh->SetupAttachment(RootComponent);
	LeverStaticMesh->SetupAttachment(LeverBodyStaticMesh);

	// Lever collision box component
	LeverCollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("Lever Collision Box"));
	LeverCollisionBox->SetupAttachment(LeverBodyStaticMesh);
	LeverCollisionBox->SetCollisionResponseToAllChannels(ECR_Block);
	
}

void ADBDGateLeverSwitchActor::OpenExitGate()
{
	if (!MyExitGateActor)
	{
		return;
	}

	bIsOpened = true;
	MyExitGateActor->StartOpen();
}

// Called when the game starts or when spawned
void ADBDGateLeverSwitchActor::BeginPlay()
{
	Super::BeginPlay();
	
}

void ADBDGateLeverSwitchActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ADBDGateLeverSwitchActor, CurrentLeverRate);
	DOREPLIFETIME(ADBDGateLeverSwitchActor, bIsOpened);
	DOREPLIFETIME(ADBDGateLeverSwitchActor, bCanOpen);
}

