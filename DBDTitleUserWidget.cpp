// Fill out your copyright notice in the Description page of Project Settings.


#include "DBDTitleUserWidget.h"

bool UDBDTitleUserWidget::Initialize()
{
	Super::Initialize();

	//Bind function
	if (QuitButton)
	{
		QuitButton->OnClicked.AddDynamic(this, &UDBDTitleUserWidget::OnQuitButtonClicked);
	}
	if (KillerButton)
	{
		KillerButton->OnClicked.AddDynamic(this, &UDBDTitleUserWidget::OnKillerButtonClicked);
	}
	if (SurvivorButton)
	{
		SurvivorButton->OnClicked.AddDynamic(this, &UDBDTitleUserWidget::OnSurvivorButtonClicked);
	}

	return true;
}

void UDBDTitleUserWidget::OnQuitButtonClicked()
{
}

void UDBDTitleUserWidget::OnKillerButtonClicked()
{
	ADBDTitlePlayerController* PC = GetOwningPlayer<ADBDTitlePlayerController>();
	if (PC)
	{
		PC->OnChracterSelected(true);
	}
}

void UDBDTitleUserWidget::OnSurvivorButtonClicked()
{
	ADBDTitlePlayerController* PC = GetOwningPlayer<ADBDTitlePlayerController>();
	if (PC)
	{
		PC->OnChracterSelected(false);
	}
}
	