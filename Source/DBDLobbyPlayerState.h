// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "DBDLobbyPlayerState.generated.h"

// For UI
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerStateChanged, bool, bReady);

/**
 * 
 */
UCLASS()
class DBDCOPY_API ADBDLobbyPlayerState : public APlayerState
{
	GENERATED_BODY()
	
public:
	UPROPERTY(ReplicatedUsing = OnRep_IsReady, BlueprintReadOnly, Category = "Lobby Data")
	bool bIsReady = false;
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Lobby Data")
	bool bIsKiller = false;

	UFUNCTION()
	void OnRep_IsReady();

	UPROPERTY(BlueprintAssignable)
	FOnPlayerStateChanged OnIsReadyChanged;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void CopyProperties(class APlayerState* PlayerState) override;
};
