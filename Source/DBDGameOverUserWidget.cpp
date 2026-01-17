// Fill out your copyright notice in the Description page of Project Settings.


#include "DBDGameOverUserWidget.h"
#include "DBDPlayerController.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"

void UDBDGameOverUserWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	if (TitleButton)
	{
		TitleButton->OnClicked.AddDynamic(this, &UDBDGameOverUserWidget::OnTitleButtonClicked);
	}
}

void UDBDGameOverUserWidget::KillerGameOver(int32 KilledSurvivor)
{
	GameOverTextBlock->SetText(FText::FromString("Game Over"));
	KilledSurvivorTextBlock->SetText(FText::FromString(FString::Printf(TEXT("You killed %d Survivors"), KilledSurvivor)));
}

void UDBDGameOverUserWidget::SurvivorGameOver(bool bEscaped)
{
	FString GameOverMessage;
	if (bEscaped)
	{
		GameOverMessage = "Escaped";
	}
	else
	{
		GameOverMessage = "Dead";
	}

	GameOverTextBlock->SetText(FText::FromString(GameOverMessage));
	KilledSurvivorTextBlock->SetVisibility(ESlateVisibility::Hidden);
}

void UDBDGameOverUserWidget::OnTitleButtonClicked()
{
	ADBDPlayerController* PC = GetOwningPlayer<ADBDPlayerController>();
	if (PC)
	{
		PC->LeaveGame();
	}
}


