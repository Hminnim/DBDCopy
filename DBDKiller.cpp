// Fill out your copyright notice in the Description page of Project Settings.


#include "DBDKiller.h"
#include "DBDSurvivor.h"

// Sets default values
ADBDKiller::ADBDKiller()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	bReplicates = true;

	// Set default character movement
	bUseControllerRotationYaw = true;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->bUseControllerDesiredRotation = true;
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;

	// Camera config
	Camera = CreateDefaultSubobject<UCameraComponent>("Camera");
	Camera->SetupAttachment(GetMesh(), "headSocket");
	Camera->SetRelativeLocation(FVector(0.0f, 15.0f, 0.0f));
	Camera->bUsePawnControlRotation = true;

	// Mesh config
	GetMesh()->SetOwnerNoSee(false);

	// Redstain config
	RedStain = CreateDefaultSubobject<USpotLightComponent>("Red Stain");
	RedStain->SetupAttachment(GetMesh());
}

void ADBDKiller::BeginOverlapCharacterChange()
{
	if (PC)
	{
		PC->ShowActionMessage("Press Space to Change Character");
	}
	bCanCharacterChange = true;
}

void ADBDKiller::EndOverlapCharacterChange()
{
	if (PC)
	{
		PC->HideActionMessage();
	}
	bCanCharacterChange = false;
}

void ADBDKiller::BeginOverlapVault()
{
	bCanVault = true;
	if (PC)
	{
		PC->ShowActionMessage("Press Space to Vault");
	}
}

void ADBDKiller::EndOverlapVault()
{
	bCanVault = false;
	if (PC)
	{
		PC->HideActionMessage();
	}
}

void ADBDKiller::SetCurrentWindow(ADBDWindowActor* WindowActor)
{
	CurrentWindow = WindowActor;
}

// Called when the game starts or when spawned
void ADBDKiller::BeginPlay()
{
	Super::BeginPlay();
	
	if (IsLocallyControlled())
	{
		PC = Cast<ADBDPlayerController>(GetController());
		RedStain->SetVisibility(false);
	}
	else
	{
		RedStain->SetVisibility(true);
	}
}

// Called every frame
void ADBDKiller::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	FindBreakable();
}

