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

	// Handling repairing
	if (CurrentInteractionState == ESurvivorInteraction::Repair && bIsInteracting)
	{
		if (ADBDPlayerController* PC = Cast<ADBDPlayerController>(GetController()))
		{
			if (CurrentGenerator)
			{
				PC->ShowInteractionProgress(CurrentGenerator->CurrentRepairRate);

				if (CurrentGenerator->bIsRepaired)
				{
					StopReapirGenerator();
				}
			}
		}
	}
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
	if (CurrentInteractionState == ESurvivorInteraction::Idle)
	{
		return;
	}

	if (CurrentInteractionState == ESurvivorInteraction::Repair)
	{
		if (Value.Get<bool>())
		{
			StartRepairGenerator();
		}
		else
		{
			StopReapirGenerator();
		}
	}
}

void ADBDSurvivor::Action(const FInputActionValue& Value)
{
}

void ADBDSurvivor::FindInteratable()
{
	if (bIsInteracting || bIsActing)
	{
		return;
	}

	FVector LineTraceStart = Camera->GetComponentLocation() + Camera->GetForwardVector();
	FVector LineTraceEnd = (Camera->GetForwardVector() * 300) + LineTraceStart;
	FHitResult HitResult;
	FCollisionQueryParams TraceParams;
	TraceParams.AddIgnoredActor(this);

	if (GetWorld()->LineTraceSingleByChannel(HitResult, LineTraceStart, LineTraceEnd, ECollisionChannel::ECC_Visibility, TraceParams))
	{
		if (HitResult.GetActor())
		{
			if (ADBDGeneratorActor* HitGenerator = Cast<ADBDGeneratorActor>(HitResult.GetActor()))
			{
				if (ADBDPlayerController* PC = Cast<ADBDPlayerController>(GetController()))
				{
					if (HitGenerator->bIsRepaired)
					{
						return;
					}

					CurrentInteractionState = ESurvivorInteraction::Repair;
					CurrentGenerator = HitGenerator;
					PC->ShowIneractionMessage("Press M1 to repair the generator");
					PC->ShowInteractionProgress(CurrentGenerator->CurrentRepairRate);
					return;
				}
			}
		}
	}
	if (ADBDPlayerController* PC = Cast<ADBDPlayerController>(GetController()))
	{
		CurrentInteractionState = ESurvivorInteraction::Idle;
		PC->HideInteractionMessage();
		PC->HideInteractionProgress();
	}
}

void ADBDSurvivor::StartRepairGenerator()
{
	if (!CurrentGenerator || CurrentInteractionState != ESurvivorInteraction::Repair)
	{
		return;
	}
	if (ADBDPlayerController* PC = Cast<ADBDPlayerController>(GetController()))
	{
		PC->HideInteractionMessage();
	}

	// Find closest location to repair
	FVector RepairLocation = CurrentGenerator->RepairLocation[0];
	double MinDistance = FVector::Distance(GetActorLocation(), RepairLocation);
	for (int i = 1; i < 4; i++) 
	{
		double NextDistance = FVector::Distance(GetActorLocation(), CurrentGenerator->RepairLocation[i]);
		if (MinDistance > NextDistance)
		{
			RepairLocation = CurrentGenerator->RepairLocation[i];
			MinDistance = NextDistance;
		}
	}
	RepairLocation.Z = GetActorLocation().Z;
	SetActorLocation(RepairLocation, false);
	
	// Set rotation to generator
	FRotator RepairRotation = (CurrentGenerator->GetActorLocation() - GetActorLocation()).Rotation();
	RepairRotation.Pitch = 0.0f;
	RepairRotation.Roll = 0.0f;
	SetActorRotation(RepairRotation);

	bIsInteracting = true;
	CurrentGenerator->CurrentRepairingSurvivor = FMath::Clamp(++CurrentGenerator->CurrentRepairingSurvivor, 0, 4);

	if (ADBDPlayerController* PC = Cast<ADBDPlayerController>(GetController()))
	{
		PC->ShowSkillCheck();
	}
}

void ADBDSurvivor::StopReapirGenerator()
{
	bIsInteracting = false;
	CurrentGenerator->CurrentRepairingSurvivor = FMath::Clamp(--CurrentGenerator->CurrentRepairingSurvivor, 0, 4);
}
