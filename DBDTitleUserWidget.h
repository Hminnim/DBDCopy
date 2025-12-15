// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "FindSessionsCallbackProxy.h"
#include "DBDTitleUserWidget.generated.h"

class UButton;
class UScrollBox;
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
	UPROPERTY(meta = (BindWidget))
	class UScrollBox* SessionListScrollBox;
	UPROPERTY(meta = (BindWidget))
	class UWidget* SessionListPopup;
	UPROPERTY(meta = (BindWidget))
	class UButton* BackButton;

	// Slot widget class
	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<class UDBDSessionSlotWidget> SessionSlotClass;

private:
	// Widget bind function
	UFUNCTION()
	void OnQuitButtonClicked();
	UFUNCTION()
	void OnKillerButtonClicked();
	UFUNCTION()
	void OnSurvivorButtonClicked();
	UFUNCTION()
	void OnBackButtonClicked();

	UFUNCTION()
	void OnFindSessionsComplete(const TArray<FBlueprintSessionResult>& SessionResults);
	UFUNCTION()
	void OnCreateSessionComplete(bool bSuccessful);
};
