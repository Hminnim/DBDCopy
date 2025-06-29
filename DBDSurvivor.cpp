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

void ADBDSurvivor::BeginOverlapWindow()
{
	if (CurrentHealthStateEnum == EHealthState::DeepWound)
	{
		return;
	}

	if (ADBDPlayerController* PC = Cast<ADBDPlayerController>(GetController()))
	{
		PC->ShowActionMessage("Press Space to Vault");
	}
	bCanVault = true;
}

void ADBDSurvivor::EndOverlapWindow()
{
	if (ADBDPlayerController* PC = Cast<ADBDPlayerController>(GetController()))
	{
		PC->HideActionMessage();
	}
	bCanVault = false;
}

void ADBDSurvivor::SetCurrentWindow(ADBDWindowActor* Target)
{
	CurrentWindow = Target;
}

void ADBDSurvivor::OnTakeDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser)
{
	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, FString::Printf(TEXT("OnTakeDamge")));
	if (CurrentHealthStateEnum == EHealthState::Healthy)
	{
		CurrentHealthStateEnum = EHealthState::Injured;
		return;
	}
	else if (CurrentHealthStateEnum == EHealthState::Injured)
	{
		CurrentHealthStateEnum = EHealthState::DeepWound;
		GetCharacterMovement()->MaxWalkSpeed = CrawlSpeed;

		if (bIsInteracting)
		{
			if (CurrentInteractionState == ESurvivorInteraction::Repair)
			{
				StopReapirGenerator();
			}
		}
		if (bIsCrouched)
		{
			UnCrouch();
		}
		return;
	}
}

// Called when the game starts or when spawned
void ADBDSurvivor::BeginPlay()
{
	Super::BeginPlay();

	// function bind
	OnTakeAnyDamage.AddDynamic(this, &ADBDSurvivor::OnTakeDamage);
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
	if (bIsInteracting || bIsActing || CurrentHealthStateEnum == EHealthState::DeepWound)
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
	if (bIsInteracting || bIsActing || CurrentHealthStateEnum == EHealthState::DeepWound)
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
	if (CurrentInteractionState == ESurvivorInteraction::Idle || CurrentHealthStateEnum == EHealthState::DeepWound)
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
	// Skill check
	if (CurrentInteractionState == ESurvivorInteraction::Repair)
	{
		if (ADBDPlayerController* PC = Cast<ADBDPlayerController>(GetController()))
		{
			if (PC->bIsSkillChecking)
			{
				HandleSkillCheck(PC->GetSkillCheckResult());
			}
		}
	}

	if (bCanVault)
	{
		StartVault();
	}
}

void ADBDSurvivor::FindInteratable()
{
	if (bIsInteracting || bIsActing || CurrentHealthStateEnum == EHealthState::DeepWound)
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
					PC->ShowIneractionMessage("Press M1 to repair");
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
	if (!CurrentGenerator || CurrentInteractionState != ESurvivorInteraction::Repair || CurrentHealthStateEnum == EHealthState::DeepWound)
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

	// Try trigger skillcheck
	GetWorld()->GetTimerManager().SetTimer
	(
		SkillCheckTriggerTimer,
		this,
		&ADBDSurvivor::TryTriggerSkillCheck,
		1.0f,
		true,
		1.0f
	);
}

void ADBDSurvivor::StopReapirGenerator()
{
	bIsInteracting = false;
	CurrentGenerator->CurrentRepairingSurvivor = FMath::Clamp(--CurrentGenerator->CurrentRepairingSurvivor, 0, 4);
	if (ADBDPlayerController* PC = Cast<ADBDPlayerController>(GetController()))
	{
		if (PC->bIsSkillChecking)
		{
			PC->StopSkillCheck();
			HandleSkillCheck(int8(2));
		}
	}
	GetWorld()->GetTimerManager().ClearTimer(SkillCheckTriggerTimer);
}

