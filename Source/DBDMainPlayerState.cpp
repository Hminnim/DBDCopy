// Fill out your copyright notice in the Description page of Project Settings.


#include "DBDMainPlayerState.h"
#include "Net/UnrealNetwork.h"

void ADBDMainPlayerState::SetHealthState(EHealthState NewState)
{
	if (HasAuthority())
	{
		CurrentHealthState = NewState;
		OnRep_CurrentHealthState();
	}
}

EHealthState ADBDMainPlayerState::GetCurrentHealthState()
{
	return CurrentHealthState;
}

void ADBDMainPlayerState::OnRep_CurrentHealthState()
{
	OnHealthStateChanged.Broadcast(CurrentHealthState);
}

void ADBDMainPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ADBDMainPlayerState, bIsKiller);
	DOREPLIFETIME(ADBDMainPlayerState, CurrentHealthState);
	DOREPLIFETIME(ADBDMainPlayerState, CurrentHookStageRate);
	DOREPLIFETIME(ADBDMainPlayerState, CurrentHookStageType);
}
