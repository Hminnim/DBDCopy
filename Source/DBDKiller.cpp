// Fill out your copyright notice in the Description page of Project Settings.


#include "DBDKiller.h"
#include "DBDSurvivor.h"
#include "DBDPlayerController.h"
#include "DBDPalletActor.h"
#include "DBDGeneratorActor.h"
#include "DBDWindowActor.h"
#include "DBDHookActor.h"
#include "Camera/CameraComponent.h"
#include "Components/SpotLightComponent.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/PostProcessComponent.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h" 
#include "Net/UnrealNetwork.h"
#include "Engine/Engine.h"

// Sets default values
ADBDKiller::ADBDKiller()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	bReplicates = true;
	GetMesh()->bReceivesDecals = false;

	// Set default character movement
	bUseControllerRotationYaw = true;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->bUseControllerDesiredRotation = true;
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;

	// Capsule Component
	GetCapsuleComponent()->SetCollisionProfileName(FName("Killer"));

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

	// Finding survivor box config
	FindingSurvivorBox = CreateDefaultSubobject<UBoxComponent>(TEXT("FindingSurvivorBox"));
	FindingSurvivorBox->SetupAttachment(GetCapsuleComponent());
	FindingSurvivorBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	FindingSurvivorBox->SetCollisionObjectType(ECC_WorldDynamic);
	FindingSurvivorBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	FindingSurvivorBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	FindingSurvivorBox->SetRelativeLocation(FVector(0.0f, 0.0f, -60.0f));

	// Postprocess component
	AuraPostProcessComponent = CreateDefaultSubobject<UPostProcessComponent>(TEXT("AuraPostProcesComponent"));
	AuraPostProcessComponent->SetupAttachment(RootComponent);
	AuraPostProcessComponent->bUnbound = true;
}

// Overlap function
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

void ADBDKiller::BeStunned()
{
	bIsStunned = true;

	// Stop actions by being stunned
	if (bIsAttacking)
	{
		EndAttack();
	}
	if (bIsPickingUp)
	{
		StopPickUp();
	}
	if (bIsCarrying)
	{
		StopCarryingSurvivor();
	}	

	if (StunPalletAnim)
	{
		PlayAnimMontage(StunPalletAnim);
	}
}

void ADBDKiller::ChangeWiggleIntensity(int8 Type)
{
	// Great Skill Check
	if (Type == int8(0))
	{
		CurrentWiggleIntensity = WiggleIntensity * 1.5f;
	}
	// Good Skill Check
	if (Type == int8(1))
	{
		CurrentWiggleIntensity = WiggleIntensity * 0.5f;
	}
	// Failled Skill Check
	if (Type == int8(2))
	{
		CurrentWiggleIntensity = 0.0f;
	}
}

void ADBDKiller::OnAllGeneratorCompleted()
{
	EnableLeverAura();
	DisableGeneratorAura();
}

// Called when the game starts or when spawned
void ADBDKiller::BeginPlay()
{
	Super::BeginPlay();
	
	// Set RedStain's Visibility
	if (IsLocallyControlled())
	{
		PC = Cast<ADBDPlayerController>(GetController());
		RedStain->SetVisibility(false);
	}
	else
	{
		RedStain->SetVisibility(true);
	}

	// FindingSurvivorBox binding
	if (FindingSurvivorBox)
	{
		FindingSurvivorBox->OnComponentBeginOverlap.AddDynamic(this, &ADBDKiller::OnSurvivorOverlapBegin);
		FindingSurvivorBox->OnComponentEndOverlap.AddDynamic(this, &ADBDKiller::OnSurvivorOverlapEnd);
	}

	// AnimMontage Notify binding
	UAnimInstance* AnimInstace = GetMesh()->GetAnimInstance();
	if (AnimInstace)
	{
		AnimInstace->OnPlayMontageNotifyBegin.AddDynamic(this, &ADBDKiller::AnimNotifyBeginHandler);
	}

	// AuraMaterial
	if (IsLocallyControlled())
	{
		if (AuraMaterialAsset)
		{
			AuraMaterialInstance = UMaterialInstanceDynamic::Create(AuraMaterialAsset, this);

			if (AuraMaterialInstance)
			{
				AuraPostProcessComponent->Settings.AddBlendable(AuraMaterialInstance, 1.0f);
				EnableGeneratorAura();
				DisableHookAura();
				DisableLeverAura();
			}
		}
	}
	else
	{
		AuraPostProcessComponent->bEnabled = false;
	}

	if (IsLocallyControlled())
	{
		FTimerHandle FindSurvivor;
		GetWorld()->GetTimerManager().SetTimer(
			FindSurvivor,
			this,
			&ADBDKiller::ShowSurvivorScratchMark,
			2.0f,
			false
		);
	}
}

