// Fill out your copyright notice in the Description page of Project Settings.


#include "DBDWindowActor.h"
#include "DBDSurvivor.h"

// Sets default values
ADBDWindowActor::ADBDWindowActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	// Static mesh component
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetupAttachment(RootComponent);

	// Box component
	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	TriggerBox->SetupAttachment(MeshComponent);
	TriggerBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TriggerBox->SetCollisionObjectType(ECC_WorldDynamic);
	TriggerBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	TriggerBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &ADBDWindowActor::OnOverlapBegin);
	TriggerBox->OnComponentEndOverlap.AddDynamic(this, &ADBDWindowActor::OnOverlapEnd);
}

// Called when the game starts or when spawned
void ADBDWindowActor::BeginPlay()
{
	Super::BeginPlay();
	
	if (MeshComponent->DoesSocketExist(FName("Top")))
	{
		TopLocation = MeshComponent->GetSocketLocation(FName("Top"));
	}
	if (MeshComponent->DoesSocketExist(FName("Vault Front")))
	{
		StartLocation[0] = MeshComponent->GetSocketLocation(FName("Vault Front"));
	}
	if (MeshComponent->DoesSocketExist(FName("Vault Back")))
	{
		StartLocation[1] = MeshComponent->GetSocketLocation(FName("Vault Back"));
	}
}

void ADBDWindowActor::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	ADBDSurvivor* OverlappedPawn = Cast<ADBDSurvivor>(OtherActor);
	if (!OverlappedPawn)
	{
		return;
	}

	OverlappedPawn->BeginOverlapWindow();
	OverlappedPawn->SetCurrentWindow(this);
}

void ADBDWindowActor::OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	ADBDSurvivor* OverlappedPawn = Cast<ADBDSurvivor>(OtherActor);
	if (!OverlappedPawn)
	{
		return;
	}

	OverlappedPawn->EndOverlapWindow();
}
