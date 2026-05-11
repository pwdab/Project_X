// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/PX_GA_SwitchAttackMode.h"

#include "AbilitySystem/Tags/PX_GameplayTags.h"
#include "Component/Weapon/PX_WeaponSystemComponent.h"

UPX_GA_SwitchAttackMode::UPX_GA_SwitchAttackMode()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
    NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

    AbilityTags.AddTag(PX_GameplayTags::Ability_Weapon_SwitchAttackMode);
    InputTag = PX_GameplayTags::Input_Combat_SwitchAttackMode;

    ActivationOwnedTags.AddTag(PX_GameplayTags::State_Combat_SwitchAttackMode);

    ActivationBlockedTags.AddTag(PX_GameplayTags::State_Combat_Equipping);
    ActivationBlockedTags.AddTag(PX_GameplayTags::State_Combat_Unequipping);
    ActivationBlockedTags.AddTag(PX_GameplayTags::State_Combat_SwappingWeapon);
    ActivationBlockedTags.AddTag(PX_GameplayTags::State_Combat_Reloading);
    ActivationBlockedTags.AddTag(PX_GameplayTags::State_Combat_SwitchAttackMode);

    ActivationBlockedTags.AddTag(PX_GameplayTags::State_Condition_Dead);
    ActivationBlockedTags.AddTag(PX_GameplayTags::State_Condition_Down);
    ActivationBlockedTags.AddTag(PX_GameplayTags::State_Condition_Disabled);
    ActivationBlockedTags.AddTag(PX_GameplayTags::State_Condition_Stunned);
    ActivationBlockedTags.AddTag(PX_GameplayTags::State_Condition_Silenced);
}

bool UPX_GA_SwitchAttackMode::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
    if ( !Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags) ) return false;

    PX_LOG(Log, TEXT(""));

    /*
    UPX_WeaponSystemComponent* WeaponSystemComponent = GetWeaponSystemComponent(ActorInfo);
    if ( !WeaponSystemComponent )
    {
        PX_LOG(Warning, TEXT("Invalid Weapon System Component"));
        return false;
    }

    UPX_WeaponItemInstance* WeaponItemInstance = WeaponSystemComponent->GetWeaponInstance();
    if ( !WeaponItemInstance )
    {
        PX_LOG(Warning, TEXT("Invalid WeaponItemInstance"));
        return false;
    }

    UPX_WeaponDataAsset* WeaponDataAsset = WeaponItemInstance->GetWeaponDataAsset();
    if ( !WeaponDataAsset )
    {
        PX_LOG(Warning, TEXT("Invalid Weapon Data"));
        return false;
    }
    */

    return true;
}

