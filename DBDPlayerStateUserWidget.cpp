// Fill out your copyright notice in the Description page of Project Settings.


#include "DBDPlayerStateUserWidget.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"

void UDBDPlayerStateUserWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void UDBDPlayerStateUserWidget::SetUpPlayerState(FString PlayerName)
{
	CurrentHealthStateImage->SetBrushFromTexture(HealthyTexture);
	PlayerNameText->SetText(FText::FromString(PlayerName));
}

void UDBDPlayerStateUserWidget::ChangeHealthStateImage(int8 Type)
{
	UTexture2D* NewTexture = HealthyTexture;

	if (Type == 0) NewTexture = HealthyTexture;
	if (Type == 1) NewTexture = InjuredTexture;
	if (Type == 2) NewTexture = CrawlTexture;
	if (Type == 3) NewTexture = CarryingTexture;
	if (Type == 4) NewTexture = HookedTexture;
	if (Type == 5) NewTexture = DeathTexture;
	if (Type == 6) NewTexture = ExitTexture;

	CurrentHealthStateImage->SetBrushFromTexture(NewTexture);
}

