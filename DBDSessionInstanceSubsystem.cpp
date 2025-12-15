// Fill out your copyright notice in the Description page of Project Settings.


#include "DBDSessionInstanceSubsystem.h"
#include "OnlineSubsystem.h"
#include "Online/OnlineSessionNames.h"

UDBDSessionInstanceSubsystem::UDBDSessionInstanceSubsystem()
{
}

IOnlineSessionPtr UDBDSessionInstanceSubsystem::GetSessionInterface()
{
	IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
	if (Subsystem)
	{
		return Subsystem->GetSessionInterface();
	}
	else
	{
		return nullptr;
	}
}

void UDBDSessionInstanceSubsystem::CreateSession(int32 NumPublicConnections, bool bIsLAN)
{
	const IOnlineSessionPtr SessionInterface = GetSessionInterface();
	if (!SessionInterface.IsValid())
	{
		return;
	}

	// Session Settings
	FOnlineSessionSettings SessionSettings;
	SessionSettings.bIsLANMatch = bIsLAN;
	SessionSettings.NumPublicConnections = NumPublicConnections;
	SessionSettings.bAllowJoinInProgress = true;
	SessionSettings.bAllowJoinViaPresence = true;
	SessionSettings.bShouldAdvertise = true;
	SessionSettings.bUsesPresence = true;

	// Custom data setting
	SessionSettings.Set(SETTING_MAPNAME, FString("LobbyMap"), EOnlineDataAdvertisementType::ViaOnlineService);

	SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(
		FOnCreateSessionCompleteDelegate::CreateUObject(this, &UDBDSessionInstanceSubsystem::OnCreatedSessionComplete));

	// Request create session
	const FName SessionName = NAME_GameSession;
	if (!SessionInterface->CreateSession(0, SessionName, SessionSettings))
	{
		OnCreateSessionCompleteEvent.Broadcast(false); // Broadcast failed
	}

}

void UDBDSessionInstanceSubsystem::FindSessions(int32 MaxSearchResults, bool bIsLAN)
{
	const IOnlineSessionPtr SessionInterface = GetSessionInterface();
	if (!SessionInterface.IsValid())
	{
		return;
	}

	LastSessionSearch = MakeShareable(new FOnlineSessionSearch());
	LastSessionSearch->bIsLanQuery = bIsLAN;
	LastSessionSearch->MaxSearchResults = MaxSearchResults;

	// PRESENCE Session Search
	LastSessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);

	SessionInterface->AddOnFindSessionsCompleteDelegate_Handle(
		FOnFindSessionsCompleteDelegate::CreateUObject(this, &UDBDSessionInstanceSubsystem::OnFindSessionComplete));

	SessionInterface->FindSessions(0, LastSessionSearch.ToSharedRef());
}

void UDBDSessionInstanceSubsystem::JoinSession(const FBlueprintSessionResult& SessionResult)
{
	const IOnlineSessionPtr SessionInterface = GetSessionInterface();
	if (!SessionInterface.IsValid())
	{
		return;
	}

	SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(
		FOnJoinSessionCompleteDelegate::CreateUObject(this,&UDBDSessionInstanceSubsystem::OnJoinSessionComplete));

	const FName SessionName = NAME_GameSession;
	SessionInterface->JoinSession(0, SessionName, SessionResult.OnlineResult);
}

void UDBDSessionInstanceSubsystem::OnCreatedSessionComplete(FName SessionName, bool bWasSuccessful)
{
	UE_LOG(LogTemp, Warning, TEXT("[Session] Create Session Result: %s"), bWasSuccessful ? TEXT("Success") : TEXT("Failed"));
	
	OnCreateSessionCompleteEvent.Broadcast(bWasSuccessful);
}

void UDBDSessionInstanceSubsystem::OnFindSessionComplete(bool bWasSuccessful)
{
	int32 Count = (bWasSuccessful && LastSessionSearch.IsValid()) ? LastSessionSearch->SearchResults.Num() : 0;
	UE_LOG(LogTemp, Warning, TEXT("[Session] Find Session Result: %s, Found Count: %d"), bWasSuccessful ? TEXT("Success") : TEXT("Failed"), Count);
	
	TArray<FBlueprintSessionResult> BlueprintResults;

	if (bWasSuccessful && LastSessionSearch.IsValid())
	{
		for (const FOnlineSessionSearchResult& Result : LastSessionSearch->SearchResults)
		{
			FBlueprintSessionResult BPResult;
			BPResult.OnlineResult = Result;
			BlueprintResults.Add(BPResult);
		}
	}

	// Broadcast for UI
	OnFindSessionCompleteEvent.Broadcast(BlueprintResults);
}

void UDBDSessionInstanceSubsystem::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	UE_LOG(LogTemp, Warning, TEXT("[Session] Join Session Result Code: %d"), (int32)Result);

	bool bIsSuccess = (Result == EOnJoinSessionCompleteResult::Success);

	if (bIsSuccess)
	{
		const IOnlineSessionPtr SessionInterface = GetSessionInterface();
		if (SessionInterface.IsValid())
		{
			// Travel to Session
			FString ConnectInfo;
			if (SessionInterface->GetResolvedConnectString(SessionName, ConnectInfo))
			{
				APlayerController* PC = GetGameInstance()->GetFirstLocalPlayerController();
				if (PC)
				{
					PC->ClientTravel(ConnectInfo, ETravelType::TRAVEL_Absolute);
				}
			}
		}
	}
	 
	// Broadcast for UI
	OnJoinSessionCompleteEvent.Broadcast(bIsSuccess);
}
