// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "PX_CharacterAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_X_API UPX_CharacterAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
	
public:
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

    UFUNCTION(BlueprintCallable, Category = "Aim")
	void SetIsAiming(bool Value);

	UFUNCTION(BlueprintCallable, Category = "Aim")
	void SetAimProgress(float Value);

	UFUNCTION(BlueprintCallable, Category = "Aim")
	void SetIsDrawing(bool Value);

	UFUNCTION(BlueprintCallable, Category = "Aim")
	void SetDrawProgress(float Value);
	
private:
	/** Character's Velocity **/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character", meta = (AllowPrivateAccess = "true"))
	FVector Velocity = FVector::ZeroVector;

	/** Is Character Aiming **/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bow", meta = (AllowPrivateAccess = "true"))
	bool bIsAiming = false;

	// 활을 들어올린 정도
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bow", meta = (AllowPrivateAccess = "true"))
	float AimProgress = 0.f;

	/** Is Character Drawing **/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bow", meta = (AllowPrivateAccess = "true"))
	bool bIsDrawing = false;

	// 활시위를 당긴 정도
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bow", meta = (AllowPrivateAccess = "true"))
	float DrawProgress = 0.f;

public:
	FORCEINLINE const bool GetIsAiming() { return bIsAiming; }
	FORCEINLINE const float GetAimProgress() { return AimProgress; }
	FORCEINLINE const bool GetIsDrawing() { return bIsDrawing; }
	FORCEINLINE const float GetDrawProgress() { return DrawProgress; }
};
