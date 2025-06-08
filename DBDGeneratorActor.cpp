// Fill out your copyright notice in the Description page of Project Settings.


#include "DBDGeneratorActor.h"

// Sets default values
ADBDGeneratorActor::ADBDGeneratorActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Static Mesh component
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetupAttachment(RootComponent);
	MeshComponent->SetRelativeLocation(FVector(100.f, 0.f, 0.f));


	static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshAsset(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (MeshAsset.Succeeded())
	{
		MeshComponent->SetStaticMesh(MeshAsset.Object);
	}

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> MaterialAsset(TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	if (MaterialAsset.Succeeded())
	{
		MeshComponent->SetMaterial(0, MaterialAsset.Object);
	}
}

// Called when the game starts or when spawned
void ADBDGeneratorActor::BeginPlay()
{
	Super::BeginPlay();
	
	// Set RepairLocation
	FName SocketName[4] = { "Front", "Right", "Left", "Back" };
	for (int32 i = 0; i < 4; i++)
	{
		if (MeshComponent->DoesSocketExist(SocketName[i]))
		{
			FVector SocketLocation = MeshComponent->GetSocketLocation(SocketName[i]);
			RepairLocation[i] = SocketLocation;
		}
	}
}

void ADBDGeneratorActor::Repairing(float DeltaTime)
{
	if (bIsRepaired)
	{
		return;
	}

	CurrentRepairRate += RateSpeed[CurrentRepairingSurvivor] * DeltaTime;

	if (CurrentRepairRate >= 100.0f)
	{
		bIsRepaired = true;
	}
}

// Called every frame
void ADBDGeneratorActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	Repairing(DeltaTime);
}

