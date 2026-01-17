// Fill out your copyright notice in the Description page of Project Settings.


#include "DBDTargetPopUpWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Image.h"
#include "TimerManager.h"

void UDBDTargetPopUpWidget::NativeConstruct()
{
	Super::NativeConstruct();

	SetVisibility(ESlateVisibility::HitTestInvisible);
	SetAlignmentInViewport(FVector2D(0.5f, 0.5f));

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(UpdateTimerHandle, this, &UDBDTargetPopUpWidget::UpdateWidgetPosition, 0.015f, true);
	}
}

void UDBDTargetPopUpWidget::NativeDestruct()
{
	Super::NativeDestruct();

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(UpdateTimerHandle);
	}
}

void UDBDTargetPopUpWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	//if (!TargetActor.IsValid())
	//{
	//	GEngine->AddOnScreenDebugMessage(-1, 0.5f, FColor::Red, FString::Printf(TEXT("Invalid TargetActor")));
	//	return;
	//}

	//APlayerController* PC = GetOwningPlayer();
	//if (!PC)
	//{
	//	GEngine->AddOnScreenDebugMessage(-1, 0.5f, FColor::Red, FString::Printf(TEXT("Not found PC")));
	//	return;
	//}

	//// Target world location
	//FVector TargetLocation = TargetActor->GetActorLocation();
	//FVector2D ScreenLocation;

	//bool bIsOnScreen = PC->ProjectWorldLocationToScreen(TargetLocation, ScreenLocation);

	//if (bIsOnScreen)
	//{
	//	SetRenderOpacity(1.0f);
	//	SetPositionInViewport(ScreenLocation);
	//}
	//else
	//{
	//	SetPositionInViewport(FVector2D(-10000.0f, -10000.0f));
	//	GEngine->AddOnScreenDebugMessage(-1, 0.5f, FColor::Red, FString::Printf(TEXT("out of screen")));
	//}
}

void UDBDTargetPopUpWidget::SetTargetActor(AActor* NewTarget, int32 NewType)
{
	TargetActor = NewTarget;
	TargetType = NewType;

	TargetImage->SetBrushFromTexture(ExitTexture);
}

void UDBDTargetPopUpWidget::UpdateWidgetPosition()
{
	if (!TargetActor.IsValid())
	{
		GEngine->AddOnScreenDebugMessage(-1, 0.5f, FColor::Red, FString::Printf(TEXT("Invalid TargetActor")));
		return;
	}

	APlayerController* PC = GetOwningPlayer();
	if (!PC)
	{
		GEngine->AddOnScreenDebugMessage(-1, 0.5f, FColor::Red, FString::Printf(TEXT("Not found PC")));
		return;
	}

	// Target world location
	FVector TargetLocation = TargetActor->GetActorLocation();
	FVector2D ScreenLocation;

	bool bIsOnScreen = PC->ProjectWorldLocationToScreen(TargetLocation, ScreenLocation);

	if (bIsOnScreen)
	{
		SetRenderOpacity(1.0f);
		SetPositionInViewport(ScreenLocation);
	}
	else
	{
		SetPositionInViewport(FVector2D(-10000.0f, -10000.0f));
		GEngine->AddOnScreenDebugMessage(-1, 0.5f, FColor::Red, FString::Printf(TEXT("out of screen")));
	}
}
