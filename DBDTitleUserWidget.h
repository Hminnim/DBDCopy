// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "DBDTitlePlayerController.h"
#include "DBDTitleUserWidget.generated.h"

/**
 * 
 */
UCLASS()
class DBDCOPY_API UDBDTitleUserWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	virtual bool Initialize() override;

protected:
	// Bind widget
	UPROPERTY(meta = (BindWidget))
	class UButton* QuitButton;
	UPROPERTY(meta = (BindWidget))
	class UButton* KillerButton;
	UPROPERTY(meta = (BindWidget))
	class UButton* SurvivorButton;

private:
	// Widget bind function
	UFUNCTION()
	void OnQuitButtonClicked();
	UFUNCTION()
	void OnKillerButtonClicked();
	UFUNCTION()
	void OnSurvivorButtonClicked();
};
