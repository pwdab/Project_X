// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/PX_GameplayAbility_GunReload.h"

#include "AbilitySystem/Abilities/PX_GA_Aim.h"
#include "AbilitySystem/Tags/PX_GameplayTags.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"

#include "Entity/PX_Character.h"
#include "Component/Weapon/PX_WeaponDataAsset.h"
#include "Component/Weapon/PX_WeaponSystemComponent.h"

#include "Component/Inventory/PX_InventoryComponent.h"
#include "Component/Inventory/PX_WeaponItemInstance.h"

UPX_GameplayAbility_GunReload::UPX_GameplayAbility_GunReload()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
    NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

    // 입력 태그
    InputTag = PX_GameplayTags::Input_Combat_Reload;

    // Ability 자체 태그
    AbilityTags.AddTag(PX_GameplayTags::Ability_Weapon_Reload);

    // Ability 활성 중 ASC에 부여될 태그
    ActivationOwnedTags.AddTag(PX_GameplayTags::State_Combat_Reloading);

    // 이 태그가 있으면 Ability 활성화 불가
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

bool UPX_GameplayAbility_GunReload::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
    if ( !Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags) ) return false;

    const UPX_WeaponSystemComponent* WSC = GetWeaponSystemComponent(ActorInfo);
    if ( !WSC ) { PX_LOG(Log, TEXT("false")); return false; }

    APX_Character* Character = GetPXCharacter(ActorInfo);
    if ( !Character ) { PX_LOG(Log, TEXT("false")); return false; }

    UPX_WeaponItemInstance* WeaponItemInstance = WSC->GetWeaponInstance();
    if ( !WeaponItemInstance )
    {
        PX_LOG(Warning, TEXT("Invalid WeaponItemInstance"));
        return false;
    }

    UPX_WeaponDataAsset* WeaponDataAsset = WeaponItemInstance->GetWeaponDataAsset();
    if ( !WeaponDataAsset )
    {
        PX_LOG(Warning, TEXT("Invalid WeaponDataAsset"));
        return false;
    }

    if ( WSC->GetCurrentAmmoInMag() >= WeaponItemInstance->GetWeaponDataAsset()->MagSize )
    {
        PX_LOG(Warning, TEXT("Ammo is already Full."));
        return false;
    }

    PX_LOG(Log, TEXT("true"));

    return true;
}

