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
	void SetWiggleSKillCheck();
	void SetStruggleSkillCheck(int8 Count);

	UFUNCTION()
	void StartGeneratorSkillCheck();
	UFUNCTION()
	void StopGeneratorSkillCheck();
	UFUNCTION()
	void StartWiggleSkillCheck();
	UFUNCTION()
	void StopWiggleSkillCheck();
	UFUNCTION()
	void StartStruggleSkillCheck();
	UFUNCTION()
	void StopStruggleSkillCheck();

	UFUNCTION()
	void ChangeWiggleGreatSkillCheckPercent(float NewPercent);
	void OnSucceededWiggleGreatSkillCheck();
	void OnFailedWiggleGreatSkillCheck();
	void OnSucceededWiggleSkillCheck();
	void OnFailedWiggleSkillCheck();

	void ChangePointerMoveDirection();
	bool GetPointerMoveDirection() const;
	int8 GetWiggleSkillCheckTarget() const;
	float GetWiggleGoodCirclePercent() const;
	float GetWiggleGreatCirclePercent() const;

	float PointerAngle = 0.0f;
	float CircleAngle = 0.0f;
	float StruggleSuccessPercent = 0.82f;
	float CurrentStruggleSuccessPercent = 0.0f;

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
	UImage* GoodCircleImage_1;
	UPROPERTY(meta = (BindWidget)) 
	UImage* GreatCircleImage_1;
	UPROPERTY(meta = (BindWidget))
	UVerticalBox* Pointer;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Circle Color")
	FLinearColor NormalCircleColor = FLinearColor::White;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Circle Color")
	FLinearColor SuccessCircleColor = FLinearColor::Yellow;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Circle Color")
	FLinearColor FailCircleColor = FLinearColor::Red;

private:
	bool bIsMoving = false;
	bool bMoveClock = true;
	bool bIsGenerator = false;
	bool bIsWiggle = false;
	bool bIsStruggle = false;
	float AngleElapsed = 0.0f;
	float WiggleAngle = 0.0f;
	float WiggleGoodCirclePercent = 0.76f;
	float WiggleGreatCirclePercent = 0.94f;
	int8 WiggleSkillCheckTarget = 0;
	int32 WiggleGreatCount = 0;
	FTimerHandle ColorChangeTimerHandle;

	float GetWiggleCircleAngle(float targetAngle, float wiggleCirclePercent);
	void ChangeCircleColorToNormal();
};
