// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Blueprint/UserWidget.h"
#include "DBDGameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "DBDTitlePlayerController.generated.h"

/**
 * 
 */
UCLASS()
class DBDCOPY_API ADBDTitlePlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	void OnChracterSelected(bool bIsKiller);

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "User Widget")
	TSubclassOf<UUserWidget> TitleUserWidgetclass;
	UUserWidget* TitleUserWidget;

private:	
	
};
