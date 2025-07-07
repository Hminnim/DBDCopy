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

	virtual void PostLogin(APlayerController* NewPlayer) override;
	void ChangeCharacter(APlayerController* PlayerController);

protected:
	// Character class
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character class")
	TSubclassOf<APawn> KillerCharacterClass;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character class")
	TSubclassOf<APawn> SurvivorCharacterClass;

};
 