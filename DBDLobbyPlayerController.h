// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "DBDLobbyPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class DBDCOPY_API ADBDLobbyPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	virtual void BeginPlay() override;

	// Server
	UFUNCTION(Server, Reliable)
	void Server_SetReady(bool bNewReady);
	UFUNCTION(Server, Reliable)
	void Server_SetIsKiller(bool bIsKiller);

	// For host
	void StartGame();
	void ProcessLobbyDestroy();

	// For client
	void LeaveLobby();
	void ToggleReadyState();
	UFUNCTION(Client, Reliable)
	void Client_ReturnToTitle();
};
