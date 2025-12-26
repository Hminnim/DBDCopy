// Fill out your copyright notice in the Description page of Project Settings.


#include "DBDPlayUserWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "GameFramework/GameStateBase.h"
#include "DBDPlayerStateUserWidget.h"
#include "DBDMainPlayerState.h"

void UDBDPlayUserWidget::NativeConstruct()
{
	Super::NativeConstruct();

	InteractionProgressBar->SetVisibility(ESlateVisibility::Hidden);
	InteractionTextBlock->SetVisibility(ESlateVisibility::Hidden);
	ActionTextBlock->SetVisibility(ESlateVisibility::Hidden);
}

void UDBDPlayUserWidget::ShowInteractionMessage(FString Message)
{
	InteractionTextBlock->SetVisibility(ESlateVisibility::Visible);
	InteractionTextBlock->SetText(FText::FromString(Message));
}

void UDBDPlayUserWidget::HideInteractionMessge()
{
	InteractionTextBlock->SetVisibility(ESlateVisibility::Hidden);
}

void UDBDPlayUserWidget::ShowInteractionProgress(float Value)
{
	InteractionProgressBar->SetVisibility(ESlateVisibility::Visible);
	InteractionProgressBar->SetPercent(Value / 100.0f);
}

void UDBDPlayUserWidget::HideInteractionProgress()
{
	InteractionProgressBar->SetVisibility(ESlateVisibility::Hidden);
}

void UDBDPlayUserWidget::ShowActionMessage(FString Message)
{
	ActionTextBlock->SetVisibility(ESlateVisibility::Visible);
	ActionTextBlock->SetText(FText::FromString(Message));
}

void UDBDPlayUserWidget::HideActionMessage()
{
	ActionTextBlock->SetVisibility(ESlateVisibility::Hidden);
}

void UDBDPlayUserWidget::SetPlayerList()
{
	AGameStateBase* GS = GetWorld()->GetGameState();
	if (GS)
	{
		for (APlayerState* PS : GS->PlayerArray)
		{
			ADBDMainPlayerState* MainPS = Cast<ADBDMainPlayerState>(PS);
			if (MainPS)
			{
				if (MainPS->bIsKiller)
				{
					continue;
				}

				UDBDPlayerStateUserWidget* NewPlayerStateWidget = CreateWidget<UDBDPlayerStateUserWidget>(this, PlayerStateWidgetClass);
				if (NewPlayerStateWidget)
				{
					NewPlayerStateWidget->SetUpPlayerState(PS->GetPlayerName());
					PlayersVerticalBox->AddChild(NewPlayerStateWidget);
				}
			}
		}
	}
}
