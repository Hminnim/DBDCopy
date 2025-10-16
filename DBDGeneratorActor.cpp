// Fill out your copyright notice in the Description page of Project Settings.


#include "DBDGeneratorActor.h"
#include "Net/UnrealNetwork.h"

// Sets default values
ADBDGeneratorActor::ADBDGeneratorActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	bReplicates = true;

	// Skeletal component
	GeneratorMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Generator Mesh"));
	GeneratorMesh->SetupAttachment(RootComponent);

	// Collision box
	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("Collision Box"));
	CollisionBox->SetupAttachment(GeneratorMesh);
	CollisionBox->SetCollisionResponseToAllChannels(ECR_Block);

}

// Called when the game starts or when spawned
void ADBDGeneratorActor::BeginPlay()
{
	Super::BeginPlay();
	
	// Set RepairLocation
	FName SocketName[4] = { "Front", "Right", "Left", "Back" };
	for (int32 i = 0; i < 4; i++)
	{
		if (GeneratorMesh->DoesSocketExist(SocketName[i]))
		{
			FVector SocketLocation = GeneratorMesh->GetSocketLocation(SocketName[i]);
			RepairLocation[i] = SocketLocation;
		}
	}
}

void ADBDGeneratorActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ADBDGeneratorActor, CurrentRepairRate);
	DOREPLIFETIME(ADBDGeneratorActor, CurrentRepairingSurvivor);
	DOREPLIFETIME(ADBDGeneratorActor, bIsRepaired);
}

// Called every frame
void ADBDGeneratorActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

