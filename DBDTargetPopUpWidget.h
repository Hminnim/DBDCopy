// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DBDTargetPopUpWidget.generated.h"

/**
 * 
 */

class UImage;

UCLASS()
class DBDCOPY_API UDBDTargetPopUpWidget : public UUserWidget
{
	GENERATED_BODY()
	
private:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

public:
	void SetTargetActor(AActor* NewTarget, int32 NewType);

protected:
	UPROPERTY(meta = (BindWidget))
	UImage* TargetImage;

	UPROPERTY(EditAnywhere, Category = "PopUp Sprite")
	UTexture2D* ExitTexture;

	UPROPERTY()
	TWeakObjectPtr<AActor> TargetActor;
	int32 TargetType = int32(0);

	UFUNCTION()
	void UpdateWidgetPosition();

	FTimerHandle UpdateTimerHandle;
};
