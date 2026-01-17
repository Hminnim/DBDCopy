// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "FindSessionsCallbackProxy.h"
#include "DBDTitleUserWidget.generated.h"

class UButton;
class UScrollBox;
class UCircularThrobber;
/**
 * 
 */
UCLASS()
class DBDCOPY_API UDBDTitleUserWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	virtual bool Initialize() override;
	virtual void NativeConstruct() override;

protected:
	// Bind widget
	UPROPERTY(meta = (BindWidget))
	class UButton* QuitButton;
	UPROPERTY(meta = (BindWidget))
	UButton* KillerButton;
	UPROPERTY(meta = (BindWidget))
	UButton* SurvivorButton;
	UPROPERTY(meta = (BindWidget))
	UScrollBox* SessionListScrollBox;
	UPROPERTY(meta = (BindWidget))
	class UWidget* SessionListPopup;
	UPROPERTY(meta = (BindWidget))
	UButton* BackButton;
	UPROPERTY(meta = (BindWidget))
	UButton* RefreshButton;
	UPROPERTY(meta = (BindWidget))
	UCircularThrobber* LoadingThrobber;

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
	void OnRefreshButtonClicked();

	UFUNCTION()
	void OnFindSessionsComplete(const TArray<FBlueprintSessionResult>& SessionResults);
	UFUNCTION()
	void OnCreateSessionComplete(bool bSuccessful);
};
