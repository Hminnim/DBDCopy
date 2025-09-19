// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/CameraComponent.h"
#include "Components/SpotLightComponent.h"
#include "Components/BoxComponent.h"
#include "DBDPlayerController.h"
#include "DBDPalletActor.h"
#include "DBDGeneratorActor.h"
#include "DBDWindowActor.h"
#include "DBDHookActor.h"
#include "DBDKiller.generated.h"

class UInputMappingContext;
class UInputAction;
class UCameraComponent;
class ADBDSurvivor;

UCLASS()
class DBDCOPY_API ADBDKiller : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ADBDKiller();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void NotifyControllerChanged() override;

	// Killer state flags
	bool bCanCharacterChange = false;
	bool bCanBreakPallet = false;
	bool bIsBreakingPallet = false;
	bool bCanBreakGenerator = false;
	bool bIsBreakingGenerator = false;
	bool bCanAttack = true;
	bool bIsAttacking = false;
	bool bCanVault = false;
	bool bIsVaulting = false;
	UPROPERTY(ReplicatedUsing = OnRep_CanPickUp)
	bool bCanPickUp = false;
	bool bIsPickingUp = false;
	UPROPERTY(ReplicatedUsing = OnRep_IsCarrying)
	bool bIsCarrying = false;
	bool bCanHook = false;
	bool bIsHooking = false;

	// To change character
	void BeginOverlapCharacterChange();
	void EndOverlapCharacterChange();

	// To vault window
	void BeginOverlapVault();
	void EndOverlapVault();
	void SetCurrentWindow(ADBDWindowActor* WindowActor);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// Cached player controller
	ADBDPlayerController* PC;

	// Components
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
	UCameraComponent* Camera;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
	USpotLightComponent* RedStain;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
	UBoxComponent* FindingSurvivorBox;

	// Animations
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	UAnimMontage* BreakAnim;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	UAnimMontage* AttackAnim;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	UAnimMontage* CarryingAttackAnim;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	UAnimMontage* VaultAnim;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	UAnimMontage* PickUpAnim;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	UAnimMontage* HookAnim;

	// Killer speed values
	float WalkSpeed = 460.0f;

private:
	// Input action
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"));
	UInputMappingContext* DefaultMappingContext;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"));
	UInputAction* MoveInput;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"));
	UInputAction* LookInput;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"));
	UInputAction* InteractInput;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"));
	UInputAction* ActionInput;

	// Killer action function
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void Interact(const FInputActionValue& Value);
	void Action(const FInputActionValue& Value);

	void FindAction();

	// Break pallet
	void BreakPallet();
	void EndBreakPallet();
	UFUNCTION(Server, Reliable)
	void Server_SetCurrentPallet(ADBDPalletActor* TargetPallet);
	UFUNCTION(Server, Reliable)
	void Server_BreakPallet();
	UFUNCTION(Server, Reliable)
	void Server_EndbreakPallet();
	UFUNCTION(NetMulticast, Reliable)
	void MultiCast_BreakPallet();
	UFUNCTION(NetMulticast, Reliable)
	void MultiCast_EndBreakPallet();

	// Break generator
	void BreakGenerator();
	void EndBreakGenerator();
	UFUNCTION(Server, Reliable)
	void Server_SetCurrenetGenerator(ADBDGeneratorActor* TargetGenerator);
	UFUNCTION(Server, Reliable)
	void Server_BreakGenerator();
	UFUNCTION(Server, Reliable)
	void Server_EndBreakGenerator();
	UFUNCTION(NetMulticast, Reliable)
	void MultiCast_BreakGenerator();
	UFUNCTION(NetMulticast, Reliable)
	void MultiCast_EndBreakGenerator();

	// Attack 
	void Attack();
	void EndAttack();
	void TryAttack();
	UFUNCTION(Server, Reliable)
	void Server_Attack();
	UFUNCTION(NetMulticast, Reliable)
	void MultiCast_Attack();
	UFUNCTION(Server, Reliable)
	void Server_TryAttack(FVector FireStart, FVector FireEnd);
	UFUNCTION(Server,Reliable)
	void Server_HandleAttackDelay(bool bIsSuccess);
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_HandleAttackDelay(bool bIsSuccess);

	// Vault
	void Vault();
	void EndVault();
	void EndVaultAnim();
	UFUNCTION(Server, Reliable)
	void Server_Vault();
	UFUNCTION(Server, Reliable)
	void Server_EndVault();
	UFUNCTION(Server, Reliable)
	void Server_EndVaultAnim();
	UFUNCTION(NetMulticast, Reliable)
	void MultiCast_Vault();
	UFUNCTION(NetMulticast, Reliable)
	void MultiCast_EndVault();
	UFUNCTION(NetMulticast, Reliable)
	void MultiCast_EndVaultAnim();

	// Pick up survivor
	UFUNCTION()
	void OnSurvivorOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, 
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
		const FHitResult& SweepResult);
	UFUNCTION()
	void OnSurvivorOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	UFUNCTION()
	void OnRep_CanPickUp();
	UFUNCTION(Server, Reliable)
	void SetTargetSurvivor(ADBDSurvivor* NewSurvivor);
	void HandleTargetSurvivor();
	void TryPickUp();
	void StartPickUp();
	void StopPickUp();
	void StartCarryingSurvivor();
	void StopCarryingSurvivor();
	UFUNCTION()
	void OnRep_IsCarrying();
	UFUNCTION(Server, Reliable)
	void Server_StartPickUp();
	UFUNCTION(NetMulticast, Reliable)
	void MultiCast_StartPickUp();
	void TryDropDown();
	UFUNCTION(Server, Reliable)
	void Server_StartDropDown();

	// Hook survivor
	void StartHookSurvivor();
	void StopHookSurvivor();
	void HookingSurvivor();
	UFUNCTION(Server, Reliable)
	void SetCurrentHook(ADBDHookActor* NewHook);
	UFUNCTION(Server, Reliable)
	void Server_StartHookSurvivor();
	UFUNCTION(NetMulticast, Reliable)
	void MultiCast_StartHookSurvivor();

	// Replicated Actors
	UPROPERTY(Replicated)
	ADBDPalletActor* CurrentPallet;
	UPROPERTY(Replicated)
	ADBDGeneratorActor* CurrentGenerator;
	ADBDWindowActor* CurrentWindow;
	UPROPERTY(Replicated)
	ADBDSurvivor* CurrentTargetSurvivor;
	UPROPERTY(Replicated)
	ADBDHookActor* CurrentHook;

	// TimerHandles
	FTimerHandle BreakTimerHandle;
	FTimerHandle AttackTimerHandle;
	FTimerHandle VaultTimerHandle;
	FTimerHandle PickUpTimerHandle;
	FTimerHandle CarryingTimerHandle;
	FTimerHandle HookingTimerHandle;
	FTimerHandle HookedTImerHandle;
};
