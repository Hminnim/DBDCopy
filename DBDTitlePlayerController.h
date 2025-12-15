// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "DBDTitlePlayerController.generated.h"

/**
 * 
 */
UCLASS()
class DBDCOPY_API ADBDTitlePlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	void CreateSession();
	void FindSession();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "User Widget")
	TSubclassOf<UUserWidget> TitleUserWidgetclass;
	UUserWidget* TitleUserWidget;

private:	
	
};