void ADBDSurvivor::StartSkillCheck()
{
	if (ADBDPlayerController* PC = Cast<ADBDPlayerController>(GetController()))
	{
		PC->ShowSkillCheck();
	}

	GetWorld()->GetTimerManager().SetTimer
	(
		SkillCheckTimer,
		this,
		&ADBDSurvivor::FailedSkillCheck,
		1.6f,
		false
	);
}

void ADBDSurvivor::FailedSkillCheck()
{
	if (ADBDPlayerController* PC = Cast<ADBDPlayerController>(GetController()))
	{
		PC->StopSkillCheck();
	}

	HandleSkillCheck((int8)2);
}

void ADBDSurvivor::HandleSkillCheck(int8 Type)
{
	GetWorld()->GetTimerManager().ClearTimer(SkillCheckTimer);

	// Great skill check
	if (Type == int8(0))
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, FString::Printf(TEXT("Great skill check")));

		if (CurrentGenerator != nullptr && CurrentInteractionState == ESurvivorInteraction::Repair)
		{
			CurrentGenerator->CurrentRepairRate = FMath::Clamp(CurrentGenerator->CurrentRepairRate + 1.0f, 0.0f, 100.0f);;
		}
	}
	// Good skill check
	if (Type == int8(1))
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, FString::Printf(TEXT("Good skill check")));
	}
	// Failed skill check
	if (Type == int8(2))
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, FString::Printf(TEXT("Failed skill check")));

		if (CurrentGenerator != nullptr && CurrentInteractionState == ESurvivorInteraction::Repair)
		{
			CurrentGenerator->CurrentRepairRate = FMath::Clamp(CurrentGenerator->CurrentRepairRate - 10.0f, 0.0f, 100.0f);;
		}
	}
}

void ADBDSurvivor::TryTriggerSkillCheck()
{
	ADBDPlayerController* PC = Cast<ADBDPlayerController>(GetController());
	if (PC)
	{
		if (PC->bIsSkillChecking)
		{
			return;
		}
	}

	float Chance = FMath::FRand(); // 0.0 ~ 1.0
	if (Chance < 0.08)
	{
		StartSkillCheck();
	}
}

void ADBDSurvivor::StartVault()
{
	if (!CurrentWindow || bIsVaulting || CurrentHealthStateEnum == EHealthState::DeepWound)
	{
		return;
	}

	bIsVaulting = true;

	VaultStartLocation = CurrentWindow->StartLocation[0];
	VaultEndLocation = CurrentWindow->StartLocation[1];
	VaultTopLocation = CurrentWindow->TopLocation;
	
	// Find front location of the window
	double MinDistance = FVector::Distance(GetActorLocation(), VaultStartLocation);
	if (MinDistance > FVector::Distance(GetActorLocation(), CurrentWindow->StartLocation[1]))
	{
		VaultStartLocation = VaultEndLocation;
		VaultEndLocation = CurrentWindow->StartLocation[0];
	}

	VaultStartLocation.Z = GetActorLocation().Z;
	VaultEndLocation.Z = GetActorLocation().Z;

	// Move to front of window
	SetActorLocation(VaultStartLocation);
	FRotator VaultRotation = (CurrentWindow->GetActorLocation() - GetActorLocation()).Rotation();
	VaultRotation.Pitch = 0.0f;
	VaultRotation.Roll = 0.0f;
	SetActorRotation(VaultRotation);

	// Set vault speed
	float PlayRate = bIsSprinting && !bIsCrouched  ? 3.16f : 1.0f;
	float PlayTime = bIsSprinting && !bIsCrouched ? 0.5f : 1.58f;

	// Change collision
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetCharacterMovement()->SetMovementMode(MOVE_Flying);

	// Play animation
	GetMesh()->GetAnimInstance()->Montage_Play(VaultAnim, PlayRate);

	// Stop vault
	GetWorld()->GetTimerManager().SetTimer
	(
		VaultTimer,
		this,
		&ADBDSurvivor::StopVault,
		PlayTime,
		false
	);
}

void ADBDSurvivor::StopVault()
{
	bIsVaulting = false;

	// Change collision
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetCharacterMovement()->SetMovementMode(MOVE_Walking);
}