void ADBDKiller::NotifyControllerChanged()
{
	Super::NotifyControllerChanged();

	if (IsLocallyControlled())
	{
		PC = Cast<ADBDPlayerController>(GetController());
	}

	// Add input mapping context
	if (PC)
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
	if (bIsBreakingPallet || bIsBreakingGenerator || bIsVaulting)
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

void ADBDKiller::Look(const FInputActionValue& Value)
{
	if (bIsBreakingPallet || bIsBreakingGenerator || bIsVaulting)
	{
		return;
	}

	const FVector2D LookValue = Value.Get<FVector2D>();

	AddControllerYawInput(LookValue.X);
	AddControllerPitchInput(-LookValue.Y);
}

void ADBDKiller::Interact(const FInputActionValue& Value)
{
	if (Value.Get<bool>() == false || bIsBreakingPallet || bIsBreakingGenerator || !bCanAttack || bIsAttacking || bIsVaulting)
	{
		return;
	}

	Attack();
}

void ADBDKiller::Action(const FInputActionValue& Value)
{
	if (bIsAttacking || bIsVaulting)
	{
		return;
	}

	if (bCanCharacterChange)
	{
		if (PC)
		{
			PC->HideActionMessage();
			PC->CharacterChange();
		}
	}
	if (bCanBreakPallet)
	{
		BreakPallet();
	}
	if (bCanBreakGenerator)
	{
		BreakGenerator();
	}
	if (bCanVault)
	{
		Vault();
	}
}

void ADBDKiller::FindBreakable()
{
	if (bIsBreakingPallet || bIsBreakingGenerator || bIsAttacking || bCanVault)
	{
		return;
	}

	FVector FireStart = Camera->GetComponentLocation() + Camera->GetForwardVector();
	FVector FireEnd = (Camera->GetForwardVector() * 200) + FireStart;

	FHitResult HitResult;

	if (GetWorld()->LineTraceSingleByChannel(HitResult, FireStart, FireEnd, ECollisionChannel::ECC_Visibility))
	{
		if (PC)
		{
			if (HitResult.GetActor())
			{
				ADBDPalletActor* HitPallet = Cast<ADBDPalletActor>(HitResult.GetActor());
				if (HitPallet)
				{
					if (HitPallet->bIsDropped)
					{
						CurrentPallet = HitPallet;
						PC->ShowActionMessage("Press Space to break the pallet");
						bCanBreakPallet = true;
						return;
					}
				}

				ADBDGeneratorActor* HitGenerator = Cast<ADBDGeneratorActor>(HitResult.GetActor());
				if (HitGenerator)
				{
					CurrentGenerator = HitGenerator;
					PC->ShowInteractionProgress(CurrentGenerator->CurrentRepairRate);
					if (CurrentGenerator->CurrentRepairRate > 0.0f)
					{
						PC->ShowActionMessage("Press Space to break the pallet");
						bCanBreakGenerator = true;
					}
					return;
				}
			}
		}
	}

	if (PC)
	{
		PC->HideActionMessage();
		PC->HideInteractionProgress();
		bCanBreakPallet = false;
		bCanBreakGenerator = false;
	}
}

void ADBDKiller::BreakPallet()
{
	if (!CurrentPallet || !bCanBreakPallet || bIsBreakingPallet)
	{
		return;
	}

	bIsBreakingPallet = true;
	
	if (PC)
	{
		PC->HideActionMessage();
	}

	// Move to front of current pallet
	FVector PalletFrontLocation;
	if (FVector::Distance(GetActorLocation(), CurrentPallet->StartLocation[0]) >
		FVector::Distance(GetActorLocation(), CurrentPallet->StartLocation[1]))
	{
		PalletFrontLocation = CurrentPallet->StartLocation[1];
	}
	else
	{
		PalletFrontLocation = CurrentPallet->StartLocation[0];
	}
	SetActorLocation(PalletFrontLocation);

	FVector PalletTriggerBoxLocation = CurrentPallet->TriggerBox->GetComponentTransform().TransformPosition(CurrentPallet->TriggerBox->GetRelativeLocation() - FVector({ 30.0f,0.0f,0.0f }));
	FRotator PalletRotation = (PalletTriggerBoxLocation - GetActorLocation()).Rotation();
	PalletRotation.Pitch = 0.0f;
	PalletRotation.Roll = 0.0f;
	SetActorRotation(PalletRotation);

	PlayAnimMontage(BreakAnim);

	GetWorld()->GetTimerManager().SetTimer
	(
		BreakTimerHandle,
		this,
		&ADBDKiller::EndBreakPallet,
		2.34f,
		false
	);
	
}

void ADBDKiller::EndBreakPallet()
{
	CurrentPallet->Destroy();
	bIsBreakingPallet = false;
	bCanBreakPallet = false;
}

void ADBDKiller::BreakGenerator()
{
	if (bIsBreakingGenerator)
	{
		return;
	}

	bIsBreakingGenerator = true;

	if (PC)
	{
		PC->HideActionMessage();
	}

	// Move to front of current generator
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

	PlayAnimMontage(BreakAnim);

	GetWorld()->GetTimerManager().SetTimer
	(
		BreakTimerHandle,
		this,
		&ADBDKiller::EndBreakGenerator,
		1.8f,
		false
	);
}

void ADBDKiller::EndBreakGenerator()
{
	bIsBreakingGenerator = false;
	bCanBreakGenerator = false;
	CurrentGenerator->CurrentRepairRate = CurrentGenerator->CurrentRepairRate > 2.5f ? CurrentGenerator->CurrentRepairRate - 2.5f : 0.0f;
	StopAnimMontage();
}

void ADBDKiller::Attack()
{
	bool bIsSuccces = false;

	FVector FireStart = Camera->GetComponentLocation() + Camera->GetForwardVector();
	FVector FireEnd = (Camera->GetForwardVector() * 250) + FireStart;

	FHitResult HitResult;

	if (GetWorld()->LineTraceSingleByChannel(HitResult, FireStart, FireEnd, ECollisionChannel::ECC_Visibility))
	{
		if (PC)
		{
			if (HitResult.GetActor())
			{
				if (ADBDSurvivor* HitSuvivor = Cast<ADBDSurvivor>(HitResult.GetActor()))
				{
					if (HitSuvivor->CurrentHealthStateEnum != EHealthState::DeepWound)
					{
						UGameplayStatics::ApplyDamage(HitResult.GetActor(), 1, PC, this, UDamageType::StaticClass());
						bIsSuccces = true;
					}
				}
				GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, FString::Printf(TEXT("HitResult : %s"), *HitResult.GetActor()->GetName()));
			}
		}
	}

	bCanAttack = false;
	GetCharacterMovement()->MaxWalkSpeed = 200.0f;
	bIsAttacking = true;
	PlayAnimMontage(AttackAnim);
	float AttackDelay = bIsSuccces ? 2.7f : 1.5f;

	GetWorld()->GetTimerManager().SetTimer
	(
		AttackTimerHandle,
		this,
		&ADBDKiller::EndAttack,
		AttackDelay,
		false
	);
}

void ADBDKiller::EndAttack()
{
	bCanAttack = true;
	bIsAttacking = false;
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
}

void ADBDKiller::Vault()
{
	if (bIsVaulting || !bCanVault)
	{
		return;
	}

	bIsVaulting = true;


	// Find front location of current window
	FVector VaultLocation = CurrentWindow->StartLocation[0];
	double VaultDistance = FVector::Distance(GetActorLocation(), VaultLocation);
	if (VaultDistance > FVector::Distance(GetActorLocation(), CurrentWindow->StartLocation[1]))
	{
		VaultLocation = CurrentWindow->StartLocation[1];
	}
	VaultLocation.Z = GetActorLocation().Z;

	// Move to front of current window
	SetActorLocation(VaultLocation);
	FRotator VaultRotation = (CurrentWindow->GetActorLocation() - GetActorLocation()).Rotation();
	VaultRotation.Pitch = 0.0f;
	VaultRotation.Roll = 0.0f;
	SetActorRotation(VaultRotation);

	// Change collision
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetCharacterMovement()->SetMovementMode(MOVE_Flying);

	PlayAnimMontage(VaultAnim);
	FTimerHandle AnimTimerHandle;
	GetWorld()->GetTimerManager().SetTimer
	(
		AnimTimerHandle,
		this,
		&ADBDKiller::EndVaultAnim,
		1.0f,
		false
	);


	GetWorld()->GetTimerManager().SetTimer
	(
		VaultTimerHandle,
		this,
		&ADBDKiller::EndVault,
		1.7f,
		false
	);
}

void ADBDKiller::EndVault()
{
	bIsVaulting = false;
	
}

void ADBDKiller::EndVaultAnim()
{
	StopAnimMontage(VaultAnim);
	// Change collision
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetCharacterMovement()->SetMovementMode(MOVE_Walking);
}
