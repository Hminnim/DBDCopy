// Fill out your copyright notice in the Description page of Project Settings.


#include "DBDTitleUserWidget.h"
#include "Components/Button.h"
#include "Components/ScrollBox.h"
#include "Components/CircularThrobber.h"
#include "DBDSessionSlotWidget.h"
#include "DBDSessionInstanceSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "DBDTitlePlayerController.h"

bool UDBDTitleUserWidget::Initialize()
{
	Super::Initialize();

	return true;
}

void UDBDTitleUserWidget::NativeConstruct()
{
	Super::NativeConstruct();

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
	if (BackButton)
	{
		BackButton->OnClicked.AddDynamic(this, &UDBDTitleUserWidget::OnBackButtonClicked);
	}
	if (RefreshButton)
	{
		RefreshButton->OnClicked.AddDynamic(this, &UDBDTitleUserWidget::OnRefreshButtonClicked);
	}
	if (SessionListPopup)
	{
		SessionListPopup->SetVisibility(ESlateVisibility::Hidden);
	}

	UGameInstance* GI = GetGameInstance();
	if (GI)
	{
		UDBDSessionInstanceSubsystem* Subsystem = GI->GetSubsystem<UDBDSessionInstanceSubsystem>();
		if (Subsystem)
		{
			Subsystem->OnFindSessionCompleteEvent.AddDynamic(this, &UDBDTitleUserWidget::OnFindSessionsComplete);
			Subsystem->OnCreateSessionCompleteEvent.AddDynamic(this, &UDBDTitleUserWidget::OnCreateSessionComplete);
		}
	}
}

void UDBDTitleUserWidget::OnQuitButtonClicked()
{
	
}

void UDBDTitleUserWidget::OnKillerButtonClicked()
{
	ADBDTitlePlayerController* PC = GetOwningPlayer<ADBDTitlePlayerController>();
	if (PC)
	{
		PC->CreateSession();
	}
}

void UDBDTitleUserWidget::OnSurvivorButtonClicked()
{
	if (SessionListScrollBox)
	{
		SessionListScrollBox->ClearChildren();
		SessionListPopup->SetVisibility(ESlateVisibility::Visible);
	}

	if (LoadingThrobber)
	{
		LoadingThrobber->SetVisibility(ESlateVisibility::Visible);
	}

	ADBDTitlePlayerController* PC = GetOwningPlayer<ADBDTitlePlayerController>();
	if (PC)
	{
		PC->FindSession();
	}
}

void UDBDTitleUserWidget::OnBackButtonClicked()
{
	if (SessionListPopup)
	{
		SessionListPopup->SetVisibility(ESlateVisibility::Hidden);
	}
	if (SessionListScrollBox)
	{
		SessionListScrollBox->ClearChildren();
	}
}

void UDBDTitleUserWidget::OnRefreshButtonClicked()
{
	if (SessionListScrollBox)
	{
		SessionListScrollBox->ClearChildren();
	}

	if (LoadingThrobber)
	{
		LoadingThrobber->SetVisibility(ESlateVisibility::Visible);
	}

	ADBDTitlePlayerController* PC = GetOwningPlayer<ADBDTitlePlayerController>();
	if (PC)
	{
		PC->FindSession();
	}

	if (RefreshButton)
	{
		RefreshButton->SetIsEnabled(false);
	}
}

void UDBDTitleUserWidget::OnFindSessionsComplete(const TArray<FBlueprintSessionResult>& SessionResults)
{
	if (!SessionSlotClass || !SessionListScrollBox || !SessionListPopup)
	{
		return;
	}
	// If player closed SessionListPopup
	if (!SessionListPopup->IsVisible())
	{
		return;
	}

	if (LoadingThrobber)
	{
		LoadingThrobber->SetVisibility(ESlateVisibility::Hidden);
	}

	if (SessionListScrollBox)
	{
		SessionListScrollBox->ClearChildren();
	}

	for (const FBlueprintSessionResult& Result : SessionResults)
	{
		UDBDSessionSlotWidget* NewSlot = CreateWidget<UDBDSessionSlotWidget>(this, SessionSlotClass);

		if (NewSlot)
		{
			NewSlot->Setup(Result);
			
			SessionListScrollBox->AddChild(NewSlot);
		}
	}

	if (RefreshButton)
	{
		RefreshButton->SetIsEnabled(true);
	}
}

void UDBDTitleUserWidget::OnCreateSessionComplete(bool bSuccessful)
{
	// Listen server
	if (bSuccessful)
	{
		UGameplayStatics::OpenLevel(this, FName("LobbyMap"), true, TEXT("listen"));
	}
}
	