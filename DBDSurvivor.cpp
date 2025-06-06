// Fill out your copyright notice in the Description page of Project Settings.


#include "DBDSurvivor.h"

// Sets default values
ADBDSurvivor::ADBDSurvivor()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Set default character movement
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->bUseControllerDesiredRotation = true;
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	GetCharacterMovement()->MaxWalkSpeedCrouched = CrouchSpeed;
	GetCharacterMovement()->GetNavAgentPropertiesRef().bCanCrouch = true;

	// SpringArm config
	SpringArm = CreateDefaultSubobject<USpringArmComponent>("SpringArm");
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->bUsePawnControlRotation = true;
	SpringArm->SetRelativeLocation(FVector(0.0f, 0.0f, 50.0f));
	SpringArm->TargetArmLength = 200.0f;

	// Camera config
	Camera = CreateDefaultSubobject<UCameraComponent>("Camera");
	Camera->SetupAttachment(SpringArm);
	Camera->bUsePawnControlRotation = false;
}

// Called when the game starts or when spawned
void ADBDSurvivor::BeginPlay()
{
	Super::BeginPlay();

}

// Called every frame
void ADBDSurvivor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	FindInteratable();
}

void ADBDSurvivor::NotifyControllerChanged()
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
void ADBDSurvivor::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Bind input action
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(MoveInput, ETriggerEvent::Triggered, this, &ADBDSurvivor::Move);
		EnhancedInputComponent->BindAction(LookInput, ETriggerEvent::Triggered, this, &ADBDSurvivor::Look);
		EnhancedInputComponent->BindAction(CrouchInput, ETriggerEvent::Triggered, this, &ADBDSurvivor::HandleCrouch);
		EnhancedInputComponent->BindAction(SprintInput, ETriggerEvent::Triggered, this, &ADBDSurvivor::Sprint);
		EnhancedInputComponent->BindAction(InteractInput, ETriggerEvent::Triggered, this, &ADBDSurvivor::Interact);
		EnhancedInputComponent->BindAction(ActionInput, ETriggerEvent::Triggered, this, &ADBDSurvivor::Action);
	}
}

void ADBDSurvivor::Move(const FInputActionValue& Value)
{
	if (bIsInteracting || bIsActing)
	{
		return;
	}

	const FVector2D MovementValue = Value.Get<FVector2D>();

	const FRotator ControllerRotation = Controller->GetControlRotation();
	const FRotator YaRotation(0.0f, ControllerRotation.Yaw, 0.0f);

	const FVector ForwardDirection = FRotationMatrix(YaRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YaRotation).GetUnitAxis(EAxis::Y);

	AddMovementInput(ForwardDirection, MovementValue.Y);
	AddMovementInput(RightDirection, MovementValue.X);
}

void ADBDSurvivor::Look(const FInputActionValue& Value)
{
	const FVector2D LookValue = Value.Get<FVector2D>();

	AddControllerYawInput(LookValue.X);
	AddControllerPitchInput(-LookValue.Y);
}

void ADBDSurvivor::HandleCrouch(const FInputActionValue& Value)
{
	if (bIsInteracting || bIsActing)
	{
		return;
	}

	if (Value.Get<bool>())
	{
		Crouch();
	}
	else
	{
		UnCrouch();
	}
}

void ADBDSurvivor::Sprint(const FInputActionValue& Value)
{
	if (bIsInteracting || bIsActing)
	{
		return;
	}

	bIsSprinting = Value.Get<bool>();
	if (bIsSprinting && !bIsCrouched)
	{
		GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;
	}
	else
	{
		GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	}
}

void ADBDSurvivor::Interact(const FInputActionValue& Value)
{
}

void ADBDSurvivor::Action(const FInputActionValue& Value)
{
}

void ADBDSurvivor::FindInteratable()
{
	FVector LineTraceStart = Camera->GetComponentLocation() + Camera->GetForwardVector();
	FVector LineTraceEnd = (Camera->GetForwardVector() * 400) + LineTraceStart;

	FHitResult HitResult;
	if (GetWorld()->LineTraceSingleByChannel(HitResult, LineTraceStart, LineTraceEnd, ECollisionChannel::ECC_Visibility))
	{
		if (HitResult.GetActor())
		{
			if (ADBDGeneratorActor* HitGenerator = Cast<ADBDGeneratorActor>(HitResult.GetActor()))
			{
				if (ADBDPlayerController* PC = Cast<ADBDPlayerController>(GetController()))
				{
					PC->ShowIneractionMessage("Press M1 to repair the generator");
				}
			}
		}
	}
}
