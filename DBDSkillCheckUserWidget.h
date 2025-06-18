// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Materials/Material.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Components/Image.h"
#include "Components/VerticalBox.h"
#include "Components/SizeBox.h"
#include "TimerManager.h"
#include "DBDSkillCheckUserWidget.generated.h"

/**
 * 
 */
UCLASS()
class DBDCOPY_API UDBDSkillCheckUserWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	virtual void NativePreConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	virtual bool Initialize() override;

	void SetGeneratorSkillCheck();

	UFUNCTION()
	void StartPointerMove();
	UFUNCTION()
	void StopPointerMove();
	int8 GetSkillCheckResult();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material")
	UMaterialInterface* Circle;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material")
	UMaterialInterface* CheckCircle;
	UPROPERTY()
	UMaterialInstanceDynamic* GoodCircle;
	UPROPERTY()
	UMaterialInstanceDynamic* GreatCircle;
	UPROPERTY(meta = (BindWidget))
	USizeBox* Circles;
	UPROPERTY(meta = (BindWidget))
	UImage* CircleImage;
	UPROPERTY(meta = (BindWidget))
	UImage* GoodCircleImage;
	UPROPERTY(meta = (BindWidget))
	UImage* GreatCircleImage;
	UPROPERTY(meta = (BindWidget))
	UVerticalBox* Pointer;

private:
	bool bIsMoving = false;
	float AngleElapsed = 0.0f;
	float CircleAngle = 0.0f;
	FTimerHandle SkillCheckTimerHandle;
};
