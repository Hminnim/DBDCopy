// Fill out your copyright notice in the Description page of Project Settings.


#include "DBDGeneratorAnim.h"

void UDBDGeneratorAnim::NativeBeginPlay()
{
	Super::NativeBeginPlay();

	AActor* Actor = GetOwningActor();
	if (::IsValid(Actor))
	{
		Generator = Cast<ADBDGeneratorActor>(Actor);
	}
}

void UDBDGeneratorAnim::NativeUpdateAnimation(float DeltaSecond)
{
	Super::NativeUpdateAnimation(DeltaSecond);

	if (Generator == nullptr)
	{
		return;
	}

	CurrentRepairRate = Generator->CurrentRepairRate;
}