void ADBDKiller::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ADBDKiller, CurrentPallet);
	DOREPLIFETIME(ADBDKiller, CurrentGenerator);
	DOREPLIFETIME(ADBDKiller, CurrentTargetSurvivor);
	DOREPLIFETIME(ADBDKiller, CurrentWindow);
	DOREPLIFETIME(ADBDKiller, CurrentHook);
	DOREPLIFETIME(ADBDKiller, bCanPickUp);
	DOREPLIFETIME(ADBDKiller, bIsCarrying);
}

// Called every frame
void ADBDKiller::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	FindActable();
	HandleTargetSurvivor();

	// Handle carrying surivor
	if (bIsCarrying)
	{
		if (CurrentTargetSurvivor)
		{
			HandleWiggleStrape();

			if (CurrentTargetSurvivor->CurrentWiggleRate >= 100.0f)
			{
				if (IsLocallyControlled())
				{
					GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, FString::Printf(TEXT("Success Wiggle Killer")));
				}
				else
				{
					if (HasAuthority())
					{
						GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, FString::Printf(TEXT("Success Wiggle Server")));
					}
					else
					{
						GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, FString::Printf(TEXT("Success Wiggle Survivor")));
					}					
				}
				StopCarryingSurvivor();
			}
		}
	}
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
	if (bIsBreakingPallet || bIsBreakingGenerator || bIsVaulting || bIsHooking || bIsPickingUp || bIsStunned)
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
	if (bIsBreakingPallet || bIsBreakingGenerator || bIsVaulting || bIsHooking || bIsPickingUp || bIsStunned)
	{
		return;
	}

	const FVector2D LookValue = Value.Get<FVector2D>();

	AddControllerYawInput(LookValue.X);
	AddControllerPitchInput(-LookValue.Y);
}

void ADBDKiller::Interact(const FInputActionValue& Value)
{
	if ( bIsBreakingPallet || bIsBreakingGenerator || !bCanAttack || bIsAttacking || bIsVaulting || bIsHooking || bIsPickingUp || bIsStunned)
	{
		return;
	}

	if (Value.Get<bool>() && bCanAttack && !bIsLunging)
	{
		StartLunge();
	}
	else if (Value.Get<bool>() == false && bCanAttack && bIsLunging)
	{
		StartAttack();
	}
}

void ADBDKiller::Action(const FInputActionValue& Value)
{
	if (bIsAttacking || bIsVaulting || bIsBreakingGenerator || bIsBreakingPallet || bIsHooking || bIsPickingUp || bIsHooking || bIsStunned)
	{
		return;
	}

	if (bCanCharacterChange)
	{
		if (PC)
		{
			PC->HideActionMessage();
			UDBDGameInstance* GI = Cast<UDBDGameInstance>(GetGameInstance());
			if (GI)
			{
				PC->CharacterChange(GI->bIsKiller);
			}
		}
	}
	if (bCanBreakPallet)
	{
		BreakPallet();
		Server_BreakPallet();
		return;
	}
	if (bCanBreakGenerator)
	{
		BreakGenerator();
		Server_BreakGenerator();
		return;
	}
	if (bCanVault)
	{
		Vault();
		Server_Vault();
		return;
	}
	if (bCanPickUp)
	{
		TryPickUp();
		return;
	}
	if (bCanHook)
	{
		StartHookSurvivor();
		Server_StartHookSurvivor();
		return;
	}
}

