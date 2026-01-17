// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DBDLobbyUserWidget.generated.h"

/**
 * 
 */
class UButton;
class UTextBlock;
class UScrollBox;

UCLASS()
class DBDCOPY_API UDBDLobbyUserWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime);

protected:
	UPROPERTY(meta = (BindWidget))
	UButton* ReadyStartButton;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* ReadyStartText;
	UPROPERTY(meta = (BindWidget))
	UButton* BackButton;
	UPROPERTY(meta = (BindWidget))
	UScrollBox* KillerScrollBox;
	UPROPERTY(meta = (BindWidget))
	UScrollBox* SurvivorScrollBox;

	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<class UUserWidget> UserSlotClass;

private:
	UFUNCTION()
	void OnReadyStartClicked();
	UFUNCTION()
	void OnBackClicked();

	void UpdateButtonState();
	void UpdatePlayerLists();

	float PlayerListUpdateTimeer = 0.0f;
};
