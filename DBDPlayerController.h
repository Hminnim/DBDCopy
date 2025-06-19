// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Blueprint/UserWidget.h"
#include "TimerManager.h"
#include "DBDPlayUserWidget.h"
#include "DBDSkillCheckUserWidget.h"
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

	void ShowIneractionMessage(FString Message);
	void HideInteractionMessage();
	void ShowInteractionProgress(float Value);
	void HideInteractionProgress();
	void ShowSkillCheck();
	UFUNCTION()
	void StartSkillCheck();
	void HideSkillCheck();
	void StopSkillCheck();
	int8 GetSkillCheckResult();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sound")
	USoundBase* AlertSound;

	bool bIsSkillChecking = false;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UDBDPlayUserWidget> PlayUserWidgetClass;
	class UDBDPlayUserWidget* PlayUserWidget;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UDBDSkillCheckUserWidget> SkillCheckWidgetClass;
	class UDBDSkillCheckUserWidget* SkillCheckWidget;

private:
	FTimerHandle SkillCheckTimerHandle;
};