void ADBDKiller::FindActable()
{
	if (bIsBreakingPallet || bIsBreakingGenerator || bIsAttacking || bIsVaulting || bIsPickingUp || bIsHooking || bIsStunned)
	{
		return;
	}

	FVector FireStart = Camera->GetComponentLocation() + Camera->GetForwardVector();
	FVector FireEnd = (Camera->GetForwardVector() * 150) + FireStart;

	FHitResult HitResult;

	if (GetWorld()->LineTraceSingleByChannel(HitResult, FireStart, FireEnd, ECollisionChannel::ECC_Visibility))
	{
		if (HitResult.GetActor())
		{
			// If hit hook while carrying survivor
			if (bIsCarrying)
			{
				ADBDHookActor* HitHook = Cast <ADBDHookActor>(HitResult.GetActor());
				if (HitHook)
				{
					CurrentHook = HitHook;
					if (bIsCarrying && CurrentTargetSurvivor)
					{
						if (PC)
						{
							PC->ShowActionMessage("Press Space to hook");
						}
						SetCurrentHook(HitHook);
						bCanHook = true;
					}
				}

				// While carrying, only can hook
				return;
			}

			// If hit dropped pallet
			ADBDPalletActor* HitPallet = Cast<ADBDPalletActor>(HitResult.GetActor());
			if (HitPallet)
			{
				if (HitPallet->bIsDropped)
				{
					CurrentPallet = HitPallet;
					if (IsLocallyControlled())
					{
						Server_SetCurrentPallet(HitPallet);
						PC->ShowActionMessage("Press Space to break");
					}
					bCanBreakPallet = true;
					return;
				}
			}

			// If hit being repaired generator
			ADBDGeneratorActor* HitGenerator = Cast<ADBDGeneratorActor>(HitResult.GetActor());
			if (HitGenerator)
			{
				CurrentGenerator = HitGenerator;
				if (IsLocallyControlled())
				{
					Server_SetCurrenetGenerator(HitGenerator);
					PC->ShowInteractionProgress(CurrentGenerator->CurrentRepairRate);
				}				
				if (CurrentGenerator->CurrentRepairRate > 0.0f)
				{
					if (IsLocallyControlled())
					{
						PC->ShowActionMessage("Press Space to break");
					}
					bCanBreakGenerator = true;
					return;
				}
			}

			// If hit window
			ADBDWindowActor* HitWindow = Cast<ADBDWindowActor>(HitResult.GetActor());
			if (HitWindow)
			{
				CurrentWindow = HitWindow;
				if (IsLocallyControlled())
				{
					Server_SetCurrentWindow(HitWindow);
					PC->ShowActionMessage("Press Space to vault");
				}
				bCanVault = true;
				return;
			}
		}
	}

	// Handle UI and state
	if (PC && IsLocallyControlled())
	{
		if (!bCanPickUp && !bCanCharacterChange && !bCanHook && !bCanVault)
		{
			PC->HideActionMessage();
			PC->HideInteractionProgress();
		}
	}
	bCanBreakPallet = false;
	bCanBreakGenerator = false;
	bCanHook = false;
	bCanVault = false;
}

void ADBDKiller::AnimNotifyBeginHandler(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointPayload)
{
	if (NotifyName == "EndVault")
	{
		StopVault();
	}
	if (NotifyName == "EndBeStunned")
	{
		EndBeStunned();
	}
	if (NotifyName == "EndAttack")
	{
		EndAttack();
	}
	if (NotifyName == "EndLunge")
	{
		HoldLunge();
	}
	if (NotifyName == "EndSwing")
	{
		TryAttack();
	}
}

void ADBDKiller::BreakPallet()
{
	if (!CurrentPallet)
	{
		return;
	}

	bIsBreakingPallet = true;

	if (PC)
	{
		PC->HideActionMessage();
	}

	if (CurrentPallet)
	{
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
		PalletFrontLocation.Z = GetActorLocation().Z;
		SetActorLocation(PalletFrontLocation, false);

		FVector PalletTriggerBoxLocation = CurrentPallet->TriggerBox->GetComponentTransform().TransformPosition(CurrentPallet->TriggerBox->GetRelativeLocation() - FVector({ 30.0f,0.0f,0.0f }));
		FRotator PalletRotation = (PalletTriggerBoxLocation - GetActorLocation()).Rotation();
		PalletRotation.Pitch = 0.0f;
		PalletRotation.Roll = 0.0f;
		SetActorRotation(PalletRotation);
		CurrentPallet->StartBreak();
	}

	PlayAnimMontage(BreakAnim);

	if (IsLocallyControlled())
	{
		GetWorld()->GetTimerManager().SetTimer
		(
			BreakTimerHandle,
			FTimerDelegate::CreateLambda([&]() {
				EndBreakPallet();
				if (IsLocallyControlled())
				{
					Server_EndbreakPallet();
				}
				}),
			2.34f,
			false
		);
	}
}

