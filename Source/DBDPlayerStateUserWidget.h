// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DBDMainPlayerState.h"
#include "DBDPlayerStateUserWidget.generated.h"

/**
 * 
 */
class UTextBlock;
class UImage;
class UProgressBar;

UCLASS()
class DBDCOPY_API UDBDPlayerStateUserWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime);
	void SetUpPlayerState(FString PlaayerName);
	UFUNCTION()
	void ChangeHealthStateImage(EHealthState NewState);
	void Setup(ADBDMainPlayerState* InPlayerState);

protected:
	UPROPERTY(meta = (BindWidget))
	UImage* CurrentHealthStateImage;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* PlayerNameText;
	UPROPERTY(meta = (BindWidget))
	UProgressBar* HookStageRateProgressBar;
	
	UPROPERTY(EditAnywhere, Category = "HUD Sprite")
	UTexture2D* HealthyTexture;
	UPROPERTY(EditAnywhere, Category = "HUD Sprite")
	UTexture2D* InjuredTexture;
	UPROPERTY(EditAnywhere, Category = "HUD Sprite")
	UTexture2D* CrawlTexture;
	UPROPERTY(EditAnywhere, Category = "HUD Sprite")
	UTexture2D* CarryingTexture;
	UPROPERTY(EditAnywhere, Category = "HUD Sprite")
	UTexture2D* HookedTexture;
	UPROPERTY(EditAnywhere, Category = "HUD Sprite")
	UTexture2D* DeathTexture;
	UPROPERTY(EditAnywhere, Category = "HUD Sprite")
	UTexture2D* ExitTexture;

private:
	UPROPERTY()
	TObjectPtr<ADBDMainPlayerState> PlayerStatePtr;

	bool bIsHooked = false;
};
