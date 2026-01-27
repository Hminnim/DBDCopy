// Fill out your copyright notice in the Description page of Project Settings.


#include "DBDGameModeBase.h"
#include "DBDMainPlayerState.h"
#include "DBDGameInstance.h"
#include "DBDPlayerController.h"
#include "DBDGeneratorActor.h"
#include "DBDGateLeverSwitchActor.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "DBDObjectSpawnManager.h"
#include "GameFramework/PlayerStart.h"

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
		RemainSurvivors = ExpectedPlayers - 1;
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

AActor* ADBDGameModeBase::ChoosePlayerStart_Implementation(AController* Player)
{
	bool bIsKiller = false;

	ADBDMainPlayerState* PS = Player->GetPlayerState<ADBDMainPlayerState>();
	if (PS)
	{
		bIsKiller = PS->bIsKiller;
	}

	FName RequiredTag = FName("Survivor");
	if (bIsKiller)
	{
		RequiredTag = FName("Killer");
	}

	TArray<AActor*> AllPlayerStarts;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerStart::StaticClass(), AllPlayerStarts);

	TArray<AActor*> ValidPlayerStarts;
	for (AActor* Actor : AllPlayerStarts)
	{
		APlayerStart* StartPoint = Cast<APlayerStart>(Actor);

		// To separate the spawn point between survivor and killer
		if (StartPoint && StartPoint->PlayerStartTag == RequiredTag)
		{
			ValidPlayerStarts.Add(StartPoint);
		}
	}

	if (ValidPlayerStarts.Num() == 0)
	{
		return nullptr;
	}

	// Choose random spawn point
	int32 RandomIndex = FMath::RandRange(0, ValidPlayerStarts.Num() - 1);

	return ValidPlayerStarts[RandomIndex];
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

// For test play
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
	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, FString::Printf(TEXT("Generator Completed")));

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		ADBDPlayerController* MyPC = Cast<ADBDPlayerController>(It->Get());
		if (MyPC)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, FString::Printf(TEXT("Try ChangeRemainedGeneratorNum")));
			MyPC->Client_ChangeRemainedGeneratorNum(RemainGnerators);
		}
	}

	CheckAllGeneratorCompleted();
}

void ADBDGameModeBase::CheckAllGeneratorCompleted()
{
	if (RemainGnerators <= 0)
	{
		for (TActorIterator<ADBDGeneratorActor> It(GetWorld()); It; ++It)
		{
			ADBDGeneratorActor* FoundActor = *It;

			if (FoundActor)
			{
				FoundActor->OnCompletedRepair();
			}
		}

		// Change all lever can be opened
		TArray<AActor*> LeverActors;
		for (TActorIterator<ADBDGateLeverSwitchActor> It(GetWorld()); It; ++It)
		{
			ADBDGateLeverSwitchActor* FoundActor = *It;

			if (FoundActor)
			{
				FoundActor->BeCanOpen();
				LeverActors.Add(FoundActor);
			}
		}

		// Show lever's pop up to show location
		for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
		{
			if (ADBDPlayerController* PC = Cast<ADBDPlayerController>(It->Get()))
			{
				PC->Client_AllGeneratorCompleted(LeverActors);
			}
		}
	}
}

void ADBDGameModeBase::OnSurvivorDied(APlayerController* SurvivorController)
{
	RemainSurvivors--;

	ADBDPlayerController* PC = Cast<ADBDPlayerController>(SurvivorController);
	if (PC)
	{
		PC->Client_NotifyGameResult(false, 0);
	}

	CheckGameOver();
}

void ADBDGameModeBase::OnSurvivorEscaped(APlayerController* SurvivorController)
{
	RemainSurvivors--;

	ADBDPlayerController* PC = Cast<ADBDPlayerController>(SurvivorController);
	if (PC)
	{
		PC->Client_NotifyGameResult(true, 0);
	}

	CheckGameOver();
}

void ADBDGameModeBase::CheckGameOver()
{
	if (RemainSurvivors <= 0)
	{
		for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
		{
			if (ADBDPlayerController* PC = Cast<ADBDPlayerController>(It->Get()))
			{
				PC->Client_NotifyGameResult(false, ExpectedPlayers - RemainSurvivors - 1);
			}
		}
	}
}