void ADBDKiller::EndBreakPallet()
{
	if (CurrentPallet)
	{
		CurrentPallet->Destroy();
	}
	bIsBreakingPallet = false;
	bCanBreakPallet = false;
}

void ADBDKiller::Server_SetCurrentPallet_Implementation(ADBDPalletActor* TargetPallet)
{
	CurrentPallet = TargetPallet;
}

void ADBDKiller::Server_BreakPallet_Implementation()
{
	BreakPallet();
	MultiCast_BreakPallet();
}

void ADBDKiller::Server_EndbreakPallet_Implementation()
{
	EndBreakPallet();
	MultiCast_EndBreakPallet();
}

void ADBDKiller::MultiCast_BreakPallet_Implementation()
{
	if (!IsLocallyControlled() && !HasAuthority())
	{
		BreakPallet();
	}
}

void ADBDKiller::MultiCast_EndBreakPallet_Implementation()
{
	if (!IsLocallyControlled())
	{
		EndBreakPallet();
	}
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

	if (CurrentGenerator)
	{
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
	}

	PlayAnimMontage(BreakAnim);

	if (IsLocallyControlled())
	{
		GetWorld()->GetTimerManager().SetTimer
		(
			BreakTimerHandle,
			FTimerDelegate::CreateLambda([&]() {
				EndBreakGenerator();
				if (IsLocallyControlled())
				{
					Server_EndBreakGenerator();
				}
			}),
			1.8f,
			false
		);
	}
}

void ADBDKiller::EndBreakGenerator()
{
	bIsBreakingGenerator = false;
	bCanBreakGenerator = false;
	StopAnimMontage();

	if (HasAuthority())
	{
		if (CurrentGenerator)
		{
			CurrentGenerator->CurrentRepairRate = CurrentGenerator->CurrentRepairRate > 2.5f ? CurrentGenerator->CurrentRepairRate - 2.5f : 0.0f;
		}
	}
}

void ADBDKiller::Server_SetCurrenetGenerator_Implementation(ADBDGeneratorActor* TargetGenerator)
{
	CurrentGenerator = TargetGenerator;
}

void ADBDKiller::Server_BreakGenerator_Implementation()
{
	BreakGenerator();
	MultiCast_BreakGenerator();
}

void ADBDKiller::Server_EndBreakGenerator_Implementation()
{
	EndBreakGenerator();
	MultiCast_EndBreakGenerator();
}

void ADBDKiller::MultiCast_BreakGenerator_Implementation()
{
	if (!IsLocallyControlled())
	{
		BreakGenerator();
	}
}

void ADBDKiller::MultiCast_EndBreakGenerator_Implementation()
{
	if (!IsLocallyControlled())
	{
		EndBreakGenerator();
	}
}

void ADBDKiller::StartAttack()
{
	Attack();
	Server_Attack();
}

void ADBDKiller::Attack()
{
	StopLunge();
	bCanAttack = false;
	GetCharacterMovement()->MaxWalkSpeed = 100.0f;
	bIsAttacking = true;

	if (bIsCarrying)
	{
		PlayAnimMontage(CarryingAttackAnim);
	}
	else 
	{
		PlayAnimMontage(AttackAnim);
	}
}

void ADBDKiller::EndAttack()
{
	bCanAttack = true;
	bIsAttacking = false;
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
}

