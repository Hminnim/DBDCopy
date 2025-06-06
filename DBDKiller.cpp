// Fill out your copyright notice in the Description page of Project Settings.


#include "DBDKiller.h"

// Sets default values
ADBDKiller::ADBDKiller()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Set default character movement
	bUseControllerRotationYaw = true;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->bUseControllerDesiredRotation = true;
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;

	// Camera config
	Camera = CreateDefaultSubobject<UCameraComponent>("Camera");
	Camera->SetupAttachment(RootComponent);
	Camera->SetRelativeLocation(FVector(20.0f, 0.0f, 70.0f));
	Camera->bUsePawnControlRotation = true;

	// Mesh config
	GetMesh()->SetOwnerNoSee(true);
}

// Called when the game starts or when spawned
void ADBDKiller::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ADBDKiller::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ADBDKiller::NotifyControllerChanged()
{
	Super::NotifyControllerChanged();

	// Add input mapping context
	if (APlayerController* PC = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}

// Called to bind functionality to input
void ADBDKiller::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Bind input action
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(MoveInput, ETriggerEvent::Triggered, this, &ADBDKiller::Move);
		EnhancedInputComponent->BindAction(LookInput, ETriggerEvent::Triggered, this, &ADBDKiller::Look);
		EnhancedInputComponent->BindAction(InteractInput, ETriggerEvent::Triggered, this, &ADBDKiller::Interact);
		EnhancedInputComponent->BindAction(ActionInput, ETriggerEvent::Triggered, this, &ADBDKiller::Action);
	}
}

void ADBDKiller::Move(const FInputActionValue& Value)
{
	const FVector2D MovementValue = Value.Get<FVector2D>();

	const FRotator ControllerRotation = Controller->GetControlRotation();
	const FRotator YaRotation(0.0f, ControllerRotation.Yaw, 0.0f);

	const FVector ForwardDirection = FRotationMatrix(YaRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YaRotation).GetUnitAxis(EAxis::Y);

	AddMovementInput(ForwardDirection, MovementValue.Y);
	AddMovementInput(RightDirection, MovementValue.X);
}

void ADBDKiller::Look(const FInputActionValue& Value)
{
	const FVector2D LookValue = Value.Get<FVector2D>();

	AddControllerYawInput(LookValue.X);
	AddControllerPitchInput(-LookValue.Y);
}

void ADBDKiller::Interact(const FInputActionValue& Value)
{
}

void ADBDKiller::Action(const FInputActionValue& Value)
{
}