// Fill out your copyright notice in the Description page of Project Settings.


#include "DBDSurvivor.h"
#include "DBDKiller.h"
#include "DBDGateLeverSwitchActor.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/AudioComponent.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimInstance.h"
#include "Net/UnrealNetwork.h"
#include "Engine/Engine.h"


// Sets default values
ADBDSurvivor::ADBDSurvivor()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	GetMesh()->bReceivesDecals = false;

	// Set default character movement
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->bUseControllerDesiredRotation = true;
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	GetCharacterMovement()->MaxWalkSpeedCrouched = CrouchSpeed;
	GetCharacterMovement()->GetNavAgentPropertiesRef().bCanCrouch = true;

	// SpringArm config
	SpringArm = CreateDefaultSubobject<USpringArmComponent>("SpringArm");
	SpringArm->SetupAttachment(GetMesh());
	SpringArm->bUsePawnControlRotation = true;
	SpringArm->SetRelativeLocation(FVector(0.0f, 0.0f, 50.0f));
	SpringArm->TargetArmLength = 200.0f;

	// Camera config
	Camera = CreateDefaultSubobject<UCameraComponent>("Camera");
	Camera->SetupAttachment(SpringArm);
	Camera->bUsePawnControlRotation = false;

	// Sound config
	HeartBeatSound = CreateDefaultSubobject<UAudioComponent>("HeartBeat");
	HeartBeatSound->SetupAttachment(GetMesh());
	HeartBeatSound->bAutoActivate = false;
}

void ADBDSurvivor::BeginOverlapWindowVault()
{
	if (CurrentHealthStateEnum == EHealthState::DeepWound)
	{
		return;
	}

	if (IsLocallyControlled())
	{
		PC->ShowActionMessage("Press Space to Vault");
	}

	VaultType = 0;
	bCanVault = true;
}

void ADBDSurvivor::EndOverlapWindowVault()
{
	if (IsLocallyControlled())
	{
		PC->HideActionMessage();
	}
	bCanVault = false;
}

void ADBDSurvivor::SetCurrentWindow(ADBDWindowActor* Target)
{
	CurrentWindow = Target;
}

void ADBDSurvivor::BeginOverlapPallet()
{
	if (IsLocallyControlled())
	{
		PC->ShowActionMessage("Press Space to Drop");
	}
	bCanDrop = true;
}

void ADBDSurvivor::EndOverlapPallet()
{
	if (IsLocallyControlled())
	{
		PC->HideActionMessage();
	}
	bCanDrop = false;
}

void ADBDSurvivor::BeginOverlapPalletVault()
{
	if (IsLocallyControlled())
	{
		PC->ShowActionMessage("Press Space to Vault");
	}

	VaultType = 1;
	bCanVault = true;
}

void ADBDSurvivor::EndOverlapPalletVault()
{
	if (IsLocallyControlled())
	{
		PC->HideActionMessage();
	}

	bCanVault = false;
}

void ADBDSurvivor::SetCurrentPallet(ADBDPalletActor* Target)
{
	CurrentPallet = Target;
}

void ADBDSurvivor::BeginOverlapCharacterChange()
{
	if (IsLocallyControlled())
	{
		PC->ShowActionMessage("Press Space to Change Character");
	}

	bCanCharacterChange = true;
}

void ADBDSurvivor::EndOverlapCharacterChange()
{
	if (IsLocallyControlled())
	{
		PC->HideActionMessage();
	}

	bCanCharacterChange = false;
}

void ADBDSurvivor::BeCarried()
{
	if (OwnedPlayerState)
	{
		OwnedPlayerState->SetHealthState(EHealthState::Carried);
	}

	SetActorEnableCollision(false);

	if (GetCharacterMovement())
	{
		GetCharacterMovement()->SetMovementMode(MOVE_None);
		GetCharacterMovement()->Deactivate();
	}

	GetWorld()->GetTimerManager().SetTimer
	(
		SkillCheckTimer,
		this,
		&ADBDSurvivor::StartWiggleSkillCheck,
		2.0f,
		false
	);

}

void ADBDSurvivor::StopBeCarried()
{
	if (OwnedPlayerState)
	{
		OwnedPlayerState->SetHealthState(EHealthState::Injured);
	}
	
	SetActorEnableCollision(true);

	if (GetCharacterMovement())
	{
		GetCharacterMovement()->Activate();
		GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	}

	StopWiggleSkillCheck();
}

void ADBDSurvivor::BeHooked()
{
	if (OwnedPlayerState)
	{
		OwnedPlayerState->SetHealthState(EHealthState::Hooked);
	}

	SetActorEnableCollision(true);

	if (GetCharacterMovement())
	{
		GetCharacterMovement()->SetMovementMode(MOVE_None);
		GetCharacterMovement()->Deactivate();
	}

	// Handdle hook stage
	if (OwnedPlayerState)
	{
		if (OwnedPlayerState->CurrentHookStageType == int8(0))
		{
			OwnedPlayerState->CurrentHookStageType = int8(1);
			OwnedPlayerState->CurrentHookStageRate = 100.0f;
		}
		else if (OwnedPlayerState->CurrentHookStageType == int8(1))
		{
			OwnedPlayerState->CurrentHookStageType = int8(2);
			OwnedPlayerState->CurrentHookStageRate = 50.0f;
			StartStruggleStage();
		}
		else if (OwnedPlayerState->CurrentHookStageType == int8(2))
		{
			if (HasAuthority())
			{
				Server_SurvivorDeath();
			}
		}
	}
}

void ADBDSurvivor::StopBeHooked()
{
	if (OwnedPlayerState)
	{
		OwnedPlayerState->SetHealthState(EHealthState::Injured);
	}

	SetActorEnableCollision(true);

	if (GetCharacterMovement())
	{
		GetCharacterMovement()->Activate();
		GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	}

	if (PC)
	{
		PC->HideInteractionProgress();
	}
}

void ADBDSurvivor::StartBeingUnhooked()
{
	if (BeingUnhookedAnim)
	{
		PlayAnimMontage(BeingUnhookedAnim);
	}
}

void ADBDSurvivor::StopBeingUnhooked()
{
	if (BeingUnhookedAnim)
	{
		StopAnimMontage(BeingUnhookedAnim);
	}
}

void ADBDSurvivor::OnToShowScratchMark()
{
	bCanShowScratch = true;
}

void ADBDSurvivor::OnTakeDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser)
{
	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, FString::Printf(TEXT("OnTakeDamge")));
	Server_TakeDamage();
}

