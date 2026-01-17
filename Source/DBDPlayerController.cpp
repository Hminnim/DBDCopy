// Fill out your copyright notice in the Description page of Project Settings.


#include "DBDPlayerController.h"
#include "DBDGameOverUserWidget.h"
#include "DBDMainPlayerState.h"
#include "DBDKiller.h"
#include "DBDSurvivor.h"
#include "DBDSessionInstanceSubsystem.h"
#include "DBDTargetPopUpWidget.h"
#include "TimerManager.h"

void ADBDPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (!IsLocalController())
	{
		return;
	}

	if (IsLocalController() && LoadingWidgetClass)
	{
		LoadingWidget = CreateWidget<UDBDLoadingUserWidget>(this, LoadingWidgetClass);
		if (LoadingWidget)
		{
			LoadingWidget->AddToViewport(999);
		}

		SetInputMode(FInputModeUIOnly());
		Server_NotifyLoaded();
	}
}

void ADBDPlayerController::Server_NotifyLoaded_Implementation()
{
	ADBDGameModeBase* GM = GetWorld()->GetAuthGameMode<ADBDGameModeBase>();
	if (GM)
	{
		GM->HandlePlayerLoaded(this);
	}
}

void ADBDPlayerController::Client_StartGame_Implementation(int32 GeneratorNum)
{
	if (LoadingWidget)
	{
		LoadingWidget->RemoveFromParent();
		LoadingWidget = nullptr;
	}

	// Widget class setting
	if (PlayUserWidgetClass)
	{
		PlayUserWidget = CreateWidget<UDBDPlayUserWidget>(this, PlayUserWidgetClass);
		if (PlayUserWidget)
		{
			PlayUserWidget->SetPlayerList();
			PlayUserWidget->SetReaminGenerator(GeneratorNum);
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

void ADBDPlayerController::Client_NotifyGameResult_Implementation(bool bEscaped, int32 KilledSurvivor)
{
	if (GameOverWidgetClass)
	{
		GameOverWidget = CreateWidget<UDBDGameOverUserWidget>(this, GameOverWidgetClass);
		if (GameOverWidget)
		{
			ADBDMainPlayerState* PS = GetPlayerState<ADBDMainPlayerState>();
			if (PS)
			{
				if (PS->bIsKiller)
				{
					GameOverWidget->KillerGameOver(KilledSurvivor);
				}
				else
				{
					GameOverWidget->SurvivorGameOver(bEscaped);
				}
			}

			GameOverWidget->AddToViewport(999);
		}
	}

	SetInputMode(FInputModeUIOnly());
	bShowMouseCursor = true;
}

void ADBDPlayerController::Client_AllGeneratorCompleted_Implementation(const TArray<AActor*>& TargetLevers)
{
	ADBDMainPlayerState* PS = GetPlayerState<ADBDMainPlayerState>();
	if (PS)
	{
		if (PS->bIsKiller)
		{
			ADBDKiller* KillerActor = GetPawn<ADBDKiller>();
			if (KillerActor)
			{
				KillerActor->OnAllGeneratorCompleted();
			}
		}
		else
		{
			
		}
	}

	for (AActor* LeverActor : TargetLevers)
	{
		ShowTargetPopUpWidget(LeverActor, int32(0));
	}
}

void ADBDPlayerController::LeaveGame()
{
	UGameInstance* GI = GetGameInstance();
	if (GI)
	{
		UDBDSessionInstanceSubsystem* Subsystem = GI->GetSubsystem<UDBDSessionInstanceSubsystem>();
		if (Subsystem)
		{
			const IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();
			if (SessionInterface)
			{
				SessionInterface->DestroySession(NAME_GameSession);
			}
		}
	}

	UGameplayStatics::OpenLevel(this, FName("TitleMenu"));
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
	if (SkillCheckWidget)
	{
		SkillCheckWidget->SetVisibility(ESlateVisibility::Visible);
	}
	bIsSkillChecking = true;
}

void ADBDPlayerController::HideSkillCheck()
{
	SkillCheckWidget->SetVisibility(ESlateVisibility::Hidden);
	bIsSkillChecking = false;
}

void ADBDPlayerController::StartGeneratorSkillCheck()
{
	if (AlertSound)
	{
		UGameplayStatics::PlaySound2D(this, AlertSound);
	}

	GetWorld()->GetTimerManager().SetTimer
	(
		SkillCheckTimerHandle,
		this,
		&ADBDPlayerController::GeneratorSkillcheck,
		0.5f,
		false
	);
}

void ADBDPlayerController::GeneratorSkillcheck()
{
	if (SkillCheckWidget)
	{
		SkillCheckWidget->SetVisibility(ESlateVisibility::Visible);
		SkillCheckWidget->SetGeneratorSkillCheck();
		SkillCheckWidget->StartGeneratorSkillCheck();
	}
	ShowSkillCheck();
}

void ADBDPlayerController::StopGeneratorSkillCheck()
{
	if (SkillCheckWidget)
	{
		SkillCheckWidget->StopGeneratorSkillCheck();
		HideSkillCheck();
	}

	GetWorld()->GetTimerManager().ClearTimer(SkillCheckTimerHandle);
}

int8 ADBDPlayerController::GetGeneratorSkillCheckResult()
{
	StopGeneratorSkillCheck();

	if (SkillCheckWidget)
	{
		if (SkillCheckWidget->PointerAngle < 0.0f)
		{
			SkillCheckWidget->PointerAngle += 360.0f;
		}

		// Great skill check
		if (SkillCheckWidget->CircleAngle <= SkillCheckWidget->PointerAngle 
			&& SkillCheckWidget->PointerAngle < SkillCheckWidget->CircleAngle + 10.8)
		{
			return int8(0);
		}

		// Good skill check
		else if (SkillCheckWidget->CircleAngle + 10.8 <= SkillCheckWidget->PointerAngle 
			&& SkillCheckWidget->PointerAngle < SkillCheckWidget->CircleAngle + 46.8)
		{
			return int8(1);
		}

		// Failed skill check
		else
		{
			return int8(2);
		}
	}
	else
	{
		return int8(2);
	}
}

void ADBDPlayerController::StartWiggleSkillCheck()
{
	if (AlertSound)
	{
		UGameplayStatics::PlaySound2D(this, AlertSound);
	}

	GetWorld()->GetTimerManager().SetTimer
	(
		SkillCheckTimerHandle,
		this,
		&ADBDPlayerController::WiggleSkillCheck,
		0.5f,
		false
	);
}

void ADBDPlayerController::WiggleSkillCheck()
{
	if (SkillCheckWidget)
	{
		SkillCheckWidget->SetWiggleSKillCheck();
		SkillCheckWidget->StartWiggleSkillCheck();
		ShowSkillCheck();
	}
}

void ADBDPlayerController::StopWiggleSkillCheck()
{
	if (SkillCheckWidget)
	{
		SkillCheckWidget->StopWiggleSkillCheck();
		HideSkillCheck();
	}
}

int8 ADBDPlayerController::GetWWiggleSkillCheckResult()
{
	if (SkillCheckWidget)
	{
		int8 SkillCheckTarget = SkillCheckWidget->GetWiggleSkillCheckTarget();
		float PointerAngle = SkillCheckWidget->PointerAngle;
		float MinGreat = 90.0f - (360.0f * (1.0f - SkillCheckWidget->GetWiggleGreatCirclePercent()) / 2);
		float MaxGreat = 90.0f + (360.0f * (1.0f - SkillCheckWidget->GetWiggleGreatCirclePercent()) / 2);
		float MinGood = 90.0f - (360.0f * (1.0f - SkillCheckWidget->GetWiggleGoodCirclePercent()) / 2);
		float MaxGood = 90.0f + (360.0f * (1.0f - SkillCheckWidget->GetWiggleGoodCirclePercent()) / 2);

		// Right target
		if (SkillCheckTarget == 0)
		{
			// Great skill check
			if (MinGreat <= PointerAngle && PointerAngle <= MaxGreat)
			{
				SkillCheckWidget->ChangePointerMoveDirection();
				SkillCheckWidget->OnSucceededWiggleGreatSkillCheck();
				SkillCheckWidget->OnSucceededWiggleSkillCheck();
				bWasInZone = false;
				return int8(0);
			}
			// Good skill check
			else if (MinGood <= PointerAngle && PointerAngle <= MaxGood)
			{
				SkillCheckWidget->ChangePointerMoveDirection();
				SkillCheckWidget->OnFailedWiggleGreatSkillCheck();
				SkillCheckWidget->OnSucceededWiggleSkillCheck();
				bWasInZone = false;
				return int8(1);
			}
			// Failed skill check
			else
			{
				SkillCheckWidget->OnFailedWiggleGreatSkillCheck();
				SkillCheckWidget->OnFailedWiggleSkillCheck();
				return int8(2);
			}
		}
		// Left target
		else
		{
			// Great skill check
			if (MinGreat + 180.0f <= PointerAngle && PointerAngle <= MaxGreat + 180.0f)
			{
				SkillCheckWidget->ChangePointerMoveDirection();
				SkillCheckWidget->OnSucceededWiggleGreatSkillCheck();
				SkillCheckWidget->OnSucceededWiggleSkillCheck();
				bWasInZone = false;
				return int8(0);
			}
			// Good skill check
			else if (MinGood + 180.0f <= PointerAngle && PointerAngle <= MaxGood + 180.0f)
			{
				SkillCheckWidget->ChangePointerMoveDirection();
				SkillCheckWidget->OnFailedWiggleGreatSkillCheck();
				SkillCheckWidget->OnSucceededWiggleSkillCheck();
				bWasInZone = false;
				return int8(1);
			}
			// Failed skill check
			else
			{
				SkillCheckWidget->OnFailedWiggleGreatSkillCheck();
				SkillCheckWidget->OnFailedWiggleSkillCheck();
				return int8(2);
			}
		}
	
	}
	else
	{
		return int8(2);
	}
}

bool ADBDPlayerController::GetWiggleSkillCheckMiss()
{
	if (SkillCheckWidget)
	{
		int8 SkillCheckTarget = SkillCheckWidget->GetWiggleSkillCheckTarget();
		bool bMoveClock = SkillCheckWidget->GetPointerMoveDirection();
		float PointerAngle = SkillCheckWidget->PointerAngle;
		float MinGood = 90.0f - (360.0f * (1.0f - SkillCheckWidget->GetWiggleGoodCirclePercent()) / 2);
		float MaxGood = 90.0f + (360.0f * (1.0f - SkillCheckWidget->GetWiggleGoodCirclePercent()) / 2);
		bool bIsZoneNow = false;

		if (SkillCheckTarget == 0)
		{
			if (MinGood <= PointerAngle && PointerAngle <= MaxGood)
			{
				bIsZoneNow = true;
				bWasInZone = true;
			}
		}
		else
		{
			if (MinGood + 180.0f <= PointerAngle && PointerAngle <= MaxGood + 180.0f)
			{
				bIsZoneNow = true;
				bWasInZone = true;
			}
		}

		if (!bIsZoneNow && bWasInZone)
		{
			bWasInZone = false;
			SkillCheckWidget->OnFailedWiggleGreatSkillCheck();
			SkillCheckWidget->OnFailedWiggleSkillCheck();
			return true;
		}
		else
		{
			return false;
		}
	}
	else 
	{
		return false;
	}
}

void ADBDPlayerController::StartStruggleSkillCheck(int8 Count)
{
	if (AlertSound)
	{
		UGameplayStatics::PlaySound2D(this, AlertSound);
	}

	if (SkillCheckWidget)
	{
		SkillCheckWidget->SetStruggleSkillCheck(Count);
	}

	GetWorld()->GetTimerManager().SetTimer
	(
		SkillCheckTimerHandle,
		this,
		&ADBDPlayerController::StruggleSkillCheck,
		0.5f,
		false
	);
}

void ADBDPlayerController::StruggleSkillCheck()
{
	if (SkillCheckWidget)
	{
		SkillCheckWidget->SetVisibility(ESlateVisibility::Visible);
		SkillCheckWidget->StartStruggleSkillCheck();
	}
	ShowSkillCheck();
}

void ADBDPlayerController::StopStruggleSkillCheck()
{
	if (SkillCheckWidget)
	{
		SkillCheckWidget->StopStruggleSkillCheck();
		HideSkillCheck();
	}

	GetWorld()->GetTimerManager().ClearTimer(SkillCheckTimerHandle);
}

int8 ADBDPlayerController::GetStruggleSkillCheckResult()
{
	if (SkillCheckWidget)
	{
		if (SkillCheckWidget->PointerAngle < 0.0f)
		{
			SkillCheckWidget->PointerAngle += 360.0f;
		}

		// Success
		if (SkillCheckWidget->CircleAngle < SkillCheckWidget->PointerAngle
			&& SkillCheckWidget->PointerAngle <= SkillCheckWidget->CircleAngle + 360.0f * (1.0f - SkillCheckWidget-> CurrentStruggleSuccessPercent))
		{
			return int8(0);
		}
		// Fail
		else
		{
			return int8(2);
		}
	}
	else
	{
		return int8(2);
	}
	
}

void ADBDPlayerController::ShowTargetPopUpWidget(AActor* NewTarget, int32 NewType)
{
	if (!NewTarget || !TargetPopUpWidgetClass)
	{
		return;
	}

	if (TargetWidgetMap.Contains(NewTarget))
	{
		return;
	}

	UDBDTargetPopUpWidget* NewWidget = CreateWidget<UDBDTargetPopUpWidget>(this, TargetPopUpWidgetClass);
	if (NewWidget)
	{
		NewWidget->SetTargetActor(NewTarget, NewType);
		NewWidget->SetAlignmentInViewport(FVector2D(0.5f, 0.5f));
		NewWidget->AddToViewport();

		TargetWidgetMap.Add(NewTarget, NewWidget);

		FTimerHandle TimerHandle;
		FTimerDelegate TimerDel;
		TimerDel.BindUFunction(this, FName("RemoveTargetPopUpWidget"), NewTarget);

		GetWorld()->GetTimerManager().SetTimer(TimerHandle, TimerDel, 5.0f, false);
	}
}

void ADBDPlayerController::RemoveTargetPopUpWidget(AActor* TargetToRemove)
{
	UDBDTargetPopUpWidget** FoundWidgetPtr = TargetWidgetMap.Find(TargetToRemove);
	if (FoundWidgetPtr)
	{
		UDBDTargetPopUpWidget* FoundWidget = *FoundWidgetPtr;
		if (FoundWidget)
		{
			FoundWidget->RemoveFromParent();
			FoundWidget = nullptr;
		}

		TargetWidgetMap.Remove(TargetToRemove);
	}
}

void ADBDPlayerController::Client_ChangeRemainedGeneratorNum_Implementation(int32 NewNum)
{
	if (PlayUserWidget)
	{
		PlayUserWidget->SetReaminGenerator(NewNum);
	}
}

void ADBDPlayerController::CharacterChange_Implementation(bool bIsKiller)
{
	ADBDGameModeBase* GM = Cast<ADBDGameModeBase>(GetWorld()->GetAuthGameMode());
	if (GM)
	{
		GM->ChangeCharacter(this, bIsKiller);
	}
}
