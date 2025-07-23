// Fill out your copyright notice in the Description page of Project Settings.


#include "DBDPalletActor.h"
#include "DBDSurvivor.h"
#include "Net/UnrealNetwork.h"

// Sets default values
ADBDPalletActor::ADBDPalletActor()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	bReplicates = true;

	// RootScene default values
	RootScene = CreateDefaultSubobject<USceneComponent>("Root Scene");
	RootScene->SetupAttachment(RootComponent);

	// PalletScene default values
	PalletScene = CreateDefaultSubobject<USceneComponent>("Pallet Scene");
	PalletScene->SetupAttachment(RootScene);
	PalletScene->SetIsReplicated(true);

	// PalletStaticMesh default values
	PalletStaticMesh = CreateDefaultSubobject<UStaticMeshComponent>("Pallet Static Mesh");
	PalletStaticMesh->SetupAttachment(PalletScene);
	static ConstructorHelpers::FObjectFinder<UStaticMesh>SMesh(TEXT("'/Game/Games/Object/Pallet/Wooden_Pallet_texcdfcda_Mid.Wooden_Pallet_texcdfcda_Mid'"));
	if (SMesh.Succeeded())
	{
		PalletStaticMesh->SetStaticMesh(SMesh.Object);
	}
	PalletStaticMesh->SetRelativeLocation(FVector({ -10.0f, 0.0f, 50.0f }));
	PalletStaticMesh->SetRelativeRotation(FRotator({ 90.0f, 360.0f,360.0f }));

	// TriggerBox default values
	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	TriggerBox->SetupAttachment(RootScene);
	TriggerBox->SetRelativeLocation(FVector({ 30.0f, 0.0f, 30.0f }));
	TriggerBox->SetRelativeScale3D(FVector({ 1.0f,1.75f,0.6f }));
	TriggerBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TriggerBox->SetCollisionObjectType(ECC_WorldDynamic);
	TriggerBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	TriggerBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	
}

void ADBDPalletActor::BeginPlay()
{
	Super::BeginPlay();

	if (TriggerBox)
	{
		TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &ADBDPalletActor::OnOverlapBegin);
		TriggerBox->OnComponentEndOverlap.AddDynamic(this, &ADBDPalletActor::OnOverlapEnd);

		// Set StartLocation from TriggerBox's relative location
		FVector OffsetLocation[2] = { FVector(-30.0f, -20.0f, 0.0f), FVector(-30.0f, 20.0f, 0.0f) };
		for (int i = 0; i < 2; i++)
		{
			FVector TargetLocation = TriggerBox->GetRelativeLocation() + OffsetLocation[i];
			StartLocation[i] = TriggerBox->GetComponentTransform().TransformPosition(TargetLocation);
		}
	}
}

void ADBDPalletActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	DropPallet(DeltaTime);
}

void ADBDPalletActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ADBDPalletActor, bIsDropped);
	DOREPLIFETIME(ADBDPalletActor, NewPalletRotation);
}

void ADBDPalletActor::StartDrop()
{
	bIsDropping = true;
}

void ADBDPalletActor::DropPallet(float DeltaTime)
{
	if (!bIsDropping || bIsDropped || !HasAuthority()) 
	{
		return;
	}

	if (DropElapsedTime < DropDuration)
	{
		DropElapsedTime += DeltaTime;

		float Alpha = DropElapsedTime / DropDuration;
		float NewPitch = FMath::Lerp(DropStartPitch, DropEndPitch, Alpha);

		NewPalletRotation = PalletScene->GetRelativeRotation();
		NewPalletRotation.Pitch = NewPitch;
		PalletScene->SetRelativeRotation(NewPalletRotation);
	}
}

void ADBDPalletActor::EndDrop()
{
	bIsDropping = false;
	bIsDropped = true;
}

void ADBDPalletActor::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (bIsDropping)
	{
		return;
	}

	ADBDSurvivor* OverlappedSurvivor = Cast<ADBDSurvivor>(OtherActor);
	if (!bIsDropped)
	{
		if (OverlappedSurvivor)
		{
			OverlappedSurvivor->BeginOverlapPallet();
			OverlappedSurvivor->SetCurrentPallet(this);
		}
	}
	else if (bIsDropped)
	{
		if (OverlappedSurvivor)
		{
			OverlappedSurvivor->BeginOverlapPalletVault();
			OverlappedSurvivor->SetCurrentPallet(this);
		}
	}
}

void ADBDPalletActor::OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	ADBDSurvivor* OverlappedSurvivor = Cast<ADBDSurvivor>(OtherActor);
	if (!bIsDropped)
	{
		if (OverlappedSurvivor)
		{
			OverlappedSurvivor->EndOverlapPallet();
		}
	}
	else if (bIsDropped)
	{
		if (OverlappedSurvivor)
		{
			OverlappedSurvivor->EndOverlapPalletVault();
		}
	}
}

void ADBDPalletActor::OnRep_ChangePalletRotation()
{
	PalletScene->SetRelativeRotation(NewPalletRotation);
}

