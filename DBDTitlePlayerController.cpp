// Fill out your copyright notice in the Description page of Project Settings.


#include "DBDTitlePlayerController.h"

void ADBDTitlePlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (TitleUserWidgetclass)
	{
		TitleUserWidget = CreateWidget<UUserWidget>(this, TitleUserWidgetclass);
		if (TitleUserWidget)
		{
			TitleUserWidget->AddToViewport(0);
			SetInputMode(FInputModeUIOnly());
			bShowMouseCursor = true;
		}
	}
}

void ADBDTitlePlayerController::OnChracterSelected(bool bIsKiller)
{
	UDBDGameInstance* GI = Cast<UDBDGameInstance>(GetGameInstance());
	
	if (GI)
	{
		GI->bIsKiller = bIsKiller;
	}

	UGameplayStatics::OpenLevel(this, "Main");
}