void ADBDKiller::TryAttack()
{
	if (!IsLocallyControlled())
	{
		return;
	}

	FVector FireStart = Camera->GetComponentLocation() + Camera->GetForwardVector() * 50;
	FVector FireEnd = (Camera->GetForwardVector() * 100) + FireStart;
	float AttackRadius = 50.0f;

	FHitResult HitResult;
	int8 HitType = 0;
	bool bHitSurvivor = false;

	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(this);

	bool bHit = UKismetSystemLibrary::SphereTraceSingle(
		GetWorld(),
		FireStart,
		FireEnd,
		AttackRadius,
		UEngineTypes::ConvertToTraceType(ECC_GameTraceChannel1),
		false,
		ActorsToIgnore,
		EDrawDebugTrace::ForDuration,
		HitResult,
		true
	);

	// Hit
	if (bHit)
	{
		ADBDSurvivor* HitSurvivor = Cast<ADBDSurvivor>(HitResult.GetActor());

		// Hit Survivor
		if (HitSurvivor)
		{
			if (HitSurvivor->CurrentHealthStateEnum != EHealthState::DeepWound) 
			{
				Server_ApplyDamgeToSurvivor(HitResult.GetActor());
				HitType = 1;
				bHitSurvivor = true;
			}
		}
		// Hit Object
		else
		{
			HitType = 2;
		}
	}
	
	Server_HandleAttackDelay(bHitSurvivor, HitType);
}

void ADBDKiller::StartLunge()
{
	bIsLunging = true;
	GetCharacterMovement()->MaxWalkSpeed = LungeSpeed;

	if (LungeAnim)
	{
		PlayAnimMontage(LungeAnim);
	}

	Server_StartLunge();

	GetWorld()->GetTimerManager().SetTimer
	(
		LungeTimerHandle,
		this,
		&ADBDKiller::StartAttack,
		MaxLungeTime,
		false
	);
}

void ADBDKiller::HoldLunge()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && LungeAnim)
	{
		AnimInstance->Montage_Pause(LungeAnim);
	}
}

void ADBDKiller::StopLunge()
{
	bIsLunging = false;
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed; 
	GetWorld()->GetTimerManager().ClearTimer(LungeTimerHandle);

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && AnimInstance->Montage_IsPlaying(LungeAnim))
	{
		AnimInstance->Montage_Stop(0.1f, LungeAnim);
	}
}

void ADBDKiller::Server_StartLunge_Implementation()
{
	bIsLunging = true;
	GetCharacterMovement()->MaxWalkSpeed = LungeSpeed;

	MultiCast_StartLunge();
}

void ADBDKiller::MultiCast_StartLunge_Implementation()
{
	bIsLunging = true;
	GetCharacterMovement()->MaxWalkSpeed = LungeSpeed;

	if (LungeAnim && !IsLocallyControlled())
	{
		PlayAnimMontage(LungeAnim);
	}
}

void ADBDKiller::Server_Attack_Implementation()
{
	Attack();
	MultiCast_Attack();
}

void ADBDKiller::MultiCast_Attack_Implementation()
{
	if (!IsLocallyControlled())
	{
		Attack();
	}
}

void ADBDKiller::Server_HandleAttackDelay_Implementation(bool bIsSuccess, int8 type)
{
	Multicast_HandleAttackDelay(bIsSuccess, type);
	float AttackDelay = bIsSuccess ? 2.7f : 1.5f;

	GetWorld()->GetTimerManager().SetTimer
	(
		AttackTimerHandle,
		this,
		&ADBDKiller::EndAttack,
		AttackDelay,
		false
	);
}

void ADBDKiller::Multicast_HandleAttackDelay_Implementation(bool bIsSuccess, int8 type)
{
	float AttackDelay = bIsSuccess ? 2.7f : 1.5f;

	GetWorld()->GetTimerManager().SetTimer
	(
		AttackTimerHandle,
		this,
		&ADBDKiller::EndAttack,
		AttackDelay,
		false
	);

	if (!bIsCarrying)
	{
		// Miss
		if (type == 0 && MissAnim)
		{
			PlayAnimMontage(MissAnim);
		}
		// Hit survivor
		if (type == 1 && HitSurvivorAnim)
		{
			PlayAnimMontage(HitSurvivorAnim);
		}
		// Hit object
		if (type == 2 && HitObjectAnim)
		{
			PlayAnimMontage(HitObjectAnim);
		}
	}
	// Carry Attack Animation is just one way.
}

