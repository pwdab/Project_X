// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/PX_GA_Aim.h"

#include "AbilitySystem/Component/PX_AbilitySystemComponent.h"
#include "AbilitySystem/Tags/PX_GameplayTags.h"
#include "Entity/PX_Character.h"
#include "Entity/Client/PX_CharacterAnimInstance.h"

UPX_GA_Aim::UPX_GA_Aim()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
    NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

    // InputTag는 GiveAbility()에서 설정하도록 변경

    // Ability 자체 태그
    AbilityTags.AddTag(PX_GameplayTags::Ability_Weapon_Aim);

    // Ability 활성 중 ASC에 부여될 태그
    ActivationOwnedTags.AddTag(PX_GameplayTags::State_Combat_Aiming);

    // 이 태그가 있으면 Ability 활성화 불가
    ActivationBlockedTags.AddTag(PX_GameplayTags::State_Combat_Equipping);
    ActivationBlockedTags.AddTag(PX_GameplayTags::State_Combat_Unequipping);
    ActivationBlockedTags.AddTag(PX_GameplayTags::State_Combat_SwappingWeapon);
    ActivationBlockedTags.AddTag(PX_GameplayTags::State_Combat_SwitchAttackMode);
    ActivationBlockedTags.AddTag(PX_GameplayTags::State_Combat_Aiming);

    ActivationBlockedTags.AddTag(PX_GameplayTags::State_Condition_Dead);
    ActivationBlockedTags.AddTag(PX_GameplayTags::State_Condition_Down);
    ActivationBlockedTags.AddTag(PX_GameplayTags::State_Condition_Disabled);
    ActivationBlockedTags.AddTag(PX_GameplayTags::State_Condition_Stunned);
    ActivationBlockedTags.AddTag(PX_GameplayTags::State_Condition_Silenced);
}

bool UPX_GA_Aim::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
    if ( !Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags) ) return false;

    UAbilitySystemComponent* ASC = GetAbilitySystemComponent(ActorInfo);
    if ( !ASC )
    {
        PX_LOG(Warning, TEXT("Invalid ASC"));
        return false;
    }

    UPX_WeaponSystemComponent* WSC = GetWeaponSystemComponent(ActorInfo);
    if ( !WSC )
    {
        PX_LOG(Warning, TEXT("Invalid Weapon System Component"));
        return false;
    }

    return true;
}

void UPX_GA_Aim::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

    PX_LOG(Log, TEXT(""));

    if ( !HasAuthority(&ActivationInfo) && ActorInfo && ActorInfo->IsLocallyControlled() && !ShouldUseClientPrediction(ActorInfo, &ActivationInfo) )
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
        return;
    }

    if ( !CommitAbility(Handle, ActorInfo, ActivationInfo) )
    {
        PX_LOG(Warning, TEXT("CommitAbility Failed"));
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
    if ( !ASC )
    {
        PX_LOG(Warning, TEXT("Invalid ASC"));
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    UPX_AbilitySystemComponent* PX_ASC = Cast<UPX_AbilitySystemComponent>(ASC);
    if ( !PX_ASC )
    {
        PX_LOG(Warning, TEXT("Invalid PX_ASC"));
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    AimBitState.Clear();

    UpdateAimState();

    if ( !AimBitState.IsAiming() )
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
        return;
    }
}

void UPX_GA_Aim::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
    UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
    if ( !ASC )
    {
        PX_LOG(Warning, TEXT("Invalid ASC"));
        return;
    }

    ClearAimStateTags();
    AimBitState.Clear();

    APX_Character* PX_Character = Cast<APX_Character>(GetAvatarActorFromActorInfo());
    if ( PX_Character )
    {
        PX_Character->ApplyAimCameraMode(false, false);
        ApplyAiming(false);
    }

    if ( !IsActive() )
    {
        PX_LOG(Log, TEXT("Ability is InActive"));
        return;
    }

    PX_LOG(Log, TEXT(""));

    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UPX_GA_Aim::InputPressed(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
    Super::InputPressed(Handle, ActorInfo, ActivationInfo);

    PX_LOG(Log, TEXT(""));

    UpdateAimState();
}

void UPX_GA_Aim::InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
    Super::InputReleased(Handle, ActorInfo, ActivationInfo);

    PX_LOG(Log, TEXT(""));

    UpdateAimState();

    if ( !AimBitState.IsAiming() )
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
    }
}

