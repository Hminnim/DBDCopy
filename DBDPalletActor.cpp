// Fill out your copyright notice in the Description page of Project Settings.


#include "DBDPalletActor.h"
#include "DBDSurvivor.h"
#include "DBDKiller.h"
#include "Net/UnrealNetwork.h"

// Sets default values
ADBDPalletActor::ADBDPalletActor()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	bReplicates = true;

	// PalletMesh default values
	PalletMesh = CreateDefaultSubobject<USkeletalMeshComponent>("PalletMesh");
	PalletMesh->SetupAttachment(RootComponent);
	PalletMesh->SetCollisionProfileName("BlockAll");

	// TriggerBox default values
	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	TriggerBox->SetupAttachment(PalletMesh);
	TriggerBox->SetRelativeLocation(FVector({ 0.0f, 80.0f, 30.0f }));
	TriggerBox->SetRelativeScale3D(FVector({ 2.75f,1.75f,0.6f }));
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
		FVector OffsetLocation[2] = { FVector(30.0f, 0.0f, 0.0f), FVector(-30.0f, 0.0f, 0.0f) };
		for (int i = 0; i < 2; i++)
		{
			FVector TargetLocation = OffsetLocation[i];
			StartLocation[i] = TriggerBox->GetComponentTransform().TransformPosition(TargetLocation);
		}
	}

	if (PalletMesh)
	{
		PalletMesh->GetAnimInstance()->OnPlayMontageNotifyBegin.AddDynamic(this, &ADBDPalletActor::AnimNotifyBeginHandler);
	}
}

void ADBDPalletActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ADBDPalletActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ADBDPalletActor, bIsDropped);
}

void ADBDPalletActor::StartDrop()
{
	bIsDropping = true;
	
	if (DropAnim)
	{
		PalletMesh->GetAnimInstance()->Montage_Play(DropAnim);
	}

	// Find overlapped killer and make be stunned
	TArray<AActor*> OverlappedActors;
	this->GetOverlappingActors(OverlappedActors, ADBDKiller::StaticClass());

	if (OverlappedActors.Num() > 0)
	{
		for (AActor* OverlappedActor : OverlappedActors)
		{
			ADBDKiller* OverlappedKiller = Cast<ADBDKiller>(OverlappedActor);

			OverlappedKiller->BeStunned();
		}
	}
}

void ADBDPalletActor::EndDrop()
{
	bIsDropping = false;
	bIsDropped = true;
}

void ADBDPalletActor::StartBreak()
{
	if(BreakAnim)
	{
		PalletMesh->GetAnimInstance()->Montage_Play(BreakAnim);
	}
}

void ADBDPalletActor::EndBreak()
{
}

void ADBDPalletActor::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!bIsDropping)
	{
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
	else
	{
		ADBDKiller* OverlappedKiller = Cast<ADBDKiller>(OtherActor);
		if (OverlappedKiller)
		{
			OverlappedKiller->BeStunned();
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

void ADBDPalletActor::AnimNotifyBeginHandler(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointPayload)
{
	if (NotifyName == "EndDrop")
	{
		EndDrop();
	}
}

