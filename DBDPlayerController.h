// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Blueprint/UserWidget.h"
#include "DBDPlayUserWidget.h"
#include "DBDPlayerController.generated.h"

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

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UDBDPlayUserWidget> PlayUserWidgetClass;
	class UDBDPlayUserWidget* PlayUserWidget;
};
