// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Project_X.h"
#include "AbilitySystem/Abilities/PX_GameplayAbilityBase.h"
#include "PX_GA_SwitchAttackMode.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_X_API UPX_GA_SwitchAttackMode : public UPX_GameplayAbilityBase
{
	GENERATED_BODY()
	
	UPX_GA_SwitchAttackMode();

	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
	UFUNCTION()
	void Predict_OnSwitchAttackModeEnd();
	UFUNCTION()
	void Authoritative_OnSwitchAttackModeEnd();

	void CancelSwitchAttackMode(const FGameplayAbilityActorInfo* ActorInfo);
	
};
