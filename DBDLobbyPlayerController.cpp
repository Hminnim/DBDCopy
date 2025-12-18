// Fill out your copyright notice in the Description page of Project Settings.


#include "DBDLobbyPlayerController.h"
#include "DBDLobbyPlayerState.h"
#include "DBDLobbyGameModeBase.h"
#include "DBDSessionInstanceSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/UserWidget.h"
#include "DBDGameInstance.h"

void ADBDLobbyPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (IsLocalController())
	{
		UDBDGameInstance* GI = GetGameInstance<UDBDGameInstance>();
		if (GI)
		{
			bool bIsKiller = GI->bIsKiller;
			Server_SetIsKiller(bIsKiller);
		}

		if (LobbyUserWidgetClass)
		{
			LobbyUserWidget = CreateWidget<UUserWidget>(this, LobbyUserWidgetClass);
			if (LobbyUserWidget)
			{
				LobbyUserWidget->AddToViewport(0);
				SetInputMode(FInputModeUIOnly());
				bShowMouseCursor = true;
			}
		}
	}
}

void ADBDLobbyPlayerController::StartGame()
{
	if (!HasAuthority())
	{
		return;
	}

	ADBDLobbyGameModeBase* GM = GetWorld()->GetAuthGameMode<ADBDLobbyGameModeBase>();
	if (GM)
	{
		GM->StartGame();
	}
}

void ADBDLobbyPlayerController::ProcessLobbyDestroy()
{
	if (!HasAuthority())
	{
		return;
	}

	ADBDLobbyGameModeBase* GM = GetWorld()->GetAuthGameMode<ADBDLobbyGameModeBase>();
	if (GM)
	{
		GM->KickAllAndDestroySession();
	}
}

void ADBDLobbyPlayerController::LeaveLobby()
{
	UGameInstance* GI = GetGameInstance();
	if (GI)
	{
		UDBDSessionInstanceSubsystem* Subsystem = GI->GetSubsystem<UDBDSessionInstanceSubsystem>();
		if (Subsystem)
		{
			const IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();
			if (SessionInterface)
			{
				SessionInterface->DestroySession(NAME_GameSession);
			}
		}
	}

	UGameplayStatics::OpenLevel(this, FName("TitleMenu"));
}

void ADBDLobbyPlayerController::ToggleReadyState()
{
	ADBDLobbyPlayerState* LobbyPS = GetPlayerState<ADBDLobbyPlayerState>();
	if (LobbyPS)
	{
		Server_SetReady(!LobbyPS->bIsReady);
	}
}

void ADBDLobbyPlayerController::Client_ReturnToTitle_Implementation()
{
	LeaveLobby();
}

void ADBDLobbyPlayerController::Server_SetIsKiller_Implementation(bool bIsKiller)
{
	ADBDLobbyPlayerState* PS = GetPlayerState<ADBDLobbyPlayerState>();
	if (PS)
	{
		PS->bIsKiller = bIsKiller;
	}
}

void ADBDLobbyPlayerController::Server_SetReady_Implementation(bool bNewReady)
{
	ADBDLobbyPlayerState* PS = GetPlayerState<ADBDLobbyPlayerState>();
	if(PS)
	{
		PS->bIsReady = bNewReady;
	}

	ADBDLobbyGameModeBase* GM = GetWorld()->GetAuthGameMode<ADBDLobbyGameModeBase>();
	if (GM)
	{
		GM->CheckAllPlayersReady();
	}
}
