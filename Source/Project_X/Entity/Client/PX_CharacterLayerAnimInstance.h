// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "PX_CharacterLayerAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_X_API UPX_CharacterLayerAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "PX|Layer")
	void SetHasInput(bool Value)
	{
		bHasInput = Value;
	}

	UFUNCTION(BlueprintCallable, Category = "PX|Layer")
	void SetVelocity(FVector InVector, float Forward, float Right)
	{
		Velocity = InVector;
		LastForwardVelocity = Forward;
		LastRightVelocity = Right;
	}

	UFUNCTION(BlueprintCallable, Category = "PX|Layer")
	void SetIsJumping(bool Value)
	{
		bIsJumping = Value;
	}

	UFUNCTION(BlueprintCallable, Category = "PX|Layer")
	void SetIsFalling(bool Value)
	{
		bIsFalling = Value;
	}

	UFUNCTION(BlueprintCallable, Category = "PX|Layer")
	void SetIsCrouching(bool Value)
	{
		bIsCrouching = Value;
	}

	UFUNCTION(BlueprintCallable, Category = "PX|Layer")
	void SetRotation(float InYaw, float InPitch )
	{
		AimYaw = InYaw;
		AimPitch = InPitch;
	}

	UFUNCTION(BlueprintCallable, Category = "PX|Layer")
	void SetIsAiming(bool Value)
	{
		//UE_LOG(LogTemp, Log, TEXT("PX_CharacterLayerAnimInstance::SetIsAiming %s -> %s"), bIsAiming ? TEXT("true") : TEXT("false"), Value ? TEXT("true") : TEXT("false"));
		bIsAiming = Value;
	}

	UFUNCTION(BlueprintCallable, Category = "PX|Layer")
	void SetAimProgress(float Value)
	{
		AimProgress = FMath::Clamp(Value, 0.f, 1.f);
	}

	UFUNCTION(BlueprintCallable, Category = "PX|Layer")
	void SetIsDrawing(bool Value)
	{
		bIsDrawing = Value;
	}
	
	UFUNCTION(BlueprintCallable, Category = "PX|Layer")
	void SetDrawProgress(float Value)
	{
		DrawProgress = FMath::Clamp(Value, 0.f, 1.f);
	}

	UFUNCTION(BlueprintCallable, Category = "PX|Layer")
	void SetIsReloading(bool Value)
	{
		bIsReloading = Value;
	}

private:
	// --- Character -----------------------------------------------------
	/** Character's Velocity **/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character", meta = (AllowPrivateAccess = "true"))
	FVector Velocity = FVector::ZeroVector;
	/** Is Character Have Input **/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character", meta = (AllowPrivateAccess = "true"))
	bool bHasInput = false;
	/** Character's Last Forward Velocity **/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character", meta = (AllowPrivateAccess = "true"))
	float LastForwardVelocity = 0.f;
	/** Character's Last Right Velocity **/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character", meta = (AllowPrivateAccess = "true"))
	float LastRightVelocity = 0.f;
	/** Is Character Jumping **/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character", meta = (AllowPrivateAccess = "true"))
	bool bIsJumping = false;
	/** Is Character Falling **/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character", meta = (AllowPrivateAccess = "true"))
	bool bIsFalling = false;
	/** Is Character Crouching **/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character", meta = (AllowPrivateAccess = "true"))
	bool bIsCrouching = false;

	// --- Aiming -----------------------------------------------------
	/** Character's Aiming Yaw **/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bow", meta = (AllowPrivateAccess = "true"))
	float AimYaw = 0.f;

	/** Character's Aiming Pitch **/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bow", meta = (AllowPrivateAccess = "true"))
	float AimPitch = 0.f;

	/** Is Character Aiming **/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bow", meta = (AllowPrivateAccess = "true"))
	bool bIsAiming = false;

	// 활을 들어올린 정도
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bow", meta = (AllowPrivateAccess = "true"))
	float AimProgress = 0.f;

	// --- Drawing -----------------------------------------------------
	/** Is Character Drawing **/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bow", meta = (AllowPrivateAccess = "true"))
	bool bIsDrawing = false;

	// 활시위를 당긴 정도
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bow", meta = (AllowPrivateAccess = "true"))
	float DrawProgress = 0.f;

	/** Is Character Reloading **/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bow", meta = (AllowPrivateAccess = "true"))
	bool bIsReloading = false;
	
	
};
