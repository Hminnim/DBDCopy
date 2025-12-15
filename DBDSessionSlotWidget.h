// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "FindSessionsCallbackProxy.h"
#include "DBDSessionSlotWidget.generated.h"

/**
 * 
 */
class UTextBlock;
class UButton;

UCLASS()
class DBDCOPY_API UDBDSessionSlotWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void Setup(const FBlueprintSessionResult& Result);

protected:
	virtual void NativeConstruct() override;

private:
	UPROPERTY(meta = (BindWidget))
	UTextBlock* RoomNameText;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* PingText;
	UPROPERTY(meta = (BindWidget))
	UButton* JoinButton;

	FBlueprintSessionResult SessionResult;

	UFUNCTION()
	void OnJoinClicked();

};
