// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DBDGameOverUserWidget.generated.h"

/**
 * 
 */
class UTextBlock;
class UButton;

UCLASS()
class DBDCOPY_API UDBDGameOverUserWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	virtual void NativeConstruct() override;
	void KillerGameOver(int32 KilledSurvivor);
	void SurvivorGameOver(bool bEscaped);
	UFUNCTION()
	void OnTitleButtonClicked();

protected:
	UPROPERTY(meta = (BindWidget))
	UTextBlock* GameOverTextBlock;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* KilledSurvivorTextBlock;
	UPROPERTY(meta = (BindWidget))
	UButton* TitleButton;
};
