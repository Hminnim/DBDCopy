// Fill out your copyright notice in the Description page of Project Settings.


#include "DBDSessionSlotWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "DBDSessionInstanceSubsystem.h"

void UDBDSessionSlotWidget::Setup(const FBlueprintSessionResult& Result)
{
	SessionResult = Result;

	if (RoomNameText)
	{
		RoomNameText->SetText(FText::FromString(Result.OnlineResult.Session.OwningUserName));
	}

	if (PingText)
	{
		int32 Ping = Result.OnlineResult.PingInMs;
		PingText->SetText(FText::FromString(FString::Printf(TEXT("%d ms"), Ping)));
	}
}

void UDBDSessionSlotWidget::NativeConstruct()
{
	Super::NativeConstruct();
	if (JoinButton)
	{
		JoinButton->OnClicked.AddDynamic(this, &UDBDSessionSlotWidget::OnJoinClicked);
	}
}

void UDBDSessionSlotWidget::OnJoinClicked()
{
	UGameInstance* GI = GetGameInstance();
	if (GI)
	{
		UDBDSessionInstanceSubsystem* Subsystem = GI->GetSubsystem<UDBDSessionInstanceSubsystem>();
		if (GI)
		{
			Subsystem->JoinSession(SessionResult);
		}
	}
}
