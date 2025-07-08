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
	Camera->SetupAttachment(GetMesh(), "headSocket");
	Camera->SetRelativeLocation(FVector(0.0f, 15.0f, 0.0f));
	Camera->bUsePawnControlRotation = true;

	// Mesh config
	GetMesh()->SetOwnerNoSee(false);
}

void ADBDKiller::BeginOverlapCharacterChange()
{
	if (ADBDPlayerController* PC = Cast<ADBDPlayerController>(GetController()))
	{
		PC->ShowActionMessage("Press Space to Change Character");
	}
	bCanCharacterChange = true;
}

void ADBDKiller::EndOverlapCharacterChange()
{
	if (ADBDPlayerController* PC = Cast<ADBDPlayerController>(GetController()))
	{
		PC->HideActionMessage();
	}
	bCanCharacterChange = false;
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

	FindBreakable();
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
	if (bIsBreakingPallet || bIsBreakingGenerator)
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
	if (bIsBreakingPallet || bIsBreakingGenerator)
	{
		return;
	}

	const FVector2D LookValue = Value.Get<FVector2D>();

	AddControllerYawInput(LookValue.X);
	AddControllerPitchInput(-LookValue.Y);
}

void ADBDKiller::Interact(const FInputActionValue& Value)
{
	if (Value.Get<bool>() == false || bIsBreakingPallet || bIsBreakingGenerator)
	{
		return;
	}

	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, FString::Printf(TEXT("Interact")));

	FVector FireStart = Camera->GetComponentLocation() + Camera->GetForwardVector();
	FVector FireEnd = (Camera->GetForwardVector() * 250) + FireStart;

	FHitResult HitResult;

	if (GetWorld()->LineTraceSingleByChannel(HitResult, FireStart, FireEnd, ECollisionChannel::ECC_Visibility))
	{
		if (ADBDPlayerController* PC = Cast<ADBDPlayerController>(GetController()))
		{
			if (HitResult.GetActor())
			{
				UGameplayStatics::ApplyDamage(HitResult.GetActor(), 1, PC, this, UDamageType::StaticClass());

				GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, FString::Printf(TEXT("HitResult : %s"), *HitResult.GetActor()->GetName()));
			}
		}
	}
}

void ADBDKiller::Action(const FInputActionValue& Value)
{
	if (bCanCharacterChange)
	{
		if (ADBDPlayerController* PC = Cast<ADBDPlayerController>(GetController()))
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
}

void ADBDKiller::FindBreakable()
{
	if (bIsBreakingPallet || bIsBreakingGenerator)
	{
		return;
	}

	FVector FireStart = Camera->GetComponentLocation() + Camera->GetForwardVector();
	FVector FireEnd = (Camera->GetForwardVector() * 200) + FireStart;

	FHitResult HitResult;

	if (GetWorld()->LineTraceSingleByChannel(HitResult, FireStart, FireEnd, ECollisionChannel::ECC_Visibility))
	{
		if (ADBDPlayerController* PC = Cast<ADBDPlayerController>(GetController()))
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

	if (ADBDPlayerController* PC = Cast<ADBDPlayerController>(GetController()))
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
	
	if (ADBDPlayerController* PC = Cast<ADBDPlayerController>(GetController()))
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

	if (ADBDPlayerController* PC = Cast<ADBDPlayerController>(GetController()))
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
