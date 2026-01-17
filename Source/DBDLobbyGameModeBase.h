// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "DBDLobbyGameModeBase.generated.h"

/**
 * 
 */
UCLASS()
class DBDCOPY_API ADBDLobbyGameModeBase : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	ADBDLobbyGameModeBase();

	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;

	void CheckAllPlayersReady();
	void StartGame();
	void KickAllAndDestroySession();

	bool bCanGameStart = false;

protected:
	float GameStartingCountdownTime = 5.0f;
	bool bIsGameStarting = false;
	FString GameMapPath = TEXT("/Game/Games/Map/Main");

private:
	void StartServerTravel();
};