// Called when the game starts or when spawned
void ADBDSurvivor::BeginPlay()
{
	Super::BeginPlay();

	if (IsLocallyControlled())
	{
		PC = Cast<ADBDPlayerController>(GetController());
	}

	// function bind
	OnTakeAnyDamage.AddDynamic(this, &ADBDSurvivor::OnTakeDamage);
	
	UAnimInstance* AnimInstace = GetMesh()->GetAnimInstance();
	if (AnimInstace)
	{
		AnimInstace->OnPlayMontageNotifyBegin.AddDynamic(this, &ADBDSurvivor::AnimNotifyBeginHandler);
	}

	// Set timer to find killer actor
	GetWorld()->GetTimerManager().SetTimer(
		FindKillerTimer,
		this,
		&ADBDSurvivor::FindKillerActor,
		2.0f,
		false
	);
}

void ADBDSurvivor::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	InitPlayerState();
}

void ADBDSurvivor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ADBDSurvivor, bIsSprinting);
	DOREPLIFETIME(ADBDSurvivor, bIsVaulting);
	DOREPLIFETIME(ADBDSurvivor, bIsInteracting);
	DOREPLIFETIME(ADBDSurvivor, bIsHealed);
	DOREPLIFETIME(ADBDSurvivor, bIsWigglePause);
	DOREPLIFETIME(ADBDSurvivor, CurrentHealedRate);
	DOREPLIFETIME(ADBDSurvivor, CurrentInteractionState);
	DOREPLIFETIME(ADBDSurvivor, CurrentWiggleRate);
}

void ADBDSurvivor::FindKillerActor()
{
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ADBDKiller::StaticClass(), FoundActors);
	if (FoundActors.Num() > 0)
	{
		KillerActorInGame = Cast<ADBDKiller>(FoundActors[0]);
	}
}

// Called every frame
void ADBDSurvivor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);	

	FindInteratable();

	// Handling repairing
	if (CurrentInteractionState == ESurvivorInteraction::Repair && bIsInteracting)
	{
		if (CurrentGenerator)
		{
			if (IsLocallyControlled() && PC)
			{
				PC->ShowInteractionProgress(CurrentGenerator->CurrentRepairRate);
			}
			HandleRepaiGenerator(DeltaTime);

			if (CurrentGenerator->bIsRepaired)
			{
				StopRepairGenerator();
			}
		}
	}
	Server_HandleRepairGenerator(DeltaTime);
	// Handling healing
	if (CurrentInteractionState == ESurvivorInteraction::Heal && bIsHealing)
	{
		if (IsLocallyControlled())
		{
			PC->ShowInteractionProgress(CurrentTargetSurvivor->CurrentHealedRate);
		}
		if (CurrentTargetSurvivor)
		{
			if (CurrentTargetSurvivor->CurrentHealthStateEnum == EHealthState::Healthy)
			{ 
				StopHealSurvivor();
			}
		}
	}
	Server_HandleHealSurvivor(DeltaTime);
	HandleHealed();

	// Handling bleeding
	HandleBleeding(DeltaTime);
	
	HandleHealthState();
	HandleHookStageRate(DeltaTime);
	Server_HandleHookStageRate(DeltaTime);

	// Wiggle
	HandleWiggleSkillCheckMiss();
	HandleWiggleRate(DeltaTime);
	/*Server_HandleWiggleRate(DeltaTime);*/

	// Self Unhooking progress
	SelfUnhooking(DeltaTime);

	HandleSpawnScratchMark(DeltaTime);

	HandleOpeningGate();
	Server_HandleOpeningGate(DeltaTime);

	/*PlayTrerrorRadiusSound();*/
}

void ADBDSurvivor::NotifyControllerChanged()
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
	if (bIsInteracting || bIsActing || bIsDropping || bIsHealed || bIsVaulting || bIsUnHooking)
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
	if (bIsInteracting || bIsActing || CurrentHealthStateEnum == EHealthState::DeepWound || bIsDropping || bIsHealed || bIsUnHooking)
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
	if (bIsInteracting || bIsActing || CurrentHealthStateEnum == EHealthState::DeepWound || bIsDropping || bIsHealed)
	{
		return;
	}

	if (Value.Get<bool>())
	{
		StartSprinting();
	}
	else
	{
		StopSprinting();
	}
}

void ADBDSurvivor::Interact(const FInputActionValue& Value)
{
	if (CurrentHealthStateEnum == EHealthState::DeepWound || bIsDropping || bIsHealed || bIsUnHooking)
	{
		return;
	}

	if (CurrentInteractionState == ESurvivorInteraction::Repair)
	{
		if (Value.Get<bool>())
		{
			StartRepairGenerator();
			Server_StartRepairGenerator();
			return;
		}
		else
		{
			StopRepairGenerator();
			Server_StopRepairGenerator();
			return;
		}
	}

	if (CurrentInteractionState == ESurvivorInteraction::Heal)
	{
		if (Value.Get<bool>())
		{
			StartHealSurvivor();
			Server_StartHealSurvivor();
			return;
		}
		else
		{
			StopHealSurvivor();
			Server_StopHealSurvivor();
			return;
		}
	}

	if (CurrentInteractionState == ESurvivorInteraction::UnHook)
	{
		if (Value.Get<bool>())
		{
			StartUnhook();
			Server_StartUnhook();
			return;
		}
		else
		{
			StopUnhook();
			Server_StopUnhook();
			return;
		}
	}

	if (CurrentHealthStateEnum == EHealthState::Hooked && CurrentHookStageType == int(1))
	{
		if (Value.Get<bool>())
		{
			StartSelfUnhook();
			return;
		}
		else
		{
			StopSelfUnhook();
			return;
		}
	}

	if(CurrentInteractionState == ESurvivorInteraction::Lever)
	{
		if (Value.Get<bool>())
		{
			StartOpeningGate();
			Server_StartOpeningGate();
			return;
		}
		else
		{
			StopOpeningGate();
			Server_StopOpeningGate();
			return;
		}
	}
}

void ADBDSurvivor::Action(const FInputActionValue& Value)
{
	if (bIsDropping || bIsVaulting || bIsHealed || bIsUnHooking)
	{
		return;
	}

	// Generator Skill Check and Heal Skill Check
	if (CurrentInteractionState == ESurvivorInteraction::Repair || CurrentInteractionState == ESurvivorInteraction::Heal)
	{
		if (IsLocallyControlled())
		{
			if (PC->bIsSkillChecking)
			{
				HandleGeneratorSkillCheck(PC->GetGeneratorSkillCheckResult());
			}
		}

		return;
	}

	// Wiggle Skill Check
	if (bIsWiggling && Value.Get<bool>())
	{
		if (IsLocallyControlled())
		{
			HandleWiggleSkillCheck(PC->GetWWiggleSkillCheckResult());
		}
	}

	// Vault
	if (bCanVault)
	{
		StartVault();
		Server_StartVault();
		return;
	}

	// Drop
	if (bCanDrop)
	{
		if (CurrentPallet)
		{
			StartDrop();
			Server_StartDrop();
		}
	}

	// Struggle
	if (CurrentHealthStateEnum == EHealthState::Hooked && CurrentHookStageType == int8(2) && bIsStruggling)
	{
		if (IsLocallyControlled())
		{
			Server_HandleStruggleSkillCheck(PC->GetStruggleSkillCheckResult());
		}
	}


	// For debug
	if (bCanCharacterChange)
	{
		if (IsLocallyControlled())
		{
			PC->HideActionMessage();
			UDBDGameInstance* GI = Cast<UDBDGameInstance>(GetGameInstance());
			if (GI)
			{
				PC->CharacterChange(GI->bIsKiller);
			}
		}
	}
}

