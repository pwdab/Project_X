// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Project_X.h"
#include "AbilitySystem/Abilities/PX_GameplayAbilityBase.h"
#include "PX_GameplayAbility_GunReload.generated.h"

class APX_Character;
class UPX_WeaponSystemComponent;
class UAbilityTask_WaitGameplayEvent;

UCLASS()
class PROJECT_X_API UPX_GameplayAbility_GunReload : public UPX_GameplayAbilityBase
{
    GENERATED_BODY()

public:
    UPX_GameplayAbility_GunReload();

protected:
    virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
    virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
    virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
    UFUNCTION()
    void Predict_OnReloadEnd(FGameplayEventData Payload);
    UFUNCTION()
    void Authoritative_OnReloadEnd();

    void CancelReload(const FGameplayAbilityActorInfo* ActorInfo);

    /*
    APX_Character* GetPXCharacter(const FGameplayAbilityActorInfo* ActorInfo) const;
    UPX_WeaponSystemComponent* GetWeaponSystemComponent(const FGameplayAbilityActorInfo* ActorInfo) const;
    */

protected:
    UPROPERTY(Transient)
    UAbilityTask_WaitGameplayEvent* PredictWaitTask;

    FTimerHandle ServerReloadTimerHandle;
};
