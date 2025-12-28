// Fill out your copyright notice in the Description page of Project Settings.


#include "DBDGameModeBase.h"
#include "DBDMainPlayerState.h"
#include "DBDGameInstance.h"
#include "DBDPlayerController.h"
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

void ADBDGameModeBase::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);


	UDBDGameInstance* GI = GetGameInstance<UDBDGameInstance>();
	if (GI)
	{
		ExpectedPlayers = GI->PlayersCount;
		RemainGnerators = GoalGeneratorNums[ExpectedPlayers];
	}
}

void ADBDGameModeBase::HandlePlayerLoaded(APlayerController* PC)
{
	CurrentLoadedPlayers++;

	if (CurrentLoadedPlayers >= ExpectedPlayers)
	{
		for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
		{
			ADBDPlayerController* MyPC = Cast<ADBDPlayerController>(It->Get());
			if (MyPC)
			{
				MyPC->Client_StartGame(RemainGnerators);
			}
		}
	}
}

void ADBDGameModeBase::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer)
{
	Super::HandleStartingNewPlayer_Implementation(NewPlayer);

	ADBDMainPlayerState* PS = NewPlayer->GetPlayerState<ADBDMainPlayerState>();
	if (PS)
	{
		// Choose player character class from GameInstance
		TSubclassOf<APawn> ChosenClass = PS->bIsKiller
			? KillerCharacterClass
			: SurvivorCharacterClass;

		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, FString::Printf(TEXT("bIsKiller : %d"), PS->bIsKiller));

		// Spawn player characterz
		FTransform SpawnTransfrom = FindPlayerStart(NewPlayer)->GetActorTransform();

		APawn* NewPawn = GetWorld()->SpawnActor<APawn>(ChosenClass, SpawnTransfrom);
		if (NewPawn)
		{
			NewPlayer->Possess(NewPawn);
		}
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, FString::Printf(TEXT("Can't find GameInstance")));
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

void ADBDGameModeBase::OnGeneratorCompleted()
{
	RemainGnerators--;

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		ADBDPlayerController* MyPC = Cast<ADBDPlayerController>(It->Get());
		if (MyPC)
		{
			MyPC->ChangeRemainedGeneratorNum(RemainGnerators);
		}
	}
}