void ADBDSurvivor::Server_Teleport_Implementation(FVector NewLocation)
{
	SetActorLocation(NewLocation);
}

void ADBDSurvivor::StartSprinting()
{
	if (IsLocallyControlled())
	{
		Server_SetSprinting(true);

		bIsSprinting = true;
		UpdateMovementSpeed();
	}
}

void ADBDSurvivor::StopSprinting()
{
	if (IsLocallyControlled())
	{
		Server_SetSprinting(false);

		bIsSprinting = false;
		UpdateMovementSpeed();
	}
}

void ADBDSurvivor::UpdateMovementSpeed()
{
	if (bIsSprinting && !bIsCrouched)
	{
		GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;
	}
	else
	{
		GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	}
}

void ADBDSurvivor::OnRep_IsSprinting()
{
	UpdateMovementSpeed();
}

void ADBDSurvivor::Server_SetSprinting_Implementation(bool bNewSprinting)
{
	bIsSprinting = bNewSprinting;
	UpdateMovementSpeed();
}

void ADBDSurvivor::StartVault()
{
	if (!bCanVault || bIsVaulting || CurrentHealthStateEnum == EHealthState::DeepWound)
	{
		return;
	}

	bIsVaulting = true;

	// Window Vault
	if (VaultType == 0 && CurrentWindow)
	{
		FVector VaultStartLocation = CurrentWindow->StartLocation[0];

		// Find front location of current window
		double MinDistance = FVector::Distance(GetActorLocation(), VaultStartLocation);
		if (MinDistance > FVector::Distance(GetActorLocation(), CurrentWindow->StartLocation[1]))
		{
			VaultStartLocation = CurrentWindow->StartLocation[1];
		}

		VaultStartLocation.Z = GetActorLocation().Z;

		// Move to front of current window
		SetActorLocation(VaultStartLocation);
		FRotator VaultRotation = (CurrentWindow->GetActorLocation() - GetActorLocation()).Rotation();
		VaultRotation.Pitch = 0.0f;
		VaultRotation.Roll = 0.0f;
		SetActorRotation(VaultRotation);
	}
	// Pallet Vault
	bool bIsLeft = false;
	if (VaultType == 1 && CurrentPallet)
	{
		FVector VaultStartLocation = CurrentPallet->StartLocation[0];

		// Find front location of current pallet
		double MinDistance = FVector::Distance(GetActorLocation(), VaultStartLocation);
		if (MinDistance > FVector::Distance(GetActorLocation(), CurrentPallet->StartLocation[1]))
		{
			VaultStartLocation = CurrentPallet->StartLocation[1];
			bIsLeft = true;
		}

		VaultStartLocation.Z = GetActorLocation().Z;

		// Move to front of current pallet
		SetActorLocation(VaultStartLocation);
		FVector PalletTriggerBoxLocation = CurrentPallet->TriggerBox->GetComponentTransform().TransformPosition(CurrentPallet->TriggerBox->GetRelativeLocation() - FVector({ 0.0f,80.0f,0.0f }));
		FRotator VaultRotation = (PalletTriggerBoxLocation - GetActorLocation()).Rotation();
		VaultRotation.Pitch = 0.0f;
		VaultRotation.Roll = 0.0f;
		SetActorRotation(VaultRotation);
	}

	// Change collision
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetCharacterMovement()->SetMovementMode(MOVE_Flying);

	// Play animation
	if (VaultType == 0 && CurrentWindow)
	{
		if (bIsSprinting)
		{
			if (VaultFastAnim)
			{
				PlayAnimMontage(VaultFastAnim);
			}
		}
		else
		{
			if (VaultSlowAnim)
			{
				PlayAnimMontage(VaultSlowAnim);
			}
		}
	}
	if (VaultType == 1 && CurrentPallet)
	{
		if (bIsSprinting)
		{
			if (bIsLeft)
			{
				if (LeftPalletVaultFastAnim)
				{
					PlayAnimMontage(LeftPalletVaultFastAnim);
				}
			}
			else
			{
				if (RightPalletVaultFastAnim)
				{
					PlayAnimMontage(RightPalletVaultFastAnim);
				}
			}
		}
		else 
		{
			if (bIsLeft)
			{
				if (LeftPalletVaultAnim)
				{
					PlayAnimMontage(LeftPalletVaultAnim);
				}
			}
			else
			{
				if (RightPalletVaultAnim)
				{
					PlayAnimMontage(RightPalletVaultAnim);
				}
			}
		}
	}
}

void ADBDSurvivor::StopVault()
{
	// Change collision
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetCharacterMovement()->SetMovementMode(MOVE_Walking);

	bIsVaulting = false;
}

void ADBDSurvivor::AnimNotifyBeginHandler(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointPayload)
{
	if (NotifyName == "EndVault")
	{
		StopVault();
	}
	if (NotifyName == "EndUnhook")
	{
		StopUnhook();
	}
	if (NotifyName == "EndBeUnhooked")
	{
		StopBeHooked();
	}
	if (NotifyName == "EndDrop")
	{
		StopDrop();
	}
}

void ADBDSurvivor::MultiCast_StartVault_Implementation()
{
	if (!IsLocallyControlled())
	{
		StartVault();
	}
}

void ADBDSurvivor::MultiCast_StopVault_Implementation()
{
	if (!IsLocallyControlled())
	{
		StopVault();
	}
}

void ADBDSurvivor::Server_StartVault_Implementation()
{
	StartVault();
	MultiCast_StartVault();
}

void ADBDSurvivor::Server_StopVault_Implementation()
{
	StopVault();
	MultiCast_StopVault();
}

