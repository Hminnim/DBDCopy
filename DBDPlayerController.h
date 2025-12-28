// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Blueprint/UserWidget.h"
#include "TimerManager.h"
#include "DBDPlayUserWidget.h"
#include "DBDSkillCheckUserWidget.h"
#include "DBDLoadingUserWidget.h"
#include "DBDGameModeBase.h"
#include "Kismet/GameplayStatics.h"
#include "DBDPlayerController.generated.h"

class USoundBase;

/**
 * 
 */
UCLASS()
class DBDCOPY_API ADBDPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	virtual void BeginPlay() override;

	// For Loading
	UFUNCTION(Server, Reliable)
	void Server_NotifyLoaded();
	UFUNCTION(Client, Reliable)
	void Client_StartGame(int32 GeneratorNum);

	// Interaction Message
	void ShowIneractionMessage(FString Message);
	void HideInteractionMessage();
	void ShowInteractionProgress(float Value);
	void HideInteractionProgress();
	// Action Message
	void ShowActionMessage(FString Message);
	void HideActionMessage();
	void ShowSkillCheck();
	void HideSkillCheck();

	// Skill Checks
	// Generator
	UFUNCTION()
	void StartGeneratorSkillCheck();
	void GeneratorSkillcheck();
	void StopGeneratorSkillCheck();
	int8 GetGeneratorSkillCheckResult();
	// Wiggle
	void StartWiggleSkillCheck();
	void WiggleSkillCheck();
	void StopWiggleSkillCheck();
	int8 GetWWiggleSkillCheckResult();
	bool GetWiggleSkillCheckMiss();
	// Struggle
	void StartStruggleSkillCheck(int8 Count);
	void StruggleSkillCheck();
	void StopStruggleSkillCheck();
	int8 GetStruggleSkillCheckResult();

	void ChangeRemainedGeneratorNum(int32 NewNum);

	// For Debug
	UFUNCTION(Server, Reliable)
	void CharacterChange(bool bIsKiller);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sound")
	USoundBase* AlertSound;

	bool bIsSkillChecking = false;
	bool bWasInZone = false;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UDBDPlayUserWidget> PlayUserWidgetClass;
	class UDBDPlayUserWidget* PlayUserWidget;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UDBDSkillCheckUserWidget> SkillCheckWidgetClass;
	class UDBDSkillCheckUserWidget* SkillCheckWidget;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UDBDLoadingUserWidget> LoadingWidgetClass;
	class UDBDLoadingUserWidget* LoadingWidget;

private:
	FTimerHandle SkillCheckTimerHandle;
};
