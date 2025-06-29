// Fill out your copyright notice in the Description page of Project Settings.


#include "DBDSurvivorAnim.h"
#include "Kismet/KismetMathLibrary.h"

void UDBDSurvivorAnim::NativeBeginPlay()
{
	Super::NativeBeginPlay();

	APawn* Pawn = TryGetPawnOwner();
	if (::IsValid(Pawn))
	{
		Survivor = Cast<ADBDSurvivor>(Pawn);
	}
}

void UDBDSurvivorAnim::NativeUpdateAnimation(float DeltaSecond)
{
	Super::NativeUpdateAnimation(DeltaSecond);

	if (Survivor == nullptr)
	{
		return;
	}

	SurvivorSpeed = Survivor->GetVelocity().Size2D();
	bIsCrouched = Survivor->bIsCrouched;
	bIsInteracting = Survivor->bIsInteracting;
	CurrentHealthStateEnum = Survivor->CurrentHealthStateEnum;
}