void ADBDSurvivor::FindInteratable()
{
	if (bIsInteracting || bIsActing || CurrentHealthStateEnum == EHealthState::DeepWound || bIsHealed || bIsUnHooking)
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
				if (HitGenerator->bIsRepaired)
				{
					return;
				}

				CurrentInteractionState = ESurvivorInteraction::Repair;
				CurrentGenerator = HitGenerator;
				Server_SetCurrentGenerator(HitGenerator);
				if (IsLocallyControlled())
				{
					if (HitGenerator->CurrentRepairRate < 100.0f && HitGenerator->CurrentRepairingSurvivor < 3)
					{
						PC->ShowIneractionMessage("Press M1 to repair");
						PC->ShowInteractionProgress(CurrentGenerator->CurrentRepairRate);
					}
				}
				
				return;

			}

			if (ADBDGateLeverSwitchActor* HitLever = Cast<ADBDGateLeverSwitchActor>(HitResult.GetActor()))
			{
				if (HitLever->bIsOpened)
				{
					return;
				}

				CurrentInteractionState = ESurvivorInteraction::Lever;
				CurrentTargetLever = HitLever;
				Server_SetCurrentLever(HitLever);

				if (IsLocallyControlled())
				{
					if (CurrentTargetLever->CurrentLeverRate < 100.0f)
					{
						PC->ShowIneractionMessage("Press M1 to open");
						PC->ShowInteractionProgress(CurrentTargetLever->CurrentLeverRate);
					}
				}

				return;

			}

			if (ADBDSurvivor* HitSurvivor = Cast<ADBDSurvivor>(HitResult.GetActor()))
			{
				if (HitSurvivor->CurrentHealthStateEnum == EHealthState::Healthy)
				{
					return;
				}

				if (HitSurvivor->CurrentHealthStateEnum == EHealthState::Injured || HitSurvivor->CurrentHealthStateEnum == EHealthState::DeepWound)
				{
					CurrentInteractionState = ESurvivorInteraction::Heal;
					CurrentTargetSurvivor = HitSurvivor;
					Server_SetTargetSurvivor(HitSurvivor);
					if (IsLocallyControlled())
					{
						PC->ShowIneractionMessage("Press M1 to Heal Survivor");
						PC->ShowInteractionProgress(CurrentTargetSurvivor->CurrentHealedRate);
					}
					return;
				}
				
				if (HitSurvivor->CurrentHealthStateEnum == EHealthState::Hooked)
				{
					CurrentInteractionState = ESurvivorInteraction::UnHook;
					CurrentTargetSurvivor = HitSurvivor;

					if (IsLocallyControlled())
					{
						PC->ShowIneractionMessage("Press M1 to Unhook Survivor");
					}
					return;
				}
				
			}
		}
	}

	// Couldn't find interactable
	CurrentInteractionState = ESurvivorInteraction::Idle;
	if (IsLocallyControlled() && PC)
	{
		PC->HideInteractionMessage();
		PC->HideInteractionProgress();
	}

}

void ADBDSurvivor::FindActable()
{
	if (bIsInteracting || bIsActing || CurrentHealthStateEnum == EHealthState::DeepWound || bIsHealed || bIsUnHooking)
	{
		return;
	}
}

void ADBDSurvivor::StartRepairGenerator()
{
	if (!CurrentGenerator || CurrentInteractionState != ESurvivorInteraction::Repair || CurrentHealthStateEnum == EHealthState::DeepWound)
	{
		return;
	}
	if (IsLocallyControlled())
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
	if (IsLocallyControlled())
	{
		GetWorld()->GetTimerManager().SetTimer
		(
			SkillCheckTriggerTimer,
			this,
			&ADBDSurvivor::TryTriggerGeneratorSkillCheck,
			1.0f,
			true,
			1.0f
		);
	}
}

void ADBDSurvivor::StopRepairGenerator()
{
	if (!CurrentGenerator)
	{
		return;
	}

	bIsInteracting = false;
	CurrentGenerator->CurrentRepairingSurvivor = FMath::Clamp(--CurrentGenerator->CurrentRepairingSurvivor, 0, 4);
	if (IsLocallyControlled())
	{
		if (PC->bIsSkillChecking)
		{
			PC->StopGeneratorSkillCheck();
			HandleGeneratorSkillCheck(int8(2));
		}
	}
	GetWorld()->GetTimerManager().ClearTimer(SkillCheckTriggerTimer);
}

void ADBDSurvivor::HandleRepaiGenerator(float DeltaTime)
{
	if (!bIsInteracting || !CurrentGenerator || CurrentInteractionState != ESurvivorInteraction::Repair || CurrentHealthStateEnum == EHealthState::DeepWound)
	{
		return;
	}

	/*if (CurrentGenerator->CurrentRepairRate >= 100.0f)
	{
		CurrentGenerator->bIsRepaired = true;
		StopRepairGenerator();
		Server_StopRepairGenerator();
	}*/
}

void ADBDSurvivor::Server_UpdateGeneratorRepairRate_Implementation(float Amount)
{
	CurrentGenerator->CurrentRepairRate = FMath::Clamp(CurrentGenerator->CurrentRepairRate + Amount, 0.0f, 100.0f);

	if (CurrentGenerator->CurrentRepairRate >= 100.0f)
	{
		ADBDGameModeBase* MainGM = GetWorld()->GetAuthGameMode<ADBDGameModeBase>();
		if(MainGM)
		{
			MainGM->OnGeneratorCompleted();
		}
		CurrentGenerator->bIsRepaired = true;
		StopRepairGenerator();
		Server_StopRepairGenerator();
	}
}

void ADBDSurvivor::Server_StartRepairGenerator_Implementation()
{
	StartRepairGenerator();
	MultiCast_StartRepairGenerator();
	bIsReparingGenerator = true;
}

void ADBDSurvivor::Server_StopRepairGenerator_Implementation()
{
	StopRepairGenerator();
	MultiCast_StopRepairGenerator();
	bIsReparingGenerator = false;
}

void ADBDSurvivor::MultiCast_StartRepairGenerator_Implementation()
{
	if (!IsLocallyControlled())
	{
		StartRepairGenerator();
	}
}

void ADBDSurvivor::MultiCast_StopRepairGenerator_Implementation()
{
	if (!IsLocallyControlled())
	{
		StopRepairGenerator();
	}
}

void ADBDSurvivor::Server_SetCurrentGenerator_Implementation(ADBDGeneratorActor* TargetGenerator)
{
	CurrentGenerator = TargetGenerator;
}

void ADBDSurvivor::Server_HandleRepairGenerator(float DeltaTIme)
{
	if (!HasAuthority() || !bIsReparingGenerator || !CurrentGenerator)
	{
		return;
	}

	CurrentGenerator->CurrentRepairRate += RepairSpeed[CurrentGenerator->CurrentRepairingSurvivor] * DeltaTIme;

	if (CurrentGenerator->CurrentRepairRate >= 100.0f)
	{
		ADBDGameModeBase* MainGM = GetWorld()->GetAuthGameMode<ADBDGameModeBase>();
		if (MainGM)
		{
			MainGM->OnGeneratorCompleted();
		}
		CurrentGenerator->bIsRepaired = true;
		StopRepairGenerator();
		Server_StopRepairGenerator();
	}
}

void ADBDSurvivor::StartGeneratorSkillCheck()
{
	if (!IsLocallyControlled())
	{
		return;
	}

	PC->StartGeneratorSkillCheck();

	GetWorld()->GetTimerManager().SetTimer
	(
		SkillCheckTimer,
		this,
		&ADBDSurvivor::FailedGeneratorSkillCheck,
		1.6f,
		false
	);
}

