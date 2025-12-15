// Fill out your copyright notice in the Description page of Project Settings.


#include "DBDLobbyUserWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/ScrollBox.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "DBDLobbyPlayerController.h"
#include "DBDLobbyPlayerState.h"
#include "DBDLobbyGameModeBase.h"
#include "Kismet/GameplayStatics.h"

void UDBDLobbyUserWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	if (ReadyStartButton)
	{
		ReadyStartButton->OnClicked.AddDynamic(this, &UDBDLobbyUserWidget::OnReadyStartClicked);
	}
	if (BackButton)
	{
		BackButton->OnClicked.AddDynamic(this, &UDBDLobbyUserWidget::OnBackClicked);
	}
}

void UDBDLobbyUserWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	UpdateButtonState();

	PlayerListUpdateTimeer += InDeltaTime;
	if (PlayerListUpdateTimeer >= 0.5f)
	{
		PlayerListUpdateTimeer = 0.0f;
		UpdatePlayerLists();
	}
}

void UDBDLobbyUserWidget::OnReadyStartClicked()
{
	ADBDLobbyPlayerController* PC = Cast<ADBDLobbyPlayerController>(GetOwningPlayer());
	if (PC)
	{
		if (PC->HasAuthority())
		{
			PC->StartGame();
		}
		else
		{
			PC->ToggleReadyState();
		}
	}
}

void UDBDLobbyUserWidget::OnBackClicked()
{
	ADBDLobbyPlayerController* PC = Cast<ADBDLobbyPlayerController>(GetOwningPlayer());
	if (PC)
	{
		if (PC->HasAuthority())
		{
			PC->ProcessLobbyDestroy();
		}
		else
		{
			PC->LeaveLobby();
		}
	}
}

void UDBDLobbyUserWidget::UpdateButtonState()
{
	APlayerController* PC = GetOwningPlayer();
	if (!PC)
	{
		return;
	}

	ADBDLobbyPlayerState* PS = PC->GetPlayerState<ADBDLobbyPlayerState>();
	if (!PS)
	{
		return;
	}

	// If host(killer)
	if (PC->HasAuthority())
	{
		ReadyStartText->SetText(FText::FromString(TEXT("Start")));
		
		ADBDLobbyGameModeBase* GM = GetWorld()->GetAuthGameMode<ADBDLobbyGameModeBase>();
		if (GM)
		{
			bool bCanStart = GM->bCanGameStart;

			if (bCanStart)
			{
				ReadyStartButton->SetIsEnabled(true);
				ReadyStartText->SetColorAndOpacity(FLinearColor::White);
			}
			else
			{
				ReadyStartButton->SetIsEnabled(false);
				ReadyStartText->SetColorAndOpacity(FLinearColor::Gray);
			}
		}
	}
	else
	{
		if (PS->bIsReady)
		{
			ReadyStartText->SetText(FText::FromString(TEXT("Cancel")));
		}
		else
		{
			ReadyStartText->SetText(FText::FromString(TEXT("Ready")));
		}

		ReadyStartButton->SetIsEnabled(true);
		ReadyStartText->SetColorAndOpacity(FLinearColor::White);
	}
}

void UDBDLobbyUserWidget::UpdatePlayerLists()
{
	if (!KillerScrollBox || !SurvivorScrollBox || !UserSlotClass)
	{
		return;
	}

	KillerScrollBox->ClearChildren();
	SurvivorScrollBox->ClearChildren();

	AGameStateBase* GS = GetWorld()->GetGameState();
	if (GS)
	{
		for (APlayerState* PS : GS->PlayerArray)
		{
			ADBDLobbyPlayerState* LobbyPS = Cast<ADBDLobbyPlayerState>(PS);
			if (LobbyPS)
			{
				UUserWidget* NewSlot = CreateWidget<UUserWidget>(this, UserSlotClass);
				if (LobbyPS->bIsKiller)
				{
					KillerScrollBox->AddChild(NewSlot);
				}
				else
				{
					SurvivorScrollBox->AddChild(NewSlot);
				}
			}
		}
	}
}
