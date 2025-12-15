// Fill out your copyright notice in the Description page of Project Settings.


#include "DBDLobbyPlayerState.h"
#include "Net/UnrealNetwork.h"

void ADBDLobbyPlayerState::OnRep_IsReady()
{
	OnIsReadyChanged.Broadcast(bIsReady);
}

void ADBDLobbyPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ADBDLobbyPlayerState, bIsReady);
	DOREPLIFETIME(ADBDLobbyPlayerState, bIsKiller);
}
