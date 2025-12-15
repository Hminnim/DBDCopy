// Fill out your copyright notice in the Description page of Project Settings.


#include "DBDTitlePlayerController.h"
#include "Blueprint/UserWidget.h"
#include "DBDGameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "DBDSessionInstanceSubsystem.h"

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

void ADBDTitlePlayerController::CreateSession()
{
	UDBDGameInstance* GI = Cast<UDBDGameInstance>(GetGameInstance());
	if (GI)
	{
		GI->bIsKiller = true;

		UDBDSessionInstanceSubsystem* SessionSystem = GI->GetSubsystem<UDBDSessionInstanceSubsystem>();
		if (SessionSystem)
		{
			SessionSystem->CreateSession(5, true);
		}
	}
}

void ADBDTitlePlayerController::FindSession()
{
	UDBDGameInstance* GI = Cast<UDBDGameInstance>(GetGameInstance());
	if (GI)
	{
		GI->bIsKiller = false;

		UDBDSessionInstanceSubsystem* SessionSystem = GI->GetSubsystem<UDBDSessionInstanceSubsystem>();
		if (SessionSystem)
		{
			SessionSystem->FindSessions(10, true);
		}
	}
}