void ADBDSurvivor::FailedGeneratorSkillCheck()
{
	if (!IsLocallyControlled())
	{
		return;
	}

	PC->StopGeneratorSkillCheck();
	HandleGeneratorSkillCheck((int8)2);
}

void ADBDSurvivor::HandleGeneratorSkillCheck(int8 Type)
{
	GetWorld()->GetTimerManager().ClearTimer(SkillCheckTimer);

	// Great skill check
	if (Type == int8(0))
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, FString::Printf(TEXT("Great skill check")));

		if (CurrentGenerator != nullptr && CurrentInteractionState == ESurvivorInteraction::Repair)
		{
			Server_UpdateGeneratorRepairRate(1.0f);
		}

		if (CurrentGenerator != nullptr && CurrentInteractionState == ESurvivorInteraction::Heal)
		{
			Server_UpdateTargetSurvivorHealRate(3.0f);
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
			Server_UpdateGeneratorRepairRate(-10.0f);
		}
		if (CurrentGenerator != nullptr && CurrentInteractionState == ESurvivorInteraction::Heal)
		{
			Server_UpdateTargetSurvivorHealRate(-10.0f);
		}
	}
}

void ADBDSurvivor::TryTriggerGeneratorSkillCheck()
{
	if (!IsLocallyControlled())
	{
		return;
	}

	if (PC->bIsSkillChecking)
	{
		return;
	}

	float Chance = FMath::FRand(); // 0.0 ~ 1.0
	if (Chance < 0.08)
	{
		StartGeneratorSkillCheck();
	}
}

void ADBDSurvivor::StartWiggleSkillCheck()
{
	if (bIsWiggling)
	{
		return;
	}

	bIsWiggling = true;

	if (PC)
	{
		PC->StartWiggleSkillCheck();
	}
}

void ADBDSurvivor::StopWiggleSkillCheck()
{
	bIsWiggling = false;
	bIsWigglePause = false;
	CurrentWiggleRate = 0.0f;

	if (PC)
	{
		PC->StopWiggleSkillCheck();
	}
}

void ADBDSurvivor::HandleWiggleSkillCheck(int8 Type)
{
	// Great skill check
	if (Type == int8(0))
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, FString::Printf(TEXT("Great skill check")));
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
	}

	Server_ChangeWiggleSKillCheckResult(Type);
}

void ADBDSurvivor::HandleWiggleSkillCheckMiss()
{
	if (!bIsWiggling)
	{
		return;
	}

	if (PC && IsLocallyControlled())
	{
		if (PC->GetWiggleSkillCheckMiss())
		{
			HandleWiggleSkillCheck(int8(2));
		}
	}
}

void ADBDSurvivor::StartStruggleSkillCheck()
{
	StruggleSkillCheckCount++;
	bIsStruggling = true;

	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, FString::Printf(TEXT("StartStruggleSkillCheck")));

	if (PC)
	{
		PC->StartStruggleSkillCheck(StruggleSkillCheckCount);
	}

	GetWorld()->GetTimerManager().SetTimer(
		SkillCheckTimer,
		this,
		&ADBDSurvivor::FailedStruggleSkillCheck,
		1.6f,
		false
	);
}

void ADBDSurvivor::StopStruggleSkillCheck()
{
	bIsStruggling = false;

	if (PC)
	{
		PC->StopStruggleSkillCheck();
	}

	GetWorld()->GetTimerManager().ClearTimer(SkillCheckTimer);
}

void ADBDSurvivor::FailedStruggleSkillCheck()
{
	Server_HandleStruggleSkillCheck(int8(2));
	StopStruggleSkillCheck();
}

void ADBDSurvivor::Server_HandleStruggleSkillCheck_Implementation(int8 Type)
{
	if (OwnedPlayerState)
	{
		if (Type == int8(0))
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, FString::Printf(TEXT("Success Struggle")));
		}
		if (Type == int8(2))
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, FString::Printf(TEXT("Fail Struggle")));
			OwnedPlayerState->CurrentHookStageRate -= 140.0f / 100.0f * 20.0f; // Decrease 20sec
		}
	}

	StopStruggleSkillCheck();
}

void ADBDSurvivor::Server_ChangeWiggleSKillCheckResult_Implementation(int8 Type)
{
	// Great skill check
	if (Type == int8(0))
	{
		bIsWigglePause = false;
	}
	// Good skill check
	if (Type == int8(1))
	{
		bIsWigglePause = false;
	}
	// Failed skill check
	if (Type == int8(2))
	{
		bIsWigglePause = true;
	}
}

void ADBDSurvivor::HandleWiggleRate(float DeltaTime)
{
	if (!bIsWiggling)
	{
		return;
	}
	else
	{
		if (IsLocallyControlled())
		{
			PC->ShowInteractionProgress(CurrentWiggleRate);
		}
	}

	if (bIsWigglePause)
	{
		return;
	}

	float RateSpeed = 100.0f / 16.0f;
	CurrentWiggleRate += RateSpeed * DeltaTime;

	if (CurrentWiggleRate >= 100.0f)
	{
		bIsWigglePause = true;

		if (IsLocallyControlled())
		{
			PC->HideInteractionProgress();
		}
	}
}

void ADBDSurvivor::Server_HandleWiggleRate_Implementation(float DeltaTime)
{
	if (!bIsWiggling || bIsWigglePause)
	{
		return;
	}

	float RateSpeed = 100.0f / 16.0f;
	CurrentWiggleRate += RateSpeed * DeltaTime;

	if (CurrentWiggleRate >= 100.0f)
	{
		bIsWigglePause = true;
	}
}

void ADBDSurvivor::StartHealSurvivor()
{
	if (CurrentTargetSurvivor == nullptr)
	{
		return;
	}

	if (IsLocallyControlled())
	{
		PC->HideActionMessage();
		PC->HideInteractionMessage();
	}

	bIsInteracting = true;
	bIsHealing = true;
	CurrentTargetSurvivor->bIsHealed = true;

	// Set rotation to target survivor
	FRotator TargetSurvivorRotation = (CurrentTargetSurvivor->GetActorLocation() - GetActorLocation()).Rotation();
	TargetSurvivorRotation.Pitch = 0.0f;
	TargetSurvivorRotation.Roll = 0.0f;
	SetActorRotation(TargetSurvivorRotation);

	// Try trigger skillcheck
	if (!IsLocallyControlled())
	{
		GetWorld()->GetTimerManager().SetTimer
		(
			SkillCheckTriggerTimer,
			this,
			&ADBDSurvivor::TryTriggerGeneratorSkillCheck,
			1.0f,
			true,
			1.0f
		);
	}
}

