// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Entity/PX_Character.h"
#include "PX_CharacterClient.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_X_API APX_CharacterClient : public APX_Character
{
	GENERATED_BODY()
	
protected:
	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for look input */
	void Look(const FInputActionValue& Value);

public:
	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	
};