void ADBDKiller::Server_ApplyDamgeToSurvivor_Implementation(AActor* Survivor)
{
	UGameplayStatics::ApplyDamage(Survivor, 1, GetController(), this, UDamageType::StaticClass());
}

void ADBDKiller::Vault()
{
	if (bIsVaulting)
	{
		return;
	}

	bIsVaulting = true;

	if (CurrentWindow)
	{
		// Find front location of current window
		FVector VaultLocation = CurrentWindow->StartLocation[0];
		double VaultDistance = FVector::Distance(GetActorLocation(), VaultLocation);
		if (VaultDistance > FVector::Distance(GetActorLocation(), CurrentWindow->StartLocation[1]))
		{
			VaultLocation = CurrentWindow->StartLocation[1];
		}
		VaultLocation.Z = GetActorLocation().Z;
		SetActorLocation(VaultLocation);

		FRotator VaultRotation = (CurrentWindow->GetActorLocation() - GetActorLocation()).Rotation();
		VaultRotation.Pitch = 0.0f;

		AController* KillerController = GetController();
		if (KillerController)
		{
			KillerController->SetControlRotation(VaultRotation);
		}
	}
	
	// Change collision
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetCharacterMovement()->SetMovementMode(MOVE_Flying);

	PlayAnimMontage(VaultAnim);
}

void ADBDKiller::StopVault()
{
	bIsVaulting = false;
	
	// Change collision
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetCharacterMovement()->SetMovementMode(MOVE_Walking);
}

void ADBDKiller::Server_Vault_Implementation()
{
	Vault();
	MultiCast_Vault();
}

void ADBDKiller::MultiCast_Vault_Implementation()
{
	if (!IsLocallyControlled())
	{
		Vault();
	}
}

void ADBDKiller::Server_SetCurrentWindow_Implementation(ADBDWindowActor* NewWindow)
{
	CurrentWindow = NewWindow;
}

void ADBDKiller::OnSurvivorOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (HasAuthority())
	{
		ADBDSurvivor* OverlappedSurvivor = Cast<ADBDSurvivor>(OtherActor);
		if (OverlappedSurvivor)
		{
			if (OverlappedSurvivor->CurrentHealthStateEnum == EHealthState::DeepWound)
			{
				bCanPickUp = true;
				CurrentTargetSurvivor = OverlappedSurvivor;
				SetTargetSurvivor(CurrentTargetSurvivor);

				if (PC)
				{
					PC->ShowActionMessage("Press Space to Pick Up");
				}
			}
		}
	}
}

void ADBDKiller::OnSurvivorOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (HasAuthority())
	{
		ADBDSurvivor* OverlappedSurvivor = Cast<ADBDSurvivor>(OtherActor);
		if (OverlappedSurvivor)
		{
			if (CurrentTargetSurvivor)
			{
				if (OverlappedSurvivor == CurrentTargetSurvivor)
				{
					bCanPickUp = false;

					if (PC)
					{
						PC->HideActionMessage();
					}
				}
			}
		}
	}
}

void ADBDKiller::OnRep_CanPickUp()
{
	if (IsLocallyControlled())
	{
		if (PC)
		{
			if (bCanPickUp)
			{
				PC->ShowActionMessage("Press Spece to Pick Up");
			}
			else
			{
				PC->HideActionMessage();
			}
		}
	}
}

void ADBDKiller::SetTargetSurvivor_Implementation(ADBDSurvivor* NewSurvivor)
{
	CurrentTargetSurvivor = NewSurvivor;
}

void ADBDKiller::HandleTargetSurvivor()
{
	if (CurrentTargetSurvivor)
	{
		if (CurrentTargetSurvivor->CurrentHealthStateEnum != EHealthState::DeepWound)
		{
			bCanPickUp = false;
		}
	}
}

void ADBDKiller::TryPickUp()
{
	StartPickUp();
	Server_StartPickUp();
}

void ADBDKiller::StartPickUp()
{
	bCanPickUp = false;

	if (PickUpAnim)
	{
		PlayAnimMontage(PickUpAnim);
	}

	if (PC)
	{
		PC->HideActionMessage();
	}

	EnableHookAura();
}

