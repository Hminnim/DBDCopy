// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "DBDGeneratorActor.h"
#include "DBDWindowActor.h"
#include "DBDPlayerController.h" 
#include "DBDSurvivor.generated.h"

UENUM(BlueprintType)
enum class EHealthState : uint8
{
	Healthy		UMETA(DisplayName = "Healthy"),
	Injured		UMETA(DisplayName = "Injured"),
	DeepWound	UMETA(DisplayName = "DeepWound")
};

UCLASS()
class DBDCOPY_API ADBDSurvivor : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ADBDSurvivor();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void NotifyControllerChanged() override;

	// Survivor state flags
	bool bIsSprinting = false;
	bool bIsInteracting = false;
	bool bIsActing = false;
	bool bCanVault = false;
	bool bIsVaulting = false;

	// To vault window
	void BeginOverlapWindow();
	void EndOverlapWindow();
	void SetCurrentWindow(ADBDWindowActor* Target);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	UAnimMontage* VaultAnim;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "HealthState")
	EHealthState CurrentHealthStateEnum = EHealthState::Healthy;

	UFUNCTION()
	void OnTakeDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Components
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
	USpringArmComponent* SpringArm;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
	UCameraComponent* Camera;

	// Survivor speed values
	float WalkSpeed = 226.0f;
	float SprintSpeed = 400.0f;
	float CrouchSpeed = 113.0f;
	float CrawlSpeed = 70.0f;

private:
	// Input action
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"));
	UInputMappingContext* DefaultMappingContext;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"));
	UInputAction* MoveInput;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"));
	UInputAction* LookInput;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"));
	UInputAction* CrouchInput;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"));
	UInputAction* SprintInput;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"));
	UInputAction* InteractInput;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"));
	UInputAction* ActionInput;

	// Survivor input action function
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void HandleCrouch(const FInputActionValue& Value);
	void Sprint(const FInputActionValue& Value);
	void Interact(const FInputActionValue& Value);
	void Action(const FInputActionValue& Value);

	// Survivor intreact function
	void FindInteratable();
	void StartRepairGenerator();
	void StopReapirGenerator();
	void StartSkillCheck();
	void FailedSkillCheck();
	void HandleSkillCheck(int8 Type);
	UFUNCTION()
	void TryTriggerSkillCheck();

	// Survivor interaction enum
	enum class ESurvivorInteraction
	{
		Idle,
		Repair,
		Heal
	};
	ESurvivorInteraction CurrentInteractionState = ESurvivorInteraction::Idle;
	
	// Vault
	void StartVault();
	void StopVault();
	FVector VaultStartLocation;
	FVector VaultEndLocation;
	FVector VaultTopLocation;

	// Values
	ADBDGeneratorActor* CurrentGenerator;
	ADBDWindowActor* CurrentWindow;
	FTimerHandle SkillCheckTimer;
	FTimerHandle SkillCheckTriggerTimer;
	FTimerHandle VaultTimer;
};
