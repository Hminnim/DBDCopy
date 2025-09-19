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
#include "Animation/AnimMontage.h"
#include "Animation/AnimInstance.h"
#include "DBDGeneratorActor.h"
#include "DBDWindowActor.h"
#include "DBDPlayerController.h" 
#include "DBDBloodDecalActor.h"
#include "DBDPalletActor.h"
#include "DBDHookActor.h"
#include "DBDSurvivor.generated.h"

UENUM(BlueprintType)
enum class EHealthState : uint8
{
	Healthy		UMETA(DisplayName = "Healthy"),
	Injured		UMETA(DisplayName = "Injured"),
	DeepWound	UMETA(DisplayName = "DeepWound"),
	Carried		UMETA(DisplayName = "Carried"),
	Hooked		UMETA(DisplayName = "Hooked")
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
	UPROPERTY(ReplicatedUsing = OnRep_IsSprinting)
	bool bIsSprinting = false;
	UPROPERTY(Replicated)
	bool bIsInteracting = false;
	bool bIsActing = false;
	bool bCanVault = false;
	UPROPERTY(Replicated)
	bool bIsVaulting = false;
	bool bCanDrop = false;
	bool bIsDropping = false;
	bool bCanCharacterChange = false;
	bool bIsHealing = false;
	UPROPERTY(Replicated)
	bool bIsHealed = false;
	bool bIsUnHooking = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	UAnimMontage* VaultSlowAnim;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	UAnimMontage* VaultFastAnim;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	UAnimMontage* BeingUnhookedAnim;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	UAnimMontage* UnhookingAnim;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadOnly, Category = "HealthState")
	EHealthState CurrentHealthStateEnum = EHealthState::Healthy;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blood")
	TSubclassOf<ADBDBloodDecalActor> BloodDecalClass;

	UFUNCTION()
	void OnTakeDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser);

	// For healing 
	UPROPERTY(Replicated)
	float CurrentHealedRate = 0.0f;

	// To vault window
	void BeginOverlapWindowVault();
	void EndOverlapWindowVault();
	void SetCurrentWindow(ADBDWindowActor* Target);

	// To drop pallet
	void BeginOverlapPallet();
	void EndOverlapPallet();
	void BeginOverlapPalletVault();
	void EndOverlapPalletVault();
	void SetCurrentPallet(ADBDPalletActor* Target);

	// To change character
	void BeginOverlapCharacterChange();
	void EndOverlapCharacterChange();

	// To be picked up and carried by killer
	void BeCarried();
	void StopBeCarried();

	// To be hooked by killer
	void BeHooked();
	void StopBeHooked();

	// To be unhooked by survivor;
	void StartBeingUnhooked();
	void StopBeingUnhooked();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// Cached player controller
	ADBDPlayerController* PC;

	// Components
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
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

	// Only Server
	UFUNCTION(Server, Reliable)
	void Server_Teleport(FVector NewLocation);

	// Sprint
	void StartSprinting();
	void StopSprinting();
	void UpdateMovementSpeed();
	UFUNCTION()
	void OnRep_IsSprinting();
	UFUNCTION(Server, Reliable)
	void Server_SetSprinting(bool bNewSprinting);

    // Vault
	void StartVault();
	void StopVault();
	UFUNCTION()
	void VaultAnimNotifyBeginHandler(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointPayload);
	int8 VaultType = 0;
	UFUNCTION(Server, Reliable)
	void Server_StartVault();
	UFUNCTION(Server, Reliable)
	void Server_StopVault();
	UFUNCTION(NetMulticast, Reliable)
	void MultiCast_StartVault();
	UFUNCTION(NetMulticast, Reliable)
	void MultiCast_StopVault();
	//UFUNCTION(Client, Reliable)
	//void CancelVault();

	// Survivor intreact function
	void FindInteratable();

	// Repair generator
	void StartRepairGenerator();
	void StopRepairGenerator();
	void HandleRepaiGenerator(float DeltaTime);
	float RepairSpeed[5] = { 0.0f, 1.25f, 1.0525f, 0.875f, 0.6875f };
	ADBDGeneratorActor* CurrentGenerator;
	UFUNCTION(Server, Reliable)
	void Server_StartRepairGenerator();
	UFUNCTION(Server, Reliable)
	void Server_StopRepairGenerator();
	UFUNCTION(Server, Reliable)
	void Server_UpdateGeneratorRepairRate(float Amount);
	UFUNCTION(Server, Reliable)
	void Server_SetCurrentGenerator(ADBDGeneratorActor* TargetGenerator);
	UFUNCTION(NetMulticast, Reliable)
	void MultiCast_StartRepairGenerator();
	UFUNCTION(NetMulticast, Reliable)
	void MultiCast_StopRepairGenerator();
	bool bIsReparingGenerator = false; // For server
	void Server_HandleRepairGenerator(float DeltaTIme);

	// Skill check
	void StartSkillCheck();
	void FailedSkillCheck();
	void HandleSkillCheck(int8 Type);
	UFUNCTION()
	void TryTriggerSkillCheck();

	// Heal survivor
	void StartHealSurvivor();
	void StopHealSurvivor();
	void HandleHealSurvivor(float DeltaTime);
	void HandleHealed();
	UFUNCTION(Server, Reliable)
	void Server_SetTargetSurvivor(ADBDSurvivor* TargetSurvivor);
	UFUNCTION(Server, Reliable)
	void Server_StartHealSurvivor();
	UFUNCTION(Server, Reliable)
	void Server_StopHealSurvivor();
	UFUNCTION(Server, Reliable)
	void Server_UpdateTargetSurvivorHealRate(float Amount);
	void Server_HandleHealSurvivor(float DeltaTime);
	UFUNCTION(NetMulticast, Reliable)
	void MultiCast_StartHealSurvivor();
	UFUNCTION(NetMulticast, Reliable)
	void MultiCast_StopHealSurvivor();

	// Survivor interaction enum
	enum class ESurvivorInteraction
	{
		Idle,
		Repair,
		Heal,
		UnHook
	};
	ESurvivorInteraction CurrentInteractionState = ESurvivorInteraction::Idle;
	
	// Bleeding
	void HandleBleeding(float DeltaTime);
	float BleedingTimer = 0.0f;

	// Drop pallet
	void StartDrop();
	void StopDrop();
	UFUNCTION(Server, Reliable)
	void Server_StartDrop();
	UFUNCTION(Server, Reliable)
	void Server_StopDrop();
	UFUNCTION(NetMulticast, Reliable)
	void MultiCast_StartDrop();
	UFUNCTION(NetMulticast, Reliable)
	void MultiCast_StopDrop();

	// Handle Survivor's health state
	UFUNCTION(Server, Reliable)
	void Server_TakeDamage();
	void HandleHealthState();

	// Unhooking survivor
	void StartUnhook();
	void StopUnhook();
	UFUNCTION(Server, Reliable)
	void Server_StartUnhook();
	UFUNCTION(Server, Reliable)
	void Server_StopUnhook();
	UFUNCTION(NetMulticast, Reliable)
	void MultiCast_StartUnhook();
	UFUNCTION(NetMulticast, Reliable)
	void MultiCast_StopUnhook();


	// Values
	ADBDWindowActor* CurrentWindow;
	ADBDPalletActor* CurrentPallet;
	ADBDSurvivor* CurrentTargetSurvivor;
	FTimerHandle SkillCheckTimer;
	FTimerHandle SkillCheckTriggerTimer;
	FTimerHandle VaultTimer;
};
