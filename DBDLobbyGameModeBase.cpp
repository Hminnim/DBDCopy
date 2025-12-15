// Fill out your copyright notice in the Description page of Project Settings.


#include "DBDLobbyGameModeBase.h"
#include "DBDLobbyPlayerState.h"
#include "DBDLobbyPlayerController.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

ADBDLobbyGameModeBase::ADBDLobbyGameModeBase()
{
	bUseSeamlessTravel = true;
}

void ADBDLobbyGameModeBase::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
}

void ADBDLobbyGameModeBase::Logout(AController* Exiting)
{
	Super::Logout(Exiting);
}

void ADBDLobbyGameModeBase::CheckAllPlayersReady()
{
	if (bIsGameStarting)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	if(GetNumPlayers() < 2)
	{
		return;
	}

	bool bAllReady = true;

	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();
		if (PC)
		{
			ADBDLobbyPlayerState* PS = PC->GetPlayerState<ADBDLobbyPlayerState>();
			if (!PS)
			{
				bAllReady = false;
				break;
			}
			else
			{
				if (!PS->bIsReady)
				{
					bAllReady = false;
					break;
				}
			}
		}
	}

	if (bAllReady)
	{
		bCanGameStart = true;
	}
}

void ADBDLobbyGameModeBase::StartGame()
{
	bIsGameStarting = true;

	FTimerHandle TimerHandle;
	UWorld* World = GetWorld();
	if (World)
	{
		World->GetTimerManager().SetTimer(
			TimerHandle,
			this,
			&ADBDLobbyGameModeBase::StartServerTravel,
			GameStartingCountdownTime,
			false
		);
	}
}

void ADBDLobbyGameModeBase::KickAllAndDestroySession()
{
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		ADBDLobbyPlayerController* PC = Cast<ADBDLobbyPlayerController>(It->Get());
		if (PC && !PC->IsLocalController())
		{
			PC->Client_ReturnToTitle();
		}
	}

	UGameplayStatics::OpenLevel(this, FName("TitleMenu"));
}

void ADBDLobbyGameModeBase::StartServerTravel()
{
	UWorld* World = GetWorld();
	if (World)
	{
		FString URL = GameMapPath + TEXT("?listen");
		World->ServerTravel(URL);
	}
}
