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
#include "DBDPlayerController.h"
#include "DBDPalletActor.h"
#include "DBDGeneratorActor.h"
#include "DBDKiller.generated.h"

class UInputMappingContext;
class UInputAction;
class UCameraComponent;

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

	// To change character
	void BeginOverlapCharacterChange();
	void EndOverlapCharacterChange();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Components
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
	UCameraComponent* Camera;

	// Animations
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	UAnimMontage* BreakAnim;

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

	void FindBreakable();
	void BreakPallet();
	void EndBreakPallet();
	void BreakGenerator();
	void EndBreakGenerator();

	ADBDPalletActor* CurrentPallet;
	ADBDGeneratorActor* CurrentGenerator;
	FTimerHandle BreakTimerHandle;
};
