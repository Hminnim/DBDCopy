// Fill out your copyright notice in the Description page of Project Settings.


#include "DBDGameModeBase.h"
#include "Kismet/GameplayStatics.h"
#include "DBDObjectSpawnManager.h"

ADBDGameModeBase::ADBDGameModeBase()
{
}

void ADBDGameModeBase::BeginPlay()
{
	Super::BeginPlay();

	ADBDObjectSpawnManager* ObjectSpawner = Cast<ADBDObjectSpawnManager>(UGameplayStatics::GetActorOfClass(GetWorld(), ADBDObjectSpawnManager::StaticClass()));

	if (ObjectSpawner)
	{
		ObjectSpawner->SpawnAllObjects();
	}
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

void ADBDGameModeBase::ChangeCharacter(APlayerController* PlayerController, bool bIsKiller)
{
	// Choose player another character class from GameInstance
	TSubclassOf<APawn> NewClass = bIsKiller
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
}
