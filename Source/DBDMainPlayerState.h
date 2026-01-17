// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "Net/UnrealNetwork.h"
#include "DBDMainPlayerState.generated.h"

/**
 * 
 */

UENUM(BlueprintType)
enum class EHealthState : uint8
{
	Healthy		UMETA(DisplayName = "Healthy"),
	Injured		UMETA(DisplayName = "Injured"),
	DeepWound	UMETA(DisplayName = "DeepWound"),
	Carried		UMETA(DisplayName = "Carried"),
	Hooked		UMETA(DisplayName = "Hooked"),
	Death		UMETA(DisplayName = "Death"),
	Exit		UMETA(DisplayName = "Exit")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHealthStateChanged, EHealthState, NewState);

UCLASS()
class DBDCOPY_API ADBDMainPlayerState : public APlayerState
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintAssignable)
	FOnHealthStateChanged OnHealthStateChanged;
	UPROPERTY(Replicated)
	bool bIsKiller = true;
	UPROPERTY(Replicated)
	int8 CurrentHookStageType = int8(0);
	UPROPERTY(Replicated)
	float CurrentHookStageRate = 100.0f;

	UFUNCTION()
	void SetHealthState(EHealthState NewState);
	UFUNCTION()
	EHealthState GetCurrentHealthState();

protected:
	UPROPERTY(ReplicatedUsing = OnRep_CurrentHealthState)
	EHealthState CurrentHealthState = EHealthState::Healthy;

	UFUNCTION()
	void OnRep_CurrentHealthState();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
};
