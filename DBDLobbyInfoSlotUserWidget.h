// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DBDLobbyInfoSlotUserWidget.generated.h"

/**
 * 
 */
class UTextBlock;
class ADBDLobbyPlayerState;

UCLASS()
class DBDCOPY_API UDBDLobbyInfoSlotUserWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void Setup(ADBDLobbyPlayerState* InPlayerState);

protected:
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltatime) override;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* PlayerNameText;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* PingText;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* ReadyText;

private:
	TWeakObjectPtr<ADBDLobbyPlayerState> PlayerStatePtr;

	void UpdateSlotInfo();
};
