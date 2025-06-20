// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "DBDPlayUserWidget.generated.h"

/**
 * 
 */
UCLASS()
class DBDCOPY_API UDBDPlayUserWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	virtual bool Initialize() override;

	void ShowInteractionMessage(FString Message);
	void HideInteractionMessge();
	void ShowInteractionProgress(float Value);
	void HideInteractionProgress();
	void ShowActionMessage(FString Message);
	void HideActionMessage();

protected:
	// Bind widget
	UPROPERTY(meta = (BindWidget))
	class UProgressBar* InteractionProgressBar;
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* InteractionTextBlock;
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* ActionTextBlock;
};
