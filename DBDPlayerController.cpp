// Fill out your copyright notice in the Description page of Project Settings.


#include "DBDPlayerController.h"

void ADBDPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (PlayUserWidgetClass)
	{
		PlayUserWidget = CreateWidget<UDBDPlayUserWidget>(this, PlayUserWidgetClass);
		if (PlayUserWidget)
		{
			PlayUserWidget->AddToViewport(0);
		}
	}

	SetInputMode(FInputModeGameOnly());
	bShowMouseCursor = false;
}

void ADBDPlayerController::ShowIneractionMessage(FString Message)
{
	if (PlayUserWidget)
	{
		PlayUserWidget->ShowInteractionMessage(Message);
	}
}

void ADBDPlayerController::HideInteractionMessage()
{
	if (PlayUserWidget)
	{
		PlayUserWidget->HideInteractionMessge();
	}
}
