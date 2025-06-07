// Fill out your copyright notice in the Description page of Project Settings.


#include "DBDPlayUserWidget.h"

bool UDBDPlayUserWidget::Initialize()
{
	Super::Initialize();

	InteractionProgressBar->SetVisibility(ESlateVisibility::Hidden);
	InteractionTextBlock->SetVisibility(ESlateVisibility::Hidden);

	return true;
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
