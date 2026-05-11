// Fill out your copyright notice in the Description page of Project Settings.

#pragma once


#include "Project_X.h"
#include "AbilitySystem/Abilities/PX_GameplayAbilityBase.h"
#include "Abilities/GameplayAbilityTargetTypes.h"
#include "GameplayTagContainer.h"
#include "PX_GameplayAbility_GunFire.generated.h"

class UAbilitySystemComponent;
class UAnimMontage;
class UPX_WeaponDataAsset;
class UPX_WeaponItemInstance;
class UPX_WeaponSystemComponent;
class APX_Character;
class APX_Weapon;

UCLASS()
class PROJECT_X_API UPX_GameplayAbility_GunFire : public UPX_GameplayAbilityBase
{
    GENERATED_BODY()

public:
    UPX_GameplayAbility_GunFire();

    virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
    virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
    virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
    virtual void InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) override;

private:
    void FireOnce(const FGameplayAbilityTargetDataHandle* ServerFireTargetData = nullptr);

    UFUNCTION()
    void Predict_OnFireOnceEnd();
    UFUNCTION()
    void Authoritative_OnFireOnceEnd();
    void OnServerFireTargetDataReady(const FGameplayAbilityTargetDataHandle& Data, FGameplayTag ActivationTag);
    void WaitForServerFireTargetData();
    void StopWaitingForServerFireTargetData();
    void SendPredictedFireTargetData(UPX_WeaponSystemComponent* WeaponSystemComponent);
    void CancelFire(const FGameplayAbilityActorInfo* ActorInfo);
    
    void StopFireInternal();
    void FinishIfReleaseAlreadyHappened();

    /*
    APX_Character* GetPXCharacter(const FGameplayAbilityActorInfo* ActorInfo) const;
    UPX_WeaponSystemComponent* GetWeaponSystemComponent(const FGameplayAbilityActorInfo* ActorInfo) const;
    UAbilitySystemComponent* GetASC(const FGameplayAbilityActorInfo* ActorInfo) const;
    */

private:
    bool ShouldFireAgain();
    
    FTimerHandle ServerFireTimerHandle;
    FTimerHandle PredictFireTimerHandle;

    UPROPERTY(Transient)
    int32 ServerBurstRemaining = 0;
    UPROPERTY(Transient)
    int32 PredictBurstRemaining = 0;

    // 연사 혹은 점사 모드에서 잔탄이 0이 되면 DryFire를 발동해야 함
    UPROPERTY(Transient)
    bool bPendingDryFire = false;
    UPROPERTY(Transient)
    bool bWaitingForServerFireTargetData = false;
    UPROPERTY(Transient)
    bool bStopFireRequested = false;
};