void UPX_GameplayAbility_GunReload::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{

    PX_LOG(Log, TEXT(""));

    if ( !HasAuthority(&ActivationInfo) && ActorInfo && ActorInfo->IsLocallyControlled() && !ShouldUseClientPrediction(ActorInfo, &ActivationInfo) )
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
        return;
    }

    UPX_WeaponSystemComponent* WSC = GetWeaponSystemComponent(ActorInfo);
    if (!WSC)
    {
        PX_LOG(Warning, TEXT("Invalid WeaponSystemComponent"));
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    // Reload 몽타주 데이터 가져오기
    UPX_WeaponItemInstance* WeaponItemInstance = WSC->GetWeaponInstance();
    if ( !WeaponItemInstance )
    {
        PX_LOG(Warning, TEXT("Invalid WeaponItemInstance"));
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    UPX_WeaponDataAsset* WeaponDataAsset = WeaponItemInstance->GetWeaponDataAsset();
    if ( !WeaponDataAsset )
    {
        PX_LOG(Warning, TEXT("Invalid WeaponDataAsset"));
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    UAnimMontage* ReloadMontage = WeaponDataAsset->ReloadAction.CharacterMontage;
    if ( !ReloadMontage )
    {
        PX_LOG(Warning, TEXT("Invalid ReloadMontage"));
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    int32 SlotIndex = WSC->GetWeaponSlotIndex();
    if ( SlotIndex == INDEX_NONE )
    {
        PX_LOG(Warning, TEXT("Invalid SlotIndex"));
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    UPX_AbilitySystemComponent* PX_ASC = Cast<UPX_AbilitySystemComponent>(GetAbilitySystemComponent(ActorInfo));
    if ( !PX_ASC )
    {
        PX_LOG(Warning, TEXT("PX_ASC is Invalid."));
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    // 재장전 시 기존 무기의 액션이 남아있을 수 있으므로 관련 액션 태그를 가진 능력들을 취소
    FGameplayTagContainer CancelTags;
    CancelTags.AddTag(PX_GameplayTags::Ability_Weapon_Attack);
    PX_ASC->CancelAbilities(&CancelTags, nullptr, this);

    FGameplayTagContainer AimTags;
    AimTags.AddTag(PX_GameplayTags::Ability_Weapon_Aim);

    TArray<FGameplayAbilitySpec*> AimSpecs;
    PX_ASC->GetActivatableGameplayAbilitySpecsByAllMatchingTags(AimTags, AimSpecs, false);
    for ( FGameplayAbilitySpec* Spec : AimSpecs )
    {
        if ( !Spec || !Spec->IsActive() ) continue;

        for ( UGameplayAbility* Instance : Spec->GetAbilityInstances() )
        {
            if ( UPX_GA_Aim* AimAbility = Cast<UPX_GA_Aim>(Instance) )
            {
                AimAbility->EndHipFireForReload();
            }
        }
    }

    // Reload 시도
    if ( HasAuthority(&ActivationInfo) )
    {
        // Server Authoritative
        PX_LOG(Log, TEXT("Begin Server Reload"));

        if ( !WSC->Authoritative_Reload() )
        {
            PX_LOG(Warning, TEXT("Fail Server Reload"));
            EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
            return;
        }
        WSC->Multicast_Reload();

        const float PlayRate = FMath::Max(WeaponDataAsset->ReloadAction.PlayRate, KINDA_SMALL_NUMBER);
        const float Duration = ReloadMontage->GetPlayLength() / PlayRate;

        PX_LOG(Log, TEXT("Server Reload Set Timer. Duration: %f"), Duration);

        UWorld* World = GetWorld();
        if ( !World )
        {
            PX_LOG(Warning, TEXT("Invalid World"));
            return;
        }
        World->GetTimerManager().SetTimer(ServerReloadTimerHandle, this, &UPX_GameplayAbility_GunReload::Authoritative_OnReloadEnd, Duration, false);

        return;
    }
    else if ( ShouldUseClientPrediction(ActorInfo, &ActivationInfo) )
    {
        // Local Prediction
        PX_LOG(Log, TEXT("Begin Client Predicted Reload"));

        if ( !WSC->Predict_Reload() )
        {
            PX_LOG(Warning, TEXT("Fail Client Predicted Reload"));
            EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
            return;
        }
        WSC->Local_PlayCharacterMontage(WeaponDataAsset->CharacterAnimClass, ReloadMontage, WeaponDataAsset->ReloadAction.PlayRate);
    }

    if ( ShouldUseClientPrediction(ActorInfo, &ActivationInfo) )
    {
        // Local Prediction에서 몽타주 Notify가 Event를 보낼 때까지 대기
        if ( PredictWaitTask )
        {
            PredictWaitTask->ExternalCancel();
            PredictWaitTask = nullptr;
        }

        PredictWaitTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, PX_GameplayTags::Event_Weapon_Reload_End, nullptr, false, false);
        if ( PredictWaitTask )
        {
            PredictWaitTask->EventReceived.AddDynamic(this, &UPX_GameplayAbility_GunReload::Predict_OnReloadEnd);
            PredictWaitTask->ReadyForActivation();
        }
    }
}

void UPX_GameplayAbility_GunReload::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
    if ( bWasCancelled ) CancelReload(ActorInfo);

    PX_LOG(Log, TEXT(""));

    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UPX_GameplayAbility_GunReload::Predict_OnReloadEnd(FGameplayEventData Payload)
{
    PX_LOG(Log, TEXT(""));

    const FGameplayAbilityActorInfo* ActorInfo = GetCurrentActorInfo();
    if ( !ActorInfo )
    {
        PX_LOG(Warning, TEXT("Invalid ActorInfo"));
        return;
    }

    UPX_WeaponSystemComponent* WSC = GetWeaponSystemComponent(ActorInfo);
    if ( !WSC )
    {
        PX_LOG(Warning, TEXT("Invalid Weapon System Component"));
        return;
    }

    if ( !ShouldUseClientPrediction(ActorInfo, &CurrentActivationInfo) ) return;

    if ( !WSC->Predict_ReloadEnd() )
    {
        CancelReload(ActorInfo);
        return;
    }

    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, false);
}

void UPX_GameplayAbility_GunReload::Authoritative_OnReloadEnd()
{
    PX_LOG(Log, TEXT(""));

    const FGameplayAbilityActorInfo* ActorInfo = GetCurrentActorInfo();
    if ( !ActorInfo )
    {
        PX_LOG(Warning, TEXT("Invalid ActorInfo"));
        return;
    }

    UPX_WeaponSystemComponent* WSC = GetWeaponSystemComponent(ActorInfo);
    if ( !WSC )
    {
        PX_LOG(Warning, TEXT("Invalid Weapon System Component"));
        return;
    }

    if ( !HasAuthority(&CurrentActivationInfo) ) return;

    if ( !WSC->Authoritative_ReloadEnd() )
    {
        CancelReload(ActorInfo);
        return;
    }

    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}


void UPX_GameplayAbility_GunReload::CancelReload(const FGameplayAbilityActorInfo* ActorInfo)
{
    PX_LOG(Log, TEXT(""));

    UPX_WeaponSystemComponent* WSC = GetWeaponSystemComponent(ActorInfo);
    if ( !WSC ) return;

    if ( HasAuthority(&CurrentActivationInfo) )
    {
        // Server Authoritative
        PX_LOG(Log, TEXT("Cancel Server Reload"));

        if ( GetWorld() )
        {
            GetWorld()->GetTimerManager().ClearTimer(ServerReloadTimerHandle);
        }

        WSC->Authoritative_CancelReload();
    }
    else if ( ShouldUseClientPrediction(ActorInfo, &CurrentActivationInfo) )
    {
        // Local Prediction
        PX_LOG(Log, TEXT("Cancel Client Predicted Reload"));

        if ( PredictWaitTask )
        {
            PredictWaitTask->ExternalCancel();
        }

        WSC->Predict_CancelReload();
    }
}

/*
APX_Character* UPX_GameplayAbility_GunReload::GetPXCharacter(const FGameplayAbilityActorInfo* ActorInfo) const
{
    if ( !ActorInfo || !ActorInfo->AvatarActor.IsValid() ) return nullptr;

    return Cast<APX_Character>(ActorInfo->AvatarActor.Get());
}

UPX_WeaponSystemComponent* UPX_GameplayAbility_GunReload::GetWeaponSystemComponent(const FGameplayAbilityActorInfo* ActorInfo) const
{
    APX_Character* Character = GetPXCharacter(ActorInfo);
    if ( !Character ) return nullptr;

    return Character->GetWeaponSystemComponent();
}
*/