void ADBDSurvivor::StopHealSurvivor()
{
	if (CurrentTargetSurvivor == nullptr)
	{
		return;
	}

	bIsInteracting = false;
	bIsHealing = false;
	CurrentTargetSurvivor->bIsHealed = false;

	if (IsLocallyControlled())
	{
		if (PC->bIsSkillChecking)
		{
			PC->StopGeneratorSkillCheck();
			HandleGeneratorSkillCheck(int8(2));
		}
	}
	GetWorld()->GetTimerManager().ClearTimer(SkillCheckTriggerTimer);
}

void ADBDSurvivor::HandleHealSurvivor(float DeltaTime)
{
	if (bIsHealing == false) 
	{
		return;
	}

	CurrentTargetSurvivor->CurrentHealedRate += 6.25f * DeltaTime;
}

void ADBDSurvivor::HandleHealed()
{
	if (bIsHealed == false || !HasAuthority())
	{
		return;
	}

	if (CurrentHealedRate >= 100.0f)
	{
		if (CurrentHealthStateEnum == EHealthState::Injured)
		{
			if (OwnedPlayerState)
			{
				OwnedPlayerState->SetHealthState(EHealthState::Healthy);
			}
			CurrentHealedRate = 0.0f;
			UpdateMovementSpeed();
		}
		if (CurrentHealthStateEnum == EHealthState::DeepWound)
		{
			if (OwnedPlayerState)
			{
				OwnedPlayerState->SetHealthState(EHealthState::Injured);
			}
			CurrentHealedRate = 0.0f;
			UpdateMovementSpeed();
		}
	}
}

void ADBDSurvivor::Server_SetTargetSurvivor_Implementation(ADBDSurvivor* TargetSurvivor)
{
	CurrentTargetSurvivor = TargetSurvivor;
}

void ADBDSurvivor::Server_StartHealSurvivor_Implementation()
{
	StartHealSurvivor();
}

void ADBDSurvivor::Server_StopHealSurvivor_Implementation()
{
	StopHealSurvivor();
}

void ADBDSurvivor::Server_UpdateTargetSurvivorHealRate_Implementation(float Amount)
{
	if (CurrentTargetSurvivor)
	{
		CurrentTargetSurvivor->CurrentHealedRate = FMath::Clamp(CurrentTargetSurvivor->CurrentHealedRate + Amount, 0.0f, 100.0f);
	}
}

void ADBDSurvivor::Server_HandleHealSurvivor(float DeltaTime)
{
	if (bIsHealing == false || !HasAuthority())
	{
		return;
	}

	CurrentTargetSurvivor->CurrentHealedRate += 6.25f * DeltaTime;
}


void ADBDSurvivor::MultiCast_StartHealSurvivor_Implementation()
{
	if (!IsLocallyControlled())
	{
		StartHealSurvivor();
	}
}

void ADBDSurvivor::MultiCast_StopHealSurvivor_Implementation()
{
	if (!IsLocallyControlled())
	{
		StopHealSurvivor();
	}
}

void ADBDSurvivor::HandleBleeding(float DeltaTime)
{
	if (CurrentHealthStateEnum != EHealthState::Healthy)
	{
		BleedingTimer += DeltaTime;

		if (BleedingTimer >= 1.2f)
		{
			UCharacterMovementComponent* CMC = GetCharacterMovement();
			if (CMC)
			{
				FFindFloorResult& FloorResult = CMC->CurrentFloor;

				if (FloorResult.bBlockingHit && FloorResult.IsWalkableFloor())
				{
					FVector HitLocation = FloorResult.HitResult.Location;
					FVector HitNormal = FloorResult.HitResult.ImpactNormal;

					FRotator SpawnDecalRotation = HitNormal.Rotation();
					/*SpawnDecalRotation.Pitch -= 90.0f;*/
					SpawnDecalRotation.Yaw += FMath::RandRange(0.0f, 360.0f);

					if (BloodDecalMaterial)
					{
						UGameplayStatics::SpawnDecalAtLocation(
							GetWorld(),
							BloodDecalMaterial,
							BloodDecalSize,
							HitLocation,
							SpawnDecalRotation,
							BloodDecalLifeTime
						);
					}
				}
			}

			BleedingTimer = 0.0f;


		}
	}
}

void ADBDSurvivor::StartDrop()
{
	// Find front location of current pallet
	FVector VaultStartLocation = CurrentPallet->StartLocation[0];
	FVector VaultEndLocation = CurrentPallet->StartLocation[1];
	bool isLeft = false;

	double MinDistance = FVector::Distance(GetActorLocation(), VaultStartLocation);
	if (MinDistance > FVector::Distance(GetActorLocation(), CurrentPallet->StartLocation[1]))
	{
		VaultStartLocation = VaultEndLocation;
		VaultEndLocation = CurrentPallet->StartLocation[0];
		isLeft = true;
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, FString::Printf(TEXT("pallet0")));
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, FString::Printf(TEXT("pallet1")));
	}

	VaultStartLocation.Z = GetActorLocation().Z;
	VaultEndLocation.Z = GetActorLocation().Z;

	// Move to front of current pallet
	SetActorLocation(VaultStartLocation);
	FVector PalletTriggerBoxLocation = CurrentPallet->TriggerBox->GetComponentTransform().TransformPosition(CurrentPallet->TriggerBox->GetRelativeLocation() - FVector({ 0.0f,80.0f,0.0f }));
	FRotator VaultRotation = (PalletTriggerBoxLocation - GetActorLocation()).Rotation();
	VaultRotation.Pitch = 0.0f;
	VaultRotation.Roll = 0.0f;
	SetActorRotation(VaultRotation);

	CurrentPallet->StartDrop();
	bIsDropping = true;

	if (isLeft)
	{
		if (LeftPalletDropAnim)
		{
			PlayAnimMontage(LeftPalletDropAnim);
		}
	}
	else
	{
		if (RightPalletDropAnim)
		{
			PlayAnimMontage(RightPalletDropAnim);
		}
	}
}

void ADBDSurvivor::StopDrop()
{
	bIsDropping = false;
	bCanDrop = false;
	CurrentPallet->EndDrop();

	if (CurrentPallet)
	{
		if (CurrentPallet->IsOverlappingActor(this))
		{
			BeginOverlapPalletVault();
		}
	}
}

void ADBDSurvivor::HandleHealthState()
{
	if (CurrentHealthStateEnum == EHealthState::DeepWound)
	{
		GetCharacterMovement()->MaxWalkSpeed = CrawlSpeed;

		if (bIsInteracting)
		{
			if (CurrentInteractionState == ESurvivorInteraction::Repair)
			{
				StopRepairGenerator();
			}
			if (CurrentInteractionState == ESurvivorInteraction::Heal)
			{
				StopHealSurvivor();
			}
		}
		if (bIsCrouched)
		{
			UnCrouch();
		}
		return;
	}
}

