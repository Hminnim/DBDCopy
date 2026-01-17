// Fill out your copyright notice in the Description page of Project Settings.


#include "DBDCharacterChangeInGameActor.h"
#include "DBDSurvivor.h"
#include "DBDKiller.h"

// Sets default values
ADBDCharacterChangeInGameActor::ADBDCharacterChangeInGameActor()
{
	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	TriggerBox->SetupAttachment(RootComponent);
	TriggerBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TriggerBox->SetCollisionObjectType(ECC_WorldDynamic);
	TriggerBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	TriggerBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

}

// Called when the game starts or when spawned
void ADBDCharacterChangeInGameActor::BeginPlay()
{
	Super::BeginPlay();
	
	if (TriggerBox)
	{
		TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &ADBDCharacterChangeInGameActor::OnOverlapBegin);
		TriggerBox->OnComponentEndOverlap.AddDynamic(this, &ADBDCharacterChangeInGameActor::OnOverlapEnd);
	}
}

void ADBDCharacterChangeInGameActor::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	ADBDSurvivor* OverlappedSurvior = Cast<ADBDSurvivor>(OtherActor);
	if (OverlappedSurvior)
	{
		OverlappedSurvior->BeginOverlapCharacterChange();
	}

	ADBDKiller* OverlappedKiller = Cast<ADBDKiller>(OtherActor);
	if(OverlappedKiller)
	{
		OverlappedKiller->BeginOverlapCharacterChange();
	}
}

void ADBDCharacterChangeInGameActor::OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	ADBDSurvivor* OverlappedSurvior = Cast<ADBDSurvivor>(OtherActor);
	if (OverlappedSurvior)
	{
		OverlappedSurvior->EndOverlapCharacterChange();
	}

	ADBDKiller* OverlappedKiller = Cast<ADBDKiller>(OtherActor);
	if (OverlappedKiller)
	{
		OverlappedKiller->EndOverlapCharacterChange();
	}
}


