// Fill out your copyright notice in the Description page of Project Settings.


#include "DBDSkillCheckUserWidget.h"

void UDBDSkillCheckUserWidget::NativePreConstruct()
{
	Super::NativePreConstruct();

	if (CheckCircle)
	{
		GoodCircle = UMaterialInstanceDynamic::Create(CheckCircle, this);
		GreatCircle = UMaterialInstanceDynamic::Create(CheckCircle, this);

		if (GoodCircle && GoodCircleImage)
		{
			GoodCircle->SetScalarParameterValue(FName("Percent"), 0.87f);
			GoodCircleImage->SetBrushFromMaterial(GoodCircle);
		}

		if (GreatCircle && GreatCircleImage)
		{
			GreatCircle->SetScalarParameterValue(FName("Percent"), 0.97f);
			GreatCircleImage->SetBrushFromMaterial(GreatCircle);
		}
	}
}

void UDBDSkillCheckUserWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (!bIsMoving || !Pointer)
	{
		return;
	}

	AngleElapsed += InDeltaTime;
	
	float Angle = FMath::Clamp(AngleElapsed / 1.1f, 0.0f, 1.0f) * 360.0f;
	Pointer->SetRenderTransformAngle(Angle);

	if (Angle >= 360.0f)
	{
		StopPointerMove();
	}
}

bool UDBDSkillCheckUserWidget::Initialize()
{
	Super::Initialize();

	return true;
}

void UDBDSkillCheckUserWidget::SetGeneratorSkillCheck()
{
	if (GoodCircle && GoodCircleImage)
	{
		GoodCircle->SetScalarParameterValue(FName("Percent"), 0.87f);
		GoodCircleImage->SetBrushFromMaterial(GoodCircle);
	}

	if (GreatCircle && GreatCircleImage)
	{
		GreatCircle->SetScalarParameterValue(FName("Percent"), 0.97f);
		GreatCircleImage->SetBrushFromMaterial(GreatCircle);
	}

	CircleAngle = FMath::RandRange(120.0f, 300.0f);
	Circles->SetRenderTransformAngle(CircleAngle);
}

void UDBDSkillCheckUserWidget::StartPointerMove()
{
	if (bIsMoving || !Pointer)
	{
		return;
	}

	bIsMoving = true;
	AngleElapsed = 0.0f;

	// Pointer will move 1.1 second
	GetWorld()->GetTimerManager().SetTimer
	(
		SkillCheckTimerHandle,
		this,
		&UDBDSkillCheckUserWidget::StopPointerMove,
		1.1f,
		false
	);
}

void UDBDSkillCheckUserWidget::StopPointerMove()
{
	bIsMoving = false;
	GetWorld()->GetTimerManager().ClearTimer(SkillCheckTimerHandle);
}

int8 UDBDSkillCheckUserWidget::GetSkillCheckResult()
{
	float PointerAngle = Pointer->GetRenderTransformAngle();
	if (PointerAngle < 0.0f)
	{
		PointerAngle += 360.0f;
	}

	// Great skill check
	if (CircleAngle <= PointerAngle && PointerAngle < CircleAngle + 10.8)
	{
		return 0;
	}

	// Good skill check
	else if (CircleAngle + 10.8 <= PointerAngle && PointerAngle < CircleAngle + 46.8)
	{
		return 1;
	}

	// Failed skill check
	else
	{
		return 2;
	}

}

