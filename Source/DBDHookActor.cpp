// Fill out your copyright notice in the Description page of Project Settings.


#include "DBDHookActor.h"

// Sets default values
ADBDHookActor::ADBDHookActor()
{
	bReplicates = true;

	// HookStaticMesh default values
	HookStaticMesh = CreateDefaultSubobject<UStaticMeshComponent>("Hook Static Mesh");
	HookStaticMesh->SetupAttachment(RootComponent);
	static ConstructorHelpers::FObjectFinder<UStaticMesh>SMesh(TEXT("'/Game/Games/Object/hook/source/hook_dbd_low.hook_dbd_low'"));
	if (SMesh.Succeeded())
	{
		HookStaticMesh->SetStaticMesh(SMesh.Object);
	}
	HookStaticMesh->SetRelativeScale3D(FVector({ 0.35f,0.35f,0.35f }));
	HookStaticMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	HookStaticMesh->SetCollisionObjectType(ECC_WorldDynamic);
	HookStaticMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	HookStaticMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	HookStaticMesh->SetRenderCustomDepth(true);
	HookStaticMesh->SetCustomDepthStencilValue(6);

	// CollisionComponent default values
	CollisionComponent = CreateDefaultSubobject<UBoxComponent>("Collision Component");
	CollisionComponent->SetupAttachment(HookStaticMesh);
	CollisionComponent->SetRelativeScale3D(FVector({ 3.0f, 3.0f,15.0f }));
	CollisionComponent->SetRelativeLocation(FVector({ 0.0f,0.0f,360.0f }));
	CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CollisionComponent->SetCollisionObjectType(ECC_WorldDynamic);
	CollisionComponent->SetCollisionResponseToAllChannels(ECR_Block);
	CollisionComponent->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);

	// HitComponent default values
	HitComponent = CreateDefaultSubobject<UBoxComponent>("Hit Component");
	HitComponent->SetupAttachment(HookStaticMesh);
	HitComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	HitComponent->SetCollisionObjectType(ECC_WorldDynamic);
	HitComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	HitComponent->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	HitComponent->SetRelativeScale3D(FVector({ 5.0f,5.0f,5.0f }));
	HitComponent->SetRelativeLocation(FVector({ 0.0f,-520.0f,760.0f }));
}

// Called when the game starts or when spawned
void ADBDHookActor::BeginPlay()
{
	Super::BeginPlay();
	

}

void ADBDHookActor::OnSurvivorHooked()
{
	bHasHookedSuvivor = true;

	HookStaticMesh->SetCustomDepthStencilValue(7);
}

void ADBDHookActor::OnSurvivorUnHooked()
{
	bHasHookedSuvivor = false;

	HookStaticMesh->SetCustomDepthStencilValue(6);
}

