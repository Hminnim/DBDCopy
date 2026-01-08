// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "DBDGameInstance.h"
#include "DBDGameModeBase.generated.h"
/**
 * 
 */

UCLASS()
class DBDCOPY_API ADBDGameModeBase : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	ADBDGameModeBase();
	virtual void BeginPlay() override;
	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;

	void HandlePlayerLoaded(APlayerController* PC);
	virtual void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;
	void ChangeCharacter(APlayerController* PlayerController, bool bIsKiller);
	void OnGeneratorCompleted();
	void CheckAllGeneratorCompleted();
	void OnSurvivorDied(APlayerController* SurvivorController);
	void OnSurvivorEscaped(APlayerController* SurvivorController);
	void CheckGameOver();

	// Character class
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character class")
	TSubclassOf<APawn> KillerCharacterClass;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character class")
	TSubclassOf<APawn> SurvivorCharacterClass;

protected:
	int32 ExpectedPlayers = 0;
	int32 CurrentLoadedPlayers = 0;
	const TArray<int32> GoalGeneratorNums = { 1,1,2,3,4,5 };
	int32 RemainGnerators = 0;
	int32 RemainSurvivors = 0;
};
 