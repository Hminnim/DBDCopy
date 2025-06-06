// Fill out your copyright notice in the Description page of Project Settings.


#include "DBDGameModeBase.h"

ADBDGameModeBase::ADBDGameModeBase()
{
}

void ADBDGameModeBase::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	UDBDGameInstance* GI = Cast<UDBDGameInstance>(GetGameInstance());
	if (GI)
	{
		TSubclassOf<APawn> ChosenClass = GI->bIsKiller
			? KillerCharacterClass
			: SurvivorCharacterClass;

		FTransform SpawnTransfrom = FindPlayerStart(NewPlayer)->GetActorTransform();

		APawn* NewPawn = GetWorld()->SpawnActor<APawn>(ChosenClass, SpawnTransfrom);
		if (NewPawn)
		{
			NewPlayer->Possess(NewPawn);
		}
	}
}
