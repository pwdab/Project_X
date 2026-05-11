// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Project_X.h"
#include "GameFramework/PlayerState.h"
#include "AbilitySystemInterface.h"
#include "PX_PlayerState.generated.h"

class UAbilitySystemComponent;
class UPX_AbilitySystemComponent;
class UPX_ResourceAttributeSet;
class UPX_CombatAttributeSet;
class UPX_MovementAttributeSet;
class UPX_AbilitiesDataAsset;

/**
 * 
 */
UCLASS()
class PROJECT_X_API APX_PlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()
	
public:
	APX_PlayerState();

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	void GrantDefaultAbilities();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	TObjectPtr<UPX_AbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	TObjectPtr<UPX_ResourceAttributeSet> ResourceAttributeSet;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	TObjectPtr<UPX_CombatAttributeSet> CombatAttributeSet;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	TObjectPtr<UPX_MovementAttributeSet> MovementAttributeSet;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS")
	TObjectPtr<UPX_AbilitiesDataAsset> DefaultAbilitySet;
	
private:
	bool bDefaultAbilitiesGranted = false;
	
};
