// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Entity/PX_Character.h"
#include "PX_CharacterAnimInstance.h"
#include "Components/TimelineComponent.h"
#include "PX_CharacterClient.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_X_API APX_CharacterClient : public APX_Character
{
	GENERATED_BODY()

public:
	APX_CharacterClient();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for look input */
	void Look(const FInputActionValue& Value);

	/** Called for aim input */
	void BeginAim(const FInputActionValue& Value);
	void EndAim(const FInputActionValue& Value);

	/** Called for draw input */
	void BeginDraw(const FInputActionInstance& Instance);
	void EndDraw(const FInputActionInstance& Instance);

public:
	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

private:
	UPX_CharacterAnimInstance* PX_CharacterAnimInstance = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Aim, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UTimelineComponent> AimProgressTimeline;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Aim, meta = (AllowPrivateAccess = "true"))
	UCurveFloat* AimProgressCurve;

	UFUNCTION()
	void AimProgressUpdate(float Alpha);
};
