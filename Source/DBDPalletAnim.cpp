// Fill out your copyright notice in the Description page of Project Settings.


#include "DBDPalletAnim.h"

void UDBDPalletAnim::NativeBeginPlay()
{
	Super::NativeBeginPlay();

	AActor* Actor = GetOwningActor();
	if (::IsValid(Actor))
	{
		Pallet = Cast<ADBDPalletActor>(Actor);
	}
}

void UDBDPalletAnim::NativeUpdateAnimation(float DeltaSecond)
{
	Super::NativeUpdateAnimation(DeltaSecond);

	if (Pallet == nullptr)
	{
		return;
	}

	bIsDropped = Pallet->bIsDropped;
}
