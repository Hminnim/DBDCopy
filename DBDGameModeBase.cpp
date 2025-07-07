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
		// Choose player character class from GameInstance
		TSubclassOf<APawn> ChosenClass = GI->bIsKiller
			? KillerCharacterClass
			: SurvivorCharacterClass;

		// Spawn player character
		FTransform SpawnTransfrom = FindPlayerStart(NewPlayer)->GetActorTransform();

		APawn* NewPawn = GetWorld()->SpawnActor<APawn>(ChosenClass, SpawnTransfrom);
		if (NewPawn)
		{
			NewPlayer->Possess(NewPawn);
		}
	}
}

void ADBDGameModeBase::ChangeCharacter(APlayerController* PlayerController)
{
	UDBDGameInstance* GI = Cast<UDBDGameInstance>(GetGameInstance());
	if (GI)
	{
		// Choose player another character class from GameInstance
		TSubclassOf<APawn> NewClass = GI->bIsKiller
			? SurvivorCharacterClass
			: KillerCharacterClass;

		// Destroy old character
		if (APawn* OldCharacter = PlayerController->GetPawn())
		{
			OldCharacter->Destroy();
		}

		// Spawn new character
		FTransform SpawnTransform = FindPlayerStart(PlayerController)->GetActorTransform();

		APawn* NewPawn = GetWorld()->SpawnActor<APawn>(NewClass, SpawnTransform);
		if (NewPawn)
		{
			PlayerController->Possess(NewPawn);
		}

		GI->bIsKiller = !GI->bIsKiller;
	}

}