void ADBDKiller::StopPickUp()
{
	bIsPickingUp = false;

	if (PickUpAnim)
	{
		StopAnimMontage(PickUpAnim);
	}

	DisableHookAura();
}

void ADBDKiller::StartCarryingSurvivor()
{
	bIsCarrying = true;
	bCanPickUp = false;

	if (CurrentTargetSurvivor)
	{
		FName SocketName(TEXT("CarryingSocket"));
		if (GetMesh()->DoesSocketExist(SocketName))
		{
			CurrentTargetSurvivor->BeCarried();

			FAttachmentTransformRules AttachmentRules(EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::KeepWorld, false);
			CurrentTargetSurvivor->AttachToComponent(GetMesh(), AttachmentRules, SocketName);
		}
	}
}

void ADBDKiller::StopCarryingSurvivor()
{
	bIsCarrying = false;
	bCanPickUp = true;

	if (CurrentTargetSurvivor)
	{
		FDetachmentTransformRules DetachmentRules(EDetachmentRule::KeepWorld, EDetachmentRule::KeepWorld, EDetachmentRule::KeepWorld, true);
		CurrentTargetSurvivor->GetRootComponent()->DetachFromComponent(DetachmentRules);
		
		CurrentTargetSurvivor->StopBeCarried();
	}

	DisableHookAura();
}

void ADBDKiller::OnRep_IsCarrying()
{
	if (bIsCarrying)
	{
		StartCarryingSurvivor();
	}
	else
	{
		StopCarryingSurvivor();
	}
}

void ADBDKiller::HandleWiggleStrape()
{
	if (!bIsCarrying || !CurrentTargetSurvivor)
	{
		return;
	}

	if (CurrentTargetSurvivor->bIsWigglePause)
	{
		return;
	}

	float TimeSeconds = GetWorld()->GetTimeSeconds();
	float SineValue = FMath::Sin(TimeSeconds * WiggleSpeed);

	AddMovementInput(GetActorRightVector(), SineValue * WiggleIntensity);
}

void ADBDKiller::TryDropDown()
{
	StartPickUp();
	Server_StartDropDown();
}

void ADBDKiller::Server_StartDropDown_Implementation()
{
	bIsPickingUp = false;

	MultiCast_StartPickUp();

	GetWorld()->GetTimerManager().SetTimer(
		CarryingTimerHandle,
		FTimerDelegate::CreateLambda([&]() {
			StopCarryingSurvivor();
			}),
		0.97f,
		false
	);
	GetWorld()->GetTimerManager().SetTimer(
		PickUpTimerHandle,
		FTimerDelegate::CreateLambda([&]() {
			StopPickUp();
			}),
		2.25f,
		false
	);
}

void ADBDKiller::MultiCast_StartPickUp_Implementation()
{
	if (!IsLocallyControlled())
	{
		StartPickUp();
	}
}

void ADBDKiller::Server_StartPickUp_Implementation()
{
	bIsPickingUp = true;

	MultiCast_StartPickUp();

	GetWorld()->GetTimerManager().SetTimer(
		CarryingTimerHandle,
		FTimerDelegate::CreateLambda([&]() {
			StartCarryingSurvivor();
			}),
		0.97f,
		false
	);
	GetWorld()->GetTimerManager().SetTimer(
		PickUpTimerHandle,
		FTimerDelegate::CreateLambda([&]() {
			StopPickUp();
			}),
		2.25f,
		false
	);
}

