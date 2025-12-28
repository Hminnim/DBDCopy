// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DBDPlayUserWidget.generated.h"

/**
 * 
 */
class UProgressBar;
class UTextBlock;
class UVerticalBox;

UCLASS()
class DBDCOPY_API UDBDPlayUserWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	virtual void NativeConstruct() override;

	void ShowInteractionMessage(FString Message);
	void HideInteractionMessge();
	void ShowInteractionProgress(float Value);
	void HideInteractionProgress();
	void ShowActionMessage(FString Message);
	void HideActionMessage();
	void SetPlayerList();
	void SetReaminGenerator(int32 GeneratorNum);

protected:
	// Bind widget
	UPROPERTY(meta = (BindWidget))
	UProgressBar* InteractionProgressBar;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* InteractionTextBlock;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* ActionTextBlock;
	UPROPERTY(meta = (BindWidget))
	UVerticalBox* PlayersVerticalBox;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* RemainGenerator;

	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<class UUserWidget> PlayerStateWidgetClass;

private:
};