void ADBDSurvivor::Server_StartDrop_Implementation()
{
	StartDrop();
	MultiCast_StartDrop();
}

void ADBDSurvivor::Server_StopDrop_Implementation()
{
	StopDrop();
	MultiCast_StopDrop();
}

void ADBDSurvivor::MultiCast_StartDrop_Implementation()
{
	if (!IsLocallyControlled())
	{
		StartDrop();
	}
}

void ADBDSurvivor::MultiCast_StopDrop_Implementation()
{
	if (!IsLocallyControlled())
	{
		StopDrop();
	}
}

void ADBDSurvivor::Server_TakeDamage_Implementation()
{
	if (CurrentHealthStateEnum == EHealthState::Healthy)
	{
		if (OwnedPlayerState)
		{
			OwnedPlayerState->SetHealthState(EHealthState::Injured);
		}
	}
	else if (CurrentHealthStateEnum == EHealthState::Injured)
	{
		if (OwnedPlayerState)
		{
			OwnedPlayerState->SetHealthState(EHealthState::DeepWound);
		}
	}
}

void ADBDSurvivor::StartUnhook()
{
	if (CurrentInteractionState != ESurvivorInteraction::UnHook)
	{
		return;
	}

	bIsUnHooking = true;

	FVector UnhookLocation = GetActorLocation();
	FRotator UnhookRotation = GetActorRotation();
	FName SocketName = "unhookSocket";
	if (CurrentTargetSurvivor)
	{
		if (CurrentTargetSurvivor->GetMesh()->DoesSocketExist(SocketName))
		{
			UnhookLocation = CurrentTargetSurvivor->GetMesh()->GetSocketLocation(SocketName);
			UnhookLocation.Z = GetActorLocation().Z;
		}

		UnhookRotation = (CurrentTargetSurvivor->GetActorLocation() - GetActorLocation()).Rotation();
	}

	Server_Teleport(UnhookLocation);
	SetActorRotation(UnhookRotation);
	
	if (CurrentTargetSurvivor)
	{
		CurrentTargetSurvivor->StartBeingUnhooked();
	}
	if (UnhookingAnim)
	{
		PlayAnimMontage(UnhookingAnim);
	}
}

void ADBDSurvivor::StopUnhook()
{
	bIsUnHooking = false;

	if (CurrentTargetSurvivor)
	{
		CurrentTargetSurvivor->StopBeingUnhooked();
	}
	if (UnhookingAnim)
	{
		StopAnimMontage(UnhookingAnim);
	}
}

void ADBDSurvivor::Server_StartUnhook_Implementation()
{
	StartUnhook();
	MultiCast_StartUnhook();
}

void ADBDSurvivor::Server_StopUnhook_Implementation()
{
	StopUnhook();
	MultiCast_StopUnhook();
}

void ADBDSurvivor::MultiCast_StartUnhook_Implementation()
{
	if (!IsLocallyControlled())
	{
		StartUnhook();
	}
}

void ADBDSurvivor::MultiCast_StopUnhook_Implementation()
{
	if (!IsLocallyControlled())
	{
		StopUnhook();
	}
}

void ADBDSurvivor::HandleHookStageRate(float DeltaTime)
{
	if (CurrentHealthStateEnum != EHealthState::Hooked)
	{
		return;
	}

	if (OwnedPlayerState)
	{
		CurrentHookStageRate = OwnedPlayerState->CurrentHookStageRate;
		CurrentHookStageType = OwnedPlayerState->CurrentHookStageType;
	}

	if (CurrentHookStageType == int8(1) && CurrentHookStageRate <= 50.0f)
	{
		CurrentHookStageType = int8(2);
		StartStruggleStage();
	}
	if (CurrentHookStageType == int8(2) && CurrentHookStageRate <= 0.0f)
	{

	}
}

void ADBDSurvivor::Server_HandleHookStageRate(float DeltaTime)
{
	if (CurrentHealthStateEnum != EHealthState::Hooked && !HasAuthority())
	{
		return;
	}

	float RateSpeed = 100.0f / 140.0f;
	if (OwnedPlayerState)
	{
		OwnedPlayerState->CurrentHookStageRate -= RateSpeed * DeltaTime;

		if (OwnedPlayerState->CurrentHookStageType == int8(1) && OwnedPlayerState->CurrentHookStageRate <= 50.0f)
		{
			OwnedPlayerState->CurrentHookStageType = int8(2);
		}
		else if(OwnedPlayerState->CurrentHookStageType == int8(2) && OwnedPlayerState->CurrentHookStageRate <= 0.0f)
		{
			if (HasAuthority())
			{
				Server_SurvivorDeath();
			}
		}
	}
}

void ADBDSurvivor::StartStruggleStage()
{
	if (CurrentHealthStateEnum != EHealthState::Hooked || CurrentHookStageType != int8(2) || !IsLocallyControlled())
	{
		return;
	}

	GetWorld()->GetTimerManager().SetTimer(
		StruggleStageTimer,
		this,
		&ADBDSurvivor::SetStruggleStage,
		10.0f,
		false
	);
}

void ADBDSurvivor::SetStruggleStage()
{
	StartStruggleSkillCheck();

	GetWorld()->GetTimerManager().SetTimer(
		SkillCheckTriggerTimer,
		this,
		&ADBDSurvivor::StartStruggleSkillCheck,
		60.0f / 11.0f,
		true,
		60.0f / 11.0f
	);
}

void ADBDSurvivor::StopStruggleStage()
{
	GetWorld()->GetTimerManager().ClearTimer(StruggleStageTimer);
	GetWorld()->GetTimerManager().ClearTimer(SkillCheckTriggerTimer);
}

void ADBDSurvivor::StartSelfUnhook()
{
	if (CurrentHealthStateEnum != EHealthState::Hooked || CurrentHookStageType != int8(1))
	{
		return;
	}

	if (TrySelfUnhookAnim)
	{
		PlayAnimMontage(TrySelfUnhookAnim);
	}
	
	CurrentSelfUnhookingRate = 0.0f;
	bIsSelfUnhooking = true;
}

void ADBDSurvivor::SelfUnhooking(float DeltaTime)
{
	if (!bIsSelfUnhooking)
	{
		return;
	}
	
	float RateSpeed = 100.0f / 1.5f;
	CurrentSelfUnhookingRate += RateSpeed * DeltaTime;

	if (PC)
	{
		PC->ShowInteractionProgress(CurrentSelfUnhookingRate);
	}

	if (CurrentSelfUnhookingRate >= 100.0f)
	{
		StopSelfUnhook();
		TrySelfUnhook();
	}
}

void ADBDSurvivor::StopSelfUnhook()
{
	if (TrySelfUnhookAnim)
	{
		StopAnimMontage(TrySelfUnhookAnim);
	}

	if (PC)
	{
		PC->HideInteractionProgress();
	}

	bIsSelfUnhooking = false;
}