void ADBDKiller::StartHookSurvivor()
{
	if (!CurrentHook)
	{
		return;
	}

	bIsHooking = true;
	bCanHook = false;

	// Get location to hook
	FVector HookLocation;
	FName HookSocketName = FName("FrontOfHook");
	if (CurrentHook->HookStaticMesh->DoesSocketExist(HookSocketName))
	{
		HookLocation = CurrentHook->HookStaticMesh->GetSocketLocation(HookSocketName);
	}
	HookLocation.Z = GetActorLocation().Z;

	// Get rotation to hook
	FRotator HookRotation = (CurrentHook->GetActorLocation() - GetActorLocation()).Rotation();
	HookRotation.Pitch = 0.0f;
	HookRotation.Roll = 0.0f;

	// Set location ans rotation
	SetActorLocation(HookLocation);
	SetActorRotation(HookRotation);

	// Play anim
	if (HookAnim)
	{
		PlayAnimMontage(HookAnim);
	}

	GetWorld()->GetTimerManager().SetTimer(
		HookingTimerHandle,
		FTimerDelegate::CreateLambda([&]() {
			HookingSurvivor();
			}),
		0.99f,
		false
	);
	GetWorld()->GetTimerManager().SetTimer(
		HookedTImerHandle,
		FTimerDelegate::CreateLambda([&]() {
			StopHookSurvivor();
			}),
		1.93f,
		false
	);
}

void ADBDKiller::StopHookSurvivor()
{
	bIsHooking = false;

	StopAnimMontage(HookAnim);
}

void ADBDKiller::HookingSurvivor()
{
	StopCarryingSurvivor();

	// Attach to hook's socket
	if (CurrentTargetSurvivor && CurrentHook)
	{
		CurrentTargetSurvivor->BeHooked();
		CurrentHook->OnSurvivorHooked();
		EnableSurvivorHookAura();

		FName SocketName(TEXT("HookingNeedle"));
		if (CurrentHook->HookStaticMesh->DoesSocketExist(SocketName))
		{
			FAttachmentTransformRules AttachmentRules(EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::KeepWorld, false);
			CurrentTargetSurvivor->AttachToComponent(CurrentHook->HookStaticMesh, AttachmentRules, SocketName);
		}
	}
}

void ADBDKiller::SetCurrentHook_Implementation(ADBDHookActor* NewHook)
{
	CurrentHook = NewHook;
}

void ADBDKiller::Server_StartHookSurvivor_Implementation()
{
	StartHookSurvivor();
	MultiCast_StartHookSurvivor();
}

void ADBDKiller::MultiCast_StartHookSurvivor_Implementation()
{
	if (!IsLocallyControlled())
	{
		StartHookSurvivor();
	}
}

void ADBDKiller::EndBeStunned()
{
	bIsStunned = false;
}

void ADBDKiller::EnableGeneratorAura()
{
	if (AuraMaterialInstance)
	{
		AuraMaterialInstance->SetScalarParameterValue("ShowGeneratorAura", 1.0f);
	}
}

void ADBDKiller::DisableGeneratorAura()
{
	if (AuraMaterialInstance)
	{
		AuraMaterialInstance->SetScalarParameterValue("ShowGeneratorAura", 0.0f);
	}
}

void ADBDKiller::EnableHookAura()
{
	if (AuraMaterialInstance)
	{
		AuraMaterialInstance->SetScalarParameterValue("ShowHookAura", 1.0f);
	}
}

void ADBDKiller::DisableHookAura()
{
	if (AuraMaterialInstance)
	{
		AuraMaterialInstance->SetScalarParameterValue("ShowHookAura", 0.0f);
	}
}

void ADBDKiller::EnableSurvivorHookAura()
{
	if (AuraMaterialInstance)
	{
		AuraMaterialInstance->SetScalarParameterValue("ShowSurvivorHookAura", 1.0f);
	}
}

void ADBDKiller::DisableSurvivorHookAura()
{
	if (AuraMaterialInstance)
	{
		AuraMaterialInstance->SetScalarParameterValue("ShowSurvivorHookAura", 0.0f);
	}
}

void ADBDKiller::EnableLeverAura()
{
	if (AuraMaterialInstance)
	{
		AuraMaterialInstance->SetScalarParameterValue("ShowLeverAura", 1.0f);
	}
}

void ADBDKiller::DisableLeverAura()
{
	if (AuraMaterialInstance)
	{
		AuraMaterialInstance->SetScalarParameterValue("ShowLeverAura", 0.0f);
	}
}

void ADBDKiller::ShowSurvivorScratchMark()
{
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ADBDSurvivor::StaticClass(), FoundActors);
	for (AActor* Actor : FoundActors)
	{
		ADBDSurvivor* FoundSurvivor = Cast<ADBDSurvivor>(Actor);
		FoundSurvivor->OnToShowScratchMark();
	}
}
