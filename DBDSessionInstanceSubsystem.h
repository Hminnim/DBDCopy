// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "FindSessionsCallbackProxy.h"
#include "DBDSessionInstanceSubsystem.generated.h"

// For Blueprint event
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCreateSessionCompleteEvent, bool, Successful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFindSessionsCompleteEvent, const TArray<FBlueprintSessionResult>&, SessionResults);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnJoinSessionCompleteEvent, bool, Successful);

/**
 * 
 */
UCLASS()
class DBDCOPY_API UDBDSessionInstanceSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	UDBDSessionInstanceSubsystem();

	UFUNCTION(BlueprintCallable)
	void CreateSession(int32 NumPublicConnections, bool bIsLAN);
	UFUNCTION(BlueprintCallable)
	void FindSessions(int32 MaxSearchResults, bool bIsLAN);
	UFUNCTION(BlueprintCallable)
	void JoinSession(const FBlueprintSessionResult& SessionResult);

	UPROPERTY(BlueprintAssignable)
	FOnCreateSessionCompleteEvent OnCreateSessionCompleteEvent;
	UPROPERTY(BlueprintAssignable)
	FOnFindSessionsCompleteEvent OnFindSessionCompleteEvent;
	UPROPERTY(BlueprintAssignable)
	FOnJoinSessionCompleteEvent OnJoinSessionCompleteEvent;

protected:
	IOnlineSessionPtr GetSessionInterface();

	void OnCreatedSessionComplete(FName SessionName, bool bWasSuccessful);
	void OnFindSessionComplete(bool bWasSuccessful);
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);

private:
	TSharedPtr<FOnlineSessionSearch> LastSessionSearch;
};
