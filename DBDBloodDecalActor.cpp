// Fill out your copyright notice in the Description page of Project Settings.


#include "DBDBloodDecalActor.h"

// Sets default values
ADBDBloodDecalActor::ADBDBloodDecalActor()
{
	// Decal component
	if (!DecalComponent)
	{
		DecalComponent = CreateDefaultSubobject<UDecalComponent>("Decal Component");
		DecalComponent->SetupAttachment(RootComponent);
		
	}
}
