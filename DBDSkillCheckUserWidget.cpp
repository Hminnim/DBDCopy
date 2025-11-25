// Fill out your copyright notice in the Description page of Project Settings.


#include "DBDSkillCheckUserWidget.h"
#include "Math/UnrealMathUtility.h"

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
			GoodCircleImage->SetVisibility(ESlateVisibility::Visible);
		}

		if (GreatCircle && GreatCircleImage)
		{
			GreatCircle->SetScalarParameterValue(FName("Percent"), 0.97f);
			GreatCircleImage->SetBrushFromMaterial(GreatCircle);
			GreatCircleImage->SetVisibility(ESlateVisibility::Visible);
		}

		if (GoodCircle && GoodCircleImage_1)
		{
			GoodCircleImage_1->SetVisibility(ESlateVisibility::Hidden);
		}

		if (GreatCircle && GreatCircleImage_1)
		{
			GreatCircleImage_1->SetVisibility(ESlateVisibility::Hidden);
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

	PointerAngle = Pointer->GetRenderTransformAngle();

	// Generator skill check
	if (bIsGenerator)
	{
		AngleElapsed += InDeltaTime;

		float Angle = FMath::Clamp(AngleElapsed / 1.1f, 0.0f, 1.0f) * 360.0f;
		Pointer->SetRenderTransformAngle(Angle);

		if (Angle >= 360.0f)
		{
			StopGeneratorSkillCheck();
		}
	}
		
	// Wiggle skill check
	if (bIsWiggle)
	{
		float RotationSpeed = 360.0f / 1.1f;
		float DeltaRotation = RotationSpeed * InDeltaTime;

		if (bMoveClock)
		{
			WiggleAngle += DeltaRotation;
		}
		else
		{
			WiggleAngle -= DeltaRotation;
		}
		
		WiggleAngle = FMath::Fmod(WiggleAngle, 360.0f);

		if (WiggleAngle < 0.0f)
		{
			WiggleAngle += 360.0f;
		}

		Pointer->SetRenderTransformAngle(WiggleAngle);
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
		GoodCircleImage->SetVisibility(ESlateVisibility::Visible);
	}

	if (GreatCircle && GreatCircleImage)
	{
		GreatCircle->SetScalarParameterValue(FName("Percent"), 0.97f);
		GreatCircleImage->SetBrushFromMaterial(GreatCircle);
		GreatCircleImage->SetVisibility(ESlateVisibility::Visible);
	}

	if (GoodCircle && GoodCircleImage_1)
	{
		GoodCircleImage_1->SetVisibility(ESlateVisibility::Hidden);
	}

	if (GreatCircle && GreatCircleImage_1)
	{
		GreatCircleImage_1->SetVisibility(ESlateVisibility::Hidden);
	}

	CircleAngle = FMath::RandRange(120.0f, 300.0f);
	Circles->SetRenderTransformAngle(CircleAngle);
}

void UDBDSkillCheckUserWidget::SetWiggleSKillCheck()
{
	Circles->SetRenderTransformAngle(0.0f);

	// 3 o'clock
	if (GoodCircle && GoodCircleImage)
	{
		GoodCircle->SetScalarParameterValue(FName("Percent"), WiggleGoodCirclePercent);
		GoodCircleImage->SetBrushFromMaterial(GoodCircle);
		GoodCircleImage->SetRenderTransformAngle(GetWiggleCircleAngle(45.0f, WiggleGoodCirclePercent));
		GoodCircleImage->SetVisibility(ESlateVisibility::Visible);
	}
	if (GreatCircle && GreatCircleImage)
	{
		GreatCircle->SetScalarParameterValue(FName("Percent"), WiggleGreatCirclePercent);
		GreatCircleImage->SetBrushFromMaterial(GreatCircle);
		GreatCircleImage->SetRenderTransformAngle(GetWiggleCircleAngle(45.0f, WiggleGreatCirclePercent));
		GreatCircleImage->SetVisibility(ESlateVisibility::Visible);
	}

	// 9 o'clock
	if (GoodCircle && GoodCircleImage_1)
	{
		GoodCircle->SetScalarParameterValue(FName("Percent"), WiggleGoodCirclePercent);
		GoodCircleImage_1->SetBrushFromMaterial(GoodCircle);
		GoodCircleImage_1->SetRenderTransformAngle(GetWiggleCircleAngle(225.0f, WiggleGoodCirclePercent));
		GoodCircleImage_1->SetVisibility(ESlateVisibility::Visible);
	}
	if (GreatCircle && GreatCircleImage_1)
	{
		GreatCircle->SetScalarParameterValue(FName("Percent"), WiggleGreatCirclePercent);
		GreatCircleImage_1->SetBrushFromMaterial(GreatCircle);
		GreatCircleImage_1->SetRenderTransformAngle(GetWiggleCircleAngle(225.0f, WiggleGreatCirclePercent));
		GreatCircleImage_1->SetVisibility(ESlateVisibility::Visible);
	}
}

