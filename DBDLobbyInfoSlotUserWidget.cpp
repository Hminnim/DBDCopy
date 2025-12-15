// Fill out your copyright notice in the Description page of Project Settings.


#include "DBDLobbyInfoSlotUserWidget.h"
#include "DBDLobbyPlayerState.h"
#include "Components/TextBlock.h"
#include "GameFramework/PlayerState.h"

void UDBDLobbyInfoSlotUserWidget::Setup(ADBDLobbyPlayerState* InPlayerState)
{
	if (InPlayerState)
	{
		PlayerStatePtr = InPlayerState;
		UpdateSlotInfo(); // Setup Update slot info
	}	
}

void UDBDLobbyInfoSlotUserWidget::NativeTick(const FGeometry& MyGeometry, float InDeltatime)
{
	Super::NativeTick(MyGeometry, InDeltatime);

	UpdateSlotInfo();
}

void UDBDLobbyInfoSlotUserWidget::UpdateSlotInfo()
{
	if (!PlayerStatePtr.IsValid())
	{
		return;
	}

	ADBDLobbyPlayerState* PS = PlayerStatePtr.Get();

	if (PlayerNameText)
	{
		PlayerNameText->SetText(FText::FromString(PS->GetPlayerName()));
	}

	if (PingText)
	{
		int32 Ping = (int32)PS->GetPingInMilliseconds();
		PingText->SetText(FText::FromString(FString::Printf(TEXT("%d ms"), Ping)));
	}

	if (ReadyText)
	{
		if (PS->bIsReady)
		{
			ReadyText->SetText(FText::FromString(TEXT("READY")));
			ReadyText->SetColorAndOpacity(FLinearColor::Green);
		}
		else
		{
			ReadyText->SetText(FText::FromString(TEXT("WAITING")));
			ReadyText->SetColorAndOpacity(FLinearColor::White);
		}
	}
}
