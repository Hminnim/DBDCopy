// Fill out your copyright notice in the Description page of Project Settings.


#include "DBDPlayerController.h"

void ADBDPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// Widget class setting
	if (PlayUserWidgetClass)
	{
		PlayUserWidget = CreateWidget<UDBDPlayUserWidget>(this, PlayUserWidgetClass);
		if (PlayUserWidget)
		{
			PlayUserWidget->AddToViewport(0);
		}
	}
	if (SkillCheckWidgetClass)
	{
		SkillCheckWidget = CreateWidget<UDBDSkillCheckUserWidget>(this, SkillCheckWidgetClass);
		if (SkillCheckWidget)
		{
			SkillCheckWidget->AddToViewport(1);
			SkillCheckWidget->SetVisibility(ESlateVisibility::Hidden);
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

void ADBDPlayerController::ShowInteractionProgress(float Value)
{
	if (PlayUserWidget)
	{
		PlayUserWidget->ShowInteractionProgress(Value);
	}
}

void ADBDPlayerController::HideInteractionProgress()
{
	if (PlayUserWidget)
	{
		PlayUserWidget->HideInteractionProgress();
	}
}

void ADBDPlayerController::ShowActionMessage(FString Message)
{
	if (PlayUserWidget)
	{
		PlayUserWidget->ShowActionMessage(Message);
	}
}

void ADBDPlayerController::HideActionMessage()
{
	if (PlayUserWidget)
	{
		PlayUserWidget->HideActionMessage();
	}
}

void ADBDPlayerController::ShowSkillCheck()
{
	if (AlertSound)
	{
		UGameplayStatics::PlaySound2D(this, AlertSound);
	}

	bIsSkillChecking = true;

	GetWorld()->GetTimerManager().SetTimer
	(
		SkillCheckTimerHandle,
		this,
		&ADBDPlayerController::StartSkillCheck,
		0.5f,
		false
	);
}

void ADBDPlayerController::StartSkillCheck()
{
	if (SkillCheckWidget)
	{
		SkillCheckWidget->SetVisibility(ESlateVisibility::Visible);
		SkillCheckWidget->SetGeneratorSkillCheck();
		SkillCheckWidget->StartPointerMove();
	}
}

void ADBDPlayerController::HideSkillCheck()
{
	SkillCheckWidget->SetVisibility(ESlateVisibility::Hidden);
	bIsSkillChecking = false;
}

void ADBDPlayerController::StopSkillCheck()
{
	if (SkillCheckWidget)
	{
		SkillCheckWidget->StopPointerMove();
		HideSkillCheck();
	}

	GetWorld()->GetTimerManager().ClearTimer(SkillCheckTimerHandle);
}

int8 ADBDPlayerController::GetSkillCheckResult()
{
	StopSkillCheck();

	if (SkillCheckWidget)
	{
		return SkillCheckWidget->GetSkillCheckResult();
	}
	else
	{
		return int8(2);
	}
}
