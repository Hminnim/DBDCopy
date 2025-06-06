// Fill out your copyright notice in the Description page of Project Settings.


#include "DBDKillerAnim.h"

void UDBDKillerAnim::NativeBeginPlay()
{
	Super::NativeBeginPlay();

	APawn* Pawn = TryGetPawnOwner();
	if (::IsValid(Pawn))
	{
		Killer = Cast<ADBDKiller>(Pawn);
	}
}

void UDBDKillerAnim::NativeUpdateAnimation(float DeltaSecond)
{
	Super::NativeUpdateAnimation(DeltaSecond);

	if (Killer == nullptr)
	{
		return;
	}

	KillerSpeed = Killer->GetVelocity().Size2D();
}
