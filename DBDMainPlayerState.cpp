// Fill out your copyright notice in the Description page of Project Settings.


#include "DBDMainPlayerState.h"
#include "Net/UnrealNetwork.h"

void ADBDMainPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ADBDMainPlayerState, bIsKiller);
}
