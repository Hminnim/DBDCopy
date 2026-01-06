// Fill out your copyright notice in the Description page of Project Settings.


#include "DBDExitGateActor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Components/SceneComponent.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values
ADBDExitGateActor::ADBDExitGateActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	// Gate Scene component
	GateSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Gate Scene Component"));
	SetRootComponent(GateSceneComponent);

	// Left Door Mesh
	LeftDoorStaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Left Door Mesh"));
	LeftDoorStaticMesh->SetupAttachment(GateSceneComponent);
	LeftDoorStaticMesh->SetRelativeLocation(StartLeftLocation);

	// Right Door Mesh
	RightDoorStaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Right Door Mesh"));
	RightDoorStaticMesh->SetupAttachment(GateSceneComponent);
	RightDoorStaticMesh->SetRelativeLocation(StartRightLocation);

	// Exit Box Component
	ExitOverlapBoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("ExitBoxComponent"));
	ExitOverlapBoxComponent->SetupAttachment(GateSceneComponent);
}

// Called when the game starts or when spawned
void ADBDExitGateActor::BeginPlay()
{
	Super::BeginPlay();
	
}

void ADBDExitGateActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ADBDExitGateActor, bIsOpening);
	DOREPLIFETIME(ADBDExitGateActor, bIsOpened);
}

void ADBDExitGateActor::StartOpen()
{
	bIsOpening = true;
}

void ADBDExitGateActor::Opening(float DeltaTime)
{
	if (!bIsOpening)
	{
		return;
	}

	ElapsedTime += DeltaTime;

	float Alpha = FMath::Clamp(ElapsedTime / MoveDuration, 0.0f, 1.0f);

	FVector NewLeftLocation = FMath::Lerp(StartLeftLocation, TargetLeftLocation, Alpha);
	FVector NewRightLocation = FMath::Lerp(StartRightLocation, TargetRightLocation, Alpha);

	LeftDoorStaticMesh->SetRelativeLocation(NewLeftLocation);
	RightDoorStaticMesh->SetRelativeLocation(NewRightLocation);

	if (Alpha >= 1.0f)
	{
		StopOpen();
	}
}

void ADBDExitGateActor::StopOpen()
{
	bIsOpening = false;
	bIsOpened = true;
	LeftDoorStaticMesh->SetRelativeLocation(TargetLeftLocation);
	RightDoorStaticMesh->SetRelativeLocation(TargetRightLocation);
}

// Called every frame
void ADBDExitGateActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	Opening(DeltaTime);
}