void ADBDSurvivor::TrySelfUnhook()
{
	float Chance = FMath::FRand();
	
	// Success
	if (Chance <= 0.04)
	{
		if (SuccessSelfUnhookAnim)
		{
			PlayAnimMontage(SuccessSelfUnhookAnim);
		}
	}
	// Fail
	else
	{
		CurrentHookStageRate -= 100.0f / 140.0f * 20.0f;
	}
}

void ADBDSurvivor::HandleSpawnScratchMark(float DeltaTime)
{
	if (!bIsSprinting || GetVelocity().Size() < 380.0f || !bCanShowScratch)
	{
		return;
	}

	ScratchTimer += DeltaTime;

	if (ScratchTimer >= 0.4f)
	{
		SpawnScratchMark();

		ScratchTimer = 0.0f;
	}
}

void ADBDSurvivor::SpawnScratchMark()
{
	UCharacterMovementComponent* CMC = GetCharacterMovement();
	if (CMC)
	{
		FFindFloorResult& FloorResult = CMC->CurrentFloor;

		if (FloorResult.bBlockingHit && FloorResult.IsWalkableFloor())
		{
			int32 SpawnCount = FMath::RandRange(5, 9);

			FVector HitLocation = FloorResult.HitResult.Location;
			FVector HitNormal = FloorResult.HitResult.ImpactNormal;

			FRotator DecalOrientation = (-HitNormal).Rotation();

			// Select surface axis that can move
			FVector SurfaceRight = FRotationMatrix(DecalOrientation).GetScaledAxis(EAxis::Y);
			FVector SurfaceUp = FRotationMatrix(DecalOrientation).GetScaledAxis(EAxis::Z);

			for (int32 i = 0; i < SpawnCount; i++)

			{
				float RandY = FMath::RandRange(-50.0f, 50.0f);
				float RandZ = FMath::RandRange(-50.0f, 50.0f);

				FVector SpawnLocation = HitLocation + (SurfaceRight * RandY) + (SurfaceUp * RandZ);

				FRotator SpawnRot = DecalOrientation;
				SpawnRot.Roll = FMath::RandRange(0.0f, 360.0f);
				/*SpawnRot.Pitch = -90.0f;*/

				if (ScratchDecalMaterial)
				{
					UGameplayStatics::SpawnDecalAtLocation(
						GetWorld(),
						ScratchDecalMaterial,
						ScratchDecalSize,
						SpawnLocation,
						SpawnRot,
						10.0f
					);
				}
			}
		}
	}
}


void ADBDSurvivor::PlayTrerrorRadiusSound()
{
	if (!KillerActorInGame || !HeartBeatSound || !IsLocallyControlled())
	{
		return;
	}

	float Distance = FVector::Dist(KillerActorInGame->GetActorLocation(), this->GetActorLocation());

	if(	Distance > 3200.0f)
	{
		HeartBeatSound->Deactivate();
		return;
	}
	else
	{
		if (!HeartBeatSound->IsPlaying())
		{
			HeartBeatSound->Activate(true);
			GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, FString::Printf(TEXT("Heart Beat play")));
		}

		float FearFactor = FMath::Clamp(1.0f - (Distance / 3200.0f), 0.0f, 1.0f);

		HeartBeatSound->SetVolumeMultiplier(FearFactor * 1.5f);
		HeartBeatSound->SetPitchMultiplier(1.0f + FearFactor);
	}

}

void ADBDSurvivor::InitPlayerState()
{
	OwnedPlayerState = GetPlayerState<ADBDMainPlayerState>();
	if (OwnedPlayerState)
	{
		OwnedPlayerState->OnHealthStateChanged.AddDynamic(this, &ADBDSurvivor::UpdateLocalHealthState);
		UpdateLocalHealthState(OwnedPlayerState->GetCurrentHealthState());
	}
}

void ADBDSurvivor::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	InitPlayerState();
}

void ADBDSurvivor::UpdateLocalHealthState(EHealthState NewState)
{
	CurrentHealthStateEnum = NewState;
}

void ADBDSurvivor::Server_SurvivorEscape_Implementation()
{
	ADBDGameModeBase* GM = GetWorld()->GetAuthGameMode<ADBDGameModeBase>();
	APlayerController* OwnedPC = GetController<APlayerController>();
	if (GM && OwnedPC)
	{
		GM->OnSurvivorEscaped(OwnedPC);
	}
}

void ADBDSurvivor::Server_SurvivorDeath_Implementation()
{
	ADBDGameModeBase* GM = GetWorld()->GetAuthGameMode<ADBDGameModeBase>();
	APlayerController* OwnedPC = GetController<APlayerController>();
	if (GM && OwnedPC)
	{
		GM->OnSurvivorDied(OwnedPC);
	}
}

void ADBDSurvivor::StartOpeningGate()
{
	bIsInteracting = true;
	bIsOpeningGate = true;
}

void ADBDSurvivor::StopOpeningGate()
{
	bIsInteracting = false;
	bIsOpeningGate = false;
}

void ADBDSurvivor::HandleOpeningGate()
{
	if (CurrentInteractionState == ESurvivorInteraction::Lever && bIsInteracting)
	{
		if (CurrentTargetLever)
		{
			if (IsLocallyControlled() && PC)
			{
				PC->ShowInteractionProgress(CurrentTargetLever->CurrentLeverRate);
			}

			if (CurrentTargetLever->bIsOpened)
			{
				StopOpeningGate();
			}
		}
	}
}

void ADBDSurvivor::Server_SetCurrentLever_Implementation(ADBDGateLeverSwitchActor* NewLever)
{
	CurrentTargetLever = NewLever;
}

void ADBDSurvivor::Server_HandleOpeningGate(float DeltaTime)
{
	if (!HasAuthority() || !bIsOpeningGate || !CurrentTargetLever)
	{
		return;
	}

	float RateSpeed = 100.0f / 20.0f; // temp value for easy test

	CurrentTargetLever->CurrentLeverRate += RateSpeed * DeltaTime;

	if (CurrentTargetLever->CurrentLeverRate >= 100.0f)
	{
		CurrentTargetLever->OpenExitGate();
	}
}

void ADBDSurvivor::Server_StartOpeningGate_Implementation()
{
	StartOpeningGate();
	Multicast_StartOpeningGate();
}

void ADBDSurvivor::Server_StopOpeningGate_Implementation()
{
	StopOpeningGate();
	Multicast_StopOpeningGate();
}

void ADBDSurvivor::Multicast_StartOpeningGate_Implementation()
{
	if (!IsLocallyControlled())
	{
		StartOpeningGate();
	}
}

void ADBDSurvivor::Multicast_StopOpeningGate_Implementation()
{
	if (!IsLocallyControlled())
	{
		StopOpeningGate();
	}
}