void UPX_GA_Aim::UpdateAimState()
{
    UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
    if ( !ASC )
    {
        PX_LOG(Warning, TEXT("Invalid ASC"));
        return;
    }

    UPX_AbilitySystemComponent* PX_ASC = Cast<UPX_AbilitySystemComponent>(ASC);
    if ( !PX_ASC )
    {
        PX_LOG(Warning, TEXT("Invalid PX_ASC"));
        return;
    }

    const FGameplayTag ChangedInputTag = PX_ASC->GetProcessingInputTag();
    const bool bPressed = PX_ASC->IsProcessingInputTagPressed();

    AimBitState.SavePrevious();

    if ( ChangedInputTag == PX_GameplayTags::Input_Combat_Aim && bPressed && ASC->HasMatchingGameplayTag(PX_GameplayTags::State_Combat_Reloading) )
    {
        return;
    }

    if ( ChangedInputTag == PX_GameplayTags::Input_Combat_Aim_ADS )
    {
        if ( bPressed )
        {
            AimBitState.AddState(EAimState::ADS);
        }
        else
        {
            AimBitState.RemoveState(EAimState::ADS);
        }
    }
    else if ( ChangedInputTag == PX_GameplayTags::Input_Combat_Aim_OTS )
    {
        if ( bPressed )
        {
            AimBitState.AddState(EAimState::OTS);
        }
        else
        {
            AimBitState.RemoveState(EAimState::OTS);
        }
    }
    else if ( ChangedInputTag == PX_GameplayTags::Input_Combat_Aim )
    {
        if ( bPressed )
        {
            AimBitState.AddState(EAimState::HipFire);
        }
        else
        {
            AimBitState.RemoveState(EAimState::HipFire);
        }
    }

    ClearAimStateTags();

    if ( AimBitState.IsADS() )
    {
        ASC->SetLooseGameplayTagCount(PX_GameplayTags::State_Combat_Aiming_ADS, 1);
    }
    else if ( AimBitState.IsOTS() )
    {
        ASC->SetLooseGameplayTagCount(PX_GameplayTags::State_Combat_Aiming_OTS, 1);
    }
    else if ( AimBitState.IsHipFire() )
    {
        ASC->SetLooseGameplayTagCount(PX_GameplayTags::State_Combat_Aiming_HipFire, 1);
    }

    if ( AimBitState.ShouldBeginAim() )
    {
        ApplyAiming(true);
    }
    else if ( AimBitState.ShouldEndAim() )
    {
        ApplyAiming(false);
    }

    ApplyAimCameraMode();
}

void UPX_GA_Aim::ClearAimStateTags()
{
    UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
    if ( !ASC )
    {
        PX_LOG(Warning, TEXT("Invalid ASC"));
        return;
    }

    ASC->SetLooseGameplayTagCount(PX_GameplayTags::State_Combat_Aiming_HipFire, 0);
    ASC->SetLooseGameplayTagCount(PX_GameplayTags::State_Combat_Aiming_ADS, 0);
    ASC->SetLooseGameplayTagCount(PX_GameplayTags::State_Combat_Aiming_OTS, 0);
}

void UPX_GA_Aim::ApplyAiming(bool bNewIsAiming)
{
    if ( bAppliedIsAiming == bNewIsAiming )
    {
        return;
    }

    APX_Character* PX_Character = Cast<APX_Character>(GetAvatarActorFromActorInfo());
    if ( !PX_Character )
    {
        return;
    }

    bAppliedIsAiming = bNewIsAiming;
    PX_Character->SetIsAiming(bNewIsAiming);

    if ( UPX_CharacterAnimInstance* AnimInstance = PX_Character->GetAnimInstance() )
    {
        AnimInstance->SetIsAiming(bNewIsAiming);
    }
}

void UPX_GA_Aim::ApplyAimCameraMode()
{
    APX_Character* PX_Character = Cast<APX_Character>(GetAvatarActorFromActorInfo());
    if ( !PX_Character )
    {
        return;
    }

    PX_Character->ApplyAimCameraMode(AimBitState.IsADS(), AimBitState.IsOTS());
}

void UPX_GA_Aim::EndHipFireForReload()
{
    if ( !IsActive() )
    {
        return;
    }

    AimBitState.SavePrevious();
    AimBitState.RemoveState(EAimState::HipFire);

    UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
    if ( ASC )
    {
        ASC->SetLooseGameplayTagCount(PX_GameplayTags::State_Combat_Aiming_HipFire, 0);
    }

    if ( !AimBitState.IsAiming() )
    {
        EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
        return;
    }

    ClearAimStateTags();

    if ( ASC )
    {
        if ( AimBitState.IsADS() )
        {
            ASC->SetLooseGameplayTagCount(PX_GameplayTags::State_Combat_Aiming_ADS, 1);
        }
        else if ( AimBitState.IsOTS() )
        {
            ASC->SetLooseGameplayTagCount(PX_GameplayTags::State_Combat_Aiming_OTS, 1);
        }
    }

    ApplyAimCameraMode();
}