void UDBDSkillCheckUserWidget::StartGeneratorSkillCheck()
{
	if (bIsMoving || !Pointer)
	{
		return;
	}

	bIsMoving = true;
	bIsGenerator = true;
	AngleElapsed = 0.0f;
}

void UDBDSkillCheckUserWidget::StopGeneratorSkillCheck()
{
	bIsMoving = false;
	bIsGenerator = false;
}

void UDBDSkillCheckUserWidget::StartWiggleSkillCheck()
{
	if (bIsMoving || !Pointer)
	{
		return;
	}

	bIsMoving = true;
	bIsWiggle = true;
	bMoveClock = true;
	AngleElapsed = 0.0f;
}

void UDBDSkillCheckUserWidget::StopWiggleSkillCheck()
{
	bIsMoving = false;
	bIsWiggle = false;
}

void UDBDSkillCheckUserWidget::ChangeWiggleGreatSkillCheckPercent(float NewPercent)
{
	if (GreatCircle && GreatCircleImage)
	{
		GreatCircle->SetScalarParameterValue(FName("Percent"), NewPercent);
		GreatCircleImage->SetBrushFromMaterial(GreatCircle);
		GreatCircleImage->SetRenderTransformAngle(GetWiggleCircleAngle(45.0f, NewPercent));
	}
	if (GreatCircle && GreatCircleImage_1)
	{
		GreatCircle->SetScalarParameterValue(FName("Percent"), NewPercent);
		GreatCircleImage_1->SetBrushFromMaterial(GreatCircle);
		GreatCircleImage_1->SetRenderTransformAngle(GetWiggleCircleAngle(225.0f, NewPercent));
	}
}

void UDBDSkillCheckUserWidget::OnSucceededWiggleGreatSkillCheck()
{
	WiggleGreatCount += 1;

	float Alpha = (float)WiggleGreatCount / 20.0f;
	float NewPercent = FMath::Lerp(WiggleGreatCirclePercent, 0.975f, Alpha);
	WiggleSkillCheckTarget = (WiggleSkillCheckTarget + 1) % 2;

	ChangeWiggleGreatSkillCheckPercent(NewPercent);
}

void UDBDSkillCheckUserWidget::OnFailedWiggleGreatSkillCheck()
{
	WiggleGreatCount = 0;

	float Alpha = (float)WiggleGreatCount / 20.0f;
	float NewPercent = FMath::Lerp(WiggleGreatCirclePercent, 0.975f, Alpha);
	WiggleSkillCheckTarget = (WiggleSkillCheckTarget + 1) % 2;

	ChangeWiggleGreatSkillCheckPercent(NewPercent);
}

void UDBDSkillCheckUserWidget::OnSucceededWiggleSkillCheck()
{
	if (GreatCircleImage)
	{
		GreatCircleImage->SetColorAndOpacity(SuccessCircleColor);
	}
	if (GreatCircleImage_1)
	{
		GreatCircleImage_1->SetColorAndOpacity(SuccessCircleColor);
	}

	GetWorld()->GetTimerManager().SetTimer(
		ColorChangeTimerHandle,
		this,
		&UDBDSkillCheckUserWidget::ChangeCircleColorToNormal,
		0.15f,
		false
	);
}

void UDBDSkillCheckUserWidget::OnFailedWiggleSkillCheck()
{
	if (GreatCircleImage)
	{
		GreatCircleImage->SetColorAndOpacity(FailCircleColor);
	}
	if (GreatCircleImage_1)
	{
		GreatCircleImage_1->SetColorAndOpacity(FailCircleColor);
	}

	GetWorld()->GetTimerManager().SetTimer(
		ColorChangeTimerHandle,
		this,
		&UDBDSkillCheckUserWidget::ChangeCircleColorToNormal,
		0.15f,
		false
	);
}

void UDBDSkillCheckUserWidget::ChangePointerMoveDirection()
{
	bMoveClock = !bMoveClock;
}

bool UDBDSkillCheckUserWidget::GetPointerMoveDirection() const
{
	return bMoveClock;
}

int8 UDBDSkillCheckUserWidget::GetWiggleSkillCheckTarget() const
{
	return WiggleSkillCheckTarget;
}

float UDBDSkillCheckUserWidget::GetWiggleGoodCirclePercent() const
{
	return WiggleGoodCirclePercent;
}

float UDBDSkillCheckUserWidget::GetWiggleGreatCirclePercent() const
{
	return WiggleGreatCirclePercent;
}

float UDBDSkillCheckUserWidget::GetWiggleCircleAngle(float targetAngle, float wiggleCirclePercent)
{
	return 360.f * (1.0f - wiggleCirclePercent) / 2 + targetAngle;
}

void UDBDSkillCheckUserWidget::ChangeCircleColorToNormal()
{
	if (GreatCircleImage)
	{
		GreatCircleImage->SetColorAndOpacity(NormalCircleColor);
	}
	if (GreatCircleImage_1)
	{
		GreatCircleImage_1->SetColorAndOpacity(NormalCircleColor);
	}
}

