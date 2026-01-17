// Fill out your copyright notice in the Description page of Project Settings.


#include "DBDPlayerStateUserWidget.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/ProgressBar.h"

void UDBDPlayerStateUserWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void UDBDPlayerStateUserWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (HookStageRateProgressBar && PlayerStatePtr)
	{
		if (bIsHooked)
		{
			HookStageRateProgressBar->SetVisibility(ESlateVisibility::Visible);
			HookStageRateProgressBar->SetPercent(PlayerStatePtr->CurrentHookStageRate / 100.0f);
		}
		else
		{
			HookStageRateProgressBar->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}

void UDBDPlayerStateUserWidget::SetUpPlayerState(FString PlayerName)
{
	CurrentHealthStateImage->SetBrushFromTexture(HealthyTexture);
	PlayerNameText->SetText(FText::FromString(PlayerName));
}

void UDBDPlayerStateUserWidget::ChangeHealthStateImage(EHealthState NewState)
{
	UTexture2D* NewTexture = HealthyTexture;

	if (NewState == EHealthState::Healthy) NewTexture = HealthyTexture;
	if (NewState == EHealthState::Injured) NewTexture = InjuredTexture;
	if (NewState == EHealthState::DeepWound) NewTexture = CrawlTexture;
	if (NewState == EHealthState::Carried) NewTexture = CarryingTexture;
	if (NewState == EHealthState::Hooked)
	{
		NewTexture = HookedTexture;
		bIsHooked = true;
	}
	else
	{
		bIsHooked = false;
	}
	if (NewState == EHealthState::Death) NewTexture = DeathTexture;
	if (NewState == EHealthState::Exit) NewTexture = ExitTexture;

	CurrentHealthStateImage->SetBrushFromTexture(NewTexture);
}

void UDBDPlayerStateUserWidget::Setup(ADBDMainPlayerState* InPlayerState)
{
	if (InPlayerState)
	{
		PlayerStatePtr = InPlayerState;
		PlayerStatePtr->OnHealthStateChanged.AddDynamic(this, &UDBDPlayerStateUserWidget::ChangeHealthStateImage);
	}
}

