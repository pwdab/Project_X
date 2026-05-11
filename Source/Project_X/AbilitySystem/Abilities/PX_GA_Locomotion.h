// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Project_X.h"
#include "AbilitySystem/Abilities/PX_GameplayAbilityBase.h"
#include "TimerManager.h"
#include "PX_GA_Locomotion.generated.h"

class APX_Character;
class UCharacterMovementComponent;

UCLASS(Abstract)
class PROJECT_X_API UPX_GA_LocomotionBase : public UPX_GameplayAbilityBase
{
	GENERATED_BODY()

public:
	UPX_GA_LocomotionBase();

protected:
	virtual bool ShouldUseClientPrediction(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo* ActivationInfo = nullptr) const override;
	bool HasEnoughStamina(const FGameplayAbilityActorInfo* ActorInfo, float Cost) const;
	bool ApplyStaminaCost(const FGameplayAbilityActorInfo* ActorInfo, float Cost) const;
	bool ApplyStaminaCostAllowPartial(const FGameplayAbilityActorInfo* ActorInfo, float Cost) const;
	void ApplyPredictedStaminaCostToHUD(const FGameplayAbilityActorInfo* ActorInfo, float Cost) const;
	void RefreshLocomotionSpeed(const FGameplayAbilityActorInfo* ActorInfo) const;
	APX_Character* GetCharacterChecked(const FGameplayAbilityActorInfo* ActorInfo) const;
	UCharacterMovementComponent* GetMovementComponent(const FGameplayAbilityActorInfo* ActorInfo) const;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PX|Prediction")
	bool bUseClientPredictedStaminaUI = true;
};

UCLASS()
class PROJECT_X_API UPX_GA_Jump : public UPX_GA_LocomotionBase
{
	GENERATED_BODY()

public:
	UPX_GA_Jump();

protected:
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

private:
	UPROPERTY(EditDefaultsOnly, Category = "PX|Locomotion")
	float JumpStaminaCost = 15.0f;
};

UCLASS()
class PROJECT_X_API UPX_GA_Walk : public UPX_GA_LocomotionBase
{
	GENERATED_BODY()

public:
	UPX_GA_Walk();

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
};

UCLASS()
class PROJECT_X_API UPX_GA_Sprint : public UPX_GA_LocomotionBase
{
	GENERATED_BODY()

public:
	UPX_GA_Sprint();

protected:
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

private:
	void DrainSprintStamina();

	UPROPERTY(EditDefaultsOnly, Category = "PX|Locomotion")
	float SprintStaminaCostPerSecond = 12.0f;

	UPROPERTY(EditDefaultsOnly, Category = "PX|Locomotion")
	float SprintStaminaTickInterval = 0.033f;

	FTimerHandle SprintStaminaTimerHandle;
	double LastSprintStaminaDrainTime = 0.0;
};

UCLASS()
class PROJECT_X_API UPX_GA_Crouch : public UPX_GA_LocomotionBase
{
	GENERATED_BODY()

public:
	UPX_GA_Crouch();

protected:
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
};