void UPX_GA_SwitchAttackMode::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

    PX_LOG(Log, TEXT(""));

    if ( !HasAuthority(&ActivationInfo) && ActorInfo && ActorInfo->IsLocallyControlled() && !ShouldUseClientPrediction(ActorInfo, &ActivationInfo) )
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
        return;
    }

    // Commit Ability
    if ( !CommitAbility(Handle, ActorInfo, ActivationInfo) )
    {
        PX_LOG(Warning, TEXT("CommitAbility Failed"));
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    UPX_WeaponSystemComponent* WeaponSystemComponent = GetWeaponSystemComponent(CurrentActorInfo);
    if ( !WeaponSystemComponent )
    {
        PX_LOG(Warning, TEXT("Invalid Weapon System Component"));
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    UPX_AbilitySystemComponent* PX_ASC = Cast<UPX_AbilitySystemComponent>(GetAbilitySystemComponent(ActorInfo));
    if ( !PX_ASC )
    {
        PX_LOG(Warning, TEXT("PX_ASC is Invalid."));
    }

    // 공격 모드 변경 시 기존 무기의 액션이 남아있을 수 있으므로 관련 액션 태그를 가진 능력들을 취소
    FGameplayTagContainer CancelTags;
    CancelTags.AddTag(PX_GameplayTags::Ability_Weapon_Attack);
    PX_ASC->CancelAbilities(&CancelTags, nullptr, this);

    // Switch Attack Mode 시도
    if ( HasAuthority(&ActivationInfo) )
    {
        // Server Authoritative
        PX_LOG(Log, TEXT("Begin Server SwitchAttackMode"));

        if ( !WeaponSystemComponent->Authoritative_SwitchAttackMode() )
        {
            PX_LOG(Warning, TEXT("Fail Server SwitchAttackMode"));
            EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
            return;
        }
        WeaponSystemComponent->Multicast_SwitchAttackMode();

        Authoritative_OnSwitchAttackModeEnd();
    }
    else if ( ShouldUseClientPrediction(ActorInfo, &ActivationInfo) )
    {
        // Local Prediction
        PX_LOG(Log, TEXT("Begin Client Predicted SwitchAttackMode"));

        if ( !WeaponSystemComponent->Predict_SwitchAttackMode() )
        {
            PX_LOG(Warning, TEXT("Fail Client Predicted SwitchAttackMode"));
            EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
            return;
        }

        Predict_OnSwitchAttackModeEnd();
    }
}

void UPX_GA_SwitchAttackMode::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
    if ( !IsActive() )
    {
        PX_LOG(Log, TEXT("Ability is InActive"));
        return;
    }

    if ( bWasCancelled ) CancelSwitchAttackMode(ActorInfo);

    PX_LOG(Log, TEXT(""));

    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UPX_GA_SwitchAttackMode::Predict_OnSwitchAttackModeEnd()
{
    PX_LOG(Log, TEXT(""));

    if ( !CurrentActorInfo )
    {
        PX_LOG(Warning, TEXT("Invalid ActorInfo"));
        return;
    }

    UPX_WeaponSystemComponent* WSC = GetWeaponSystemComponent(CurrentActorInfo);
    if ( !WSC )
    {
        PX_LOG(Warning, TEXT("Invalid Weapon System Component"));
        return;
    }

    if ( !ShouldUseClientPrediction(CurrentActorInfo, &CurrentActivationInfo) ) return;

    if ( !WSC->Predict_SwitchAttackModeEnd() )
    {
        CancelSwitchAttackMode(CurrentActorInfo);
        return;
    }

    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, false);
}

void UPX_GA_SwitchAttackMode::Authoritative_OnSwitchAttackModeEnd()
{
    PX_LOG(Log, TEXT(""));

    if ( !CurrentActorInfo )
    {
        PX_LOG(Warning, TEXT("Invalid ActorInfo"));
        return;
    }

    UPX_WeaponSystemComponent* WSC = GetWeaponSystemComponent(CurrentActorInfo);
    if ( !WSC )
    {
        PX_LOG(Warning, TEXT("Invalid Weapon System Component"));
        return;
    }

    if ( !HasAuthority(&CurrentActivationInfo) ) return;

    if ( !WSC->Authoritative_SwitchAttackModeEnd() )
    {
        CancelSwitchAttackMode(CurrentActorInfo);
        return;
    }

    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UPX_GA_SwitchAttackMode::CancelSwitchAttackMode(const FGameplayAbilityActorInfo* ActorInfo)
{
    PX_LOG(Log, TEXT(""));

    UPX_WeaponSystemComponent* WSC = GetWeaponSystemComponent(ActorInfo);
    if ( !WSC ) return;

    if ( HasAuthority(&CurrentActivationInfo) )
    {
        // Server Authoritative
        PX_LOG(Log, TEXT("Cancel Server SwitchAttackMode"));

        WSC->Authoritative_CancelSwitchAttackMode();
    }
    else if ( ShouldUseClientPrediction(ActorInfo, &CurrentActivationInfo) )
    {
        // Local Prediction
        PX_LOG(Log, TEXT("Cancel Client Predicted SwitchAttackMode"));

        WSC->Predict_CancelSwitchAttackMode();
    }
}
