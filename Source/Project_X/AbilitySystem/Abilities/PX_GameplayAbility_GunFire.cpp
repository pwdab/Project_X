// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/PX_GameplayAbility_GunFire.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystem/Component/PX_AbilitySystemComponent.h"
#include "AbilitySystem/Tags/PX_GameplayTags.h"
#include "Animation/AnimInstance.h"
#include "Component/Inventory/PX_EquippableItemDataAsset.h"
#include "Component/Inventory/PX_WeaponItemInstance.h"
#include "Component/Weapon/PX_WeaponDataAsset.h"
#include "Component/Weapon/PX_WeaponSystemComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Entity/PX_Character.h"
#include "Entity/PX_Weapon.h"
#include "GameplayPrediction.h"
#include "TimerManager.h"

UPX_GameplayAbility_GunFire::UPX_GameplayAbility_GunFire()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
    NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

    InputTag = PX_GameplayTags::Input_Combat_Attack_Gun;

    AbilityTags.AddTag(PX_GameplayTags::Ability_Weapon_Attack_Gun);

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

bool UPX_GameplayAbility_GunFire::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
    if ( !Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags) ) return false;

    PX_LOG(Log, TEXT(""));

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

    return true;
}

void UPX_GameplayAbility_GunFire::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

    PX_LOG(Log, TEXT(""));

    if ( !HasAuthority(&ActivationInfo) && ActorInfo && ActorInfo->IsLocallyControlled() && !ShouldUseClientPrediction(ActorInfo, &ActivationInfo) )
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
        return;
    }

    if ( HasAuthority(&ActivationInfo) )
    {
        if ( UPX_AbilitySystemComponent* PX_ASC = Cast<UPX_AbilitySystemComponent>(GetAbilitySystemComponent(ActorInfo)) )
        {
            PX_ASC->ClearServerFallbackActivation(Handle);
        }
    }

    // Commit Ability
    if ( !CommitAbility(Handle, ActorInfo, ActivationInfo) )
    {
        PX_LOG(Warning, TEXT("CommitAbility Failed"));
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    UPX_WeaponSystemComponent* WeaponSystemComponent = GetWeaponSystemComponent(ActorInfo);
    if ( !WeaponSystemComponent )
    {
        PX_LOG(Warning, TEXT("Invalid Weapon System Component"));
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    UPX_WeaponItemInstance* WeaponItemInstance = WeaponSystemComponent->GetWeaponInstance();
    if ( !WeaponItemInstance )
    {
        PX_LOG(Warning, TEXT("Invalid WeaponItemInstance"));
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    UPX_WeaponDataAsset* WeaponDataAsset = WeaponItemInstance->GetWeaponDataAsset();
    if ( !WeaponDataAsset )
    {
        PX_LOG(Warning, TEXT("Invalid Weapon Data"));
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    FGameplayTag FireModeTag = WeaponSystemComponent->GetCurrentAttackModeTag();
    if ( HasAuthority(&ActivationInfo) )
    {
        ServerBurstRemaining = WeaponDataAsset->BurstCount;

        if ( FireModeTag == PX_GameplayTags::Weapon_AttackMode_Burst )
        {
            ServerBurstRemaining = WeaponDataAsset->BurstCount;
        }
    }
    else if ( ShouldUseClientPrediction(ActorInfo, &ActivationInfo) )
    {
        if ( FireModeTag == PX_GameplayTags::Weapon_AttackMode_Burst )
        {
            PredictBurstRemaining = WeaponDataAsset->BurstCount;
        }

        PX_LOG(Log, TEXT("ActivateAbility Local WeaponItemInstance Ammo: %d"), WeaponItemInstance->GetAmmo());
    }

    UAbilitySystemComponent* ASC = GetAbilitySystemComponent(ActorInfo);
    if ( !ASC )
    {
        PX_LOG(Warning, TEXT("Invalid ASC"));
        return;
    }

    bPendingDryFire = false;
    bWaitingForServerFireTargetData = false;
    bStopFireRequested = false;

    // 사격 시작
    FireOnce();
}

void UPX_GameplayAbility_GunFire::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
    if ( !IsActive() )
    {
        PX_LOG(Log, TEXT("Ability is InActive"));
        return;
    }

    if ( bWasCancelled ) CancelFire(ActorInfo);

    if ( UWorld* World = GetWorld() )
    {
        World->GetTimerManager().ClearTimer(ServerFireTimerHandle);
        World->GetTimerManager().ClearTimer(PredictFireTimerHandle);
    }

    if ( ActivationInfo.ActivationMode == EGameplayAbilityActivationMode::Rejected && ShouldUseClientPrediction(ActorInfo, &ActivationInfo) )
    {
        if ( UPX_WeaponSystemComponent* WSC = GetWeaponSystemComponent(ActorInfo) )
        {
            WSC->Predict_RejectFireOnce();
        }
    }

    if ( UAbilitySystemComponent* ASC = GetAbilitySystemComponent(ActorInfo) )
    {
        ASC->AbilityTargetDataSetDelegate(Handle, ActivationInfo.GetActivationPredictionKey()).RemoveAll(this);
    }

    StopWaitingForServerFireTargetData();
    bStopFireRequested = false;

    PX_LOG(Log, TEXT(""));

    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UPX_GameplayAbility_GunFire::FireOnce(const FGameplayAbilityTargetDataHandle* ServerFireTargetData)
{
    UPX_WeaponSystemComponent* WeaponSystemComponent = GetWeaponSystemComponent(CurrentActorInfo);
    if ( !WeaponSystemComponent )
    {
        PX_LOG(Warning, TEXT("Invalid Weapon System Component"));
        return;
    }

    UPX_WeaponItemInstance* WeaponItemInstance = WeaponSystemComponent->GetWeaponInstance();
    if ( !WeaponItemInstance )
    {
        PX_LOG(Warning, TEXT("Invalid WeaponItemInstance"));
        return;
    }

    UPX_WeaponDataAsset* WeaponDataAsset = WeaponItemInstance->GetWeaponDataAsset();
    if ( !WeaponDataAsset )
    {
        PX_LOG(Warning, TEXT("Invalid Weapon Data"));
        return;
    }

    const bool bUsePredictedAmmo = ShouldUseClientPrediction(CurrentActorInfo, &CurrentActivationInfo);
    const int32 AmmoInMag = bUsePredictedAmmo ? WeaponSystemComponent->GetCurrentAmmoInMag() : WeaponItemInstance->GetAmmo();

    bool bDryFire = AmmoInMag <= 0;
    UAnimMontage* CharacterFireMontage = bDryFire ? WeaponDataAsset->DryFireAction.CharacterMontage : WeaponDataAsset->FireAction.CharacterMontage;
    if ( !CharacterFireMontage )
    {
        PX_LOG(Warning, TEXT("Invalid CharacterFireMontage"));
        EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
        return;
    }

    
    UAnimMontage* WeaponFireMontage = bDryFire ? WeaponDataAsset->DryFireAction.WeaponMontage : WeaponDataAsset->FireAction.WeaponMontage;
    /*
    if ( !WeaponFireMontage )
    {
        PX_LOG(Warning, TEXT("Invalid WeaponFireMontage"));
        EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
        return;
    }
    */

    UWorld* World = GetWorld();
    if ( !World )
    {
        PX_LOG(Warning, TEXT("Invalid World"));
        return;
    }

    // Fire 시도
    float FireInterval = WeaponDataAsset->FireInterval;
    const bool bIsLastAmmo = (AmmoInMag == 1);
    FGameplayTag FireModeTag = WeaponSystemComponent->GetCurrentAttackModeTag();
    if ( HasAuthority(&CurrentActivationInfo) )
    {
        if ( CurrentActorInfo && !CurrentActorInfo->IsLocallyControlled() && !ServerFireTargetData && AmmoInMag > 0 && WeaponDataAsset->ProjectileClass )
        {
            WaitForServerFireTargetData();
            return;
        }

        // Server Authoritative
        PX_LOG(Log, TEXT("Begin Server Fire"));

        if ( !WeaponSystemComponent->Authoritative_FireOnce(ServerFireTargetData) )
        {
            PX_LOG(Warning, TEXT("Fail Server Fire"));
            EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
            return;
        }
        WeaponSystemComponent->Multicast_FireOnce();

        if ( FireModeTag == PX_GameplayTags::Weapon_AttackMode_Burst )
        {
            --ServerBurstRemaining;
        }

        World->GetTimerManager().SetTimer(ServerFireTimerHandle, this, &UPX_GameplayAbility_GunFire::Authoritative_OnFireOnceEnd, FireInterval, false);
    }
    else if ( ShouldUseClientPrediction(CurrentActorInfo, &CurrentActivationInfo) )
    {
        // Local Prediction
        PX_LOG(Log, TEXT("Begin Client Predicted Fire"));

        if ( AmmoInMag > 0 && WeaponDataAsset->ProjectileClass )
        {
            SendPredictedFireTargetData(WeaponSystemComponent);
        }

        if ( !WeaponSystemComponent->Predict_FireOnce() )
        {
            PX_LOG(Warning, TEXT("Fail Client Predicted Fire"));
            EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
            return;
        }
        WeaponSystemComponent->Local_PlayCharacterMontage(WeaponDataAsset->CharacterAnimClass, CharacterFireMontage, WeaponDataAsset->FireAction.PlayRate);
        WeaponSystemComponent->Local_PlayWeaponMontage(WeaponDataAsset->WeaponAnimClass, WeaponFireMontage, WeaponDataAsset->FireAction.PlayRate);

        if ( FireModeTag == PX_GameplayTags::Weapon_AttackMode_Burst )
        {
            --PredictBurstRemaining;
        }

        World->GetTimerManager().SetTimer(PredictFireTimerHandle, this, &UPX_GameplayAbility_GunFire::Predict_OnFireOnceEnd, FireInterval, false);
    }

    // 연사 모드에서 잔탄이 0이 되면 다음 FireOnce()에서 DryFire를 발동
    if ( !bPendingDryFire && bIsLastAmmo )
    {
        // 연사 모드에서는 잔탄이 0이 되면 항상 DryFire를 발동
        if ( FireModeTag == PX_GameplayTags::Weapon_AttackMode_Auto )
        {
            bPendingDryFire = true;
        }

        // 점사 모드에서는 Burst Remaining이 있는 상태에서 잔탄이 0이 되면 DryFire를 발동
        if ( FireModeTag == PX_GameplayTags::Weapon_AttackMode_Burst )
        {
            if ( HasAuthority(&CurrentActivationInfo) && ServerBurstRemaining > 0 )
            {
                bPendingDryFire = true;
            }
            else if ( ShouldUseClientPrediction(CurrentActorInfo, &CurrentActivationInfo) && PredictBurstRemaining > 0 )
            {
                bPendingDryFire = true;
            }
        }
    }
}

void UPX_GameplayAbility_GunFire::SendPredictedFireTargetData(UPX_WeaponSystemComponent* WeaponSystemComponent)
{
    UAbilitySystemComponent* ASC = GetAbilitySystemComponent(CurrentActorInfo);
    if ( !ASC || !WeaponSystemComponent )
    {
        return;
    }

    FGameplayAbilityTargetDataHandle TargetData;
    if ( !WeaponSystemComponent->MakePredictedProjectileFireTargetData(TargetData) )
    {
        return;
    }

    FScopedPredictionWindow ScopedPrediction(ASC, true);
    ASC->CallServerSetReplicatedTargetData(CurrentSpecHandle, CurrentActivationInfo.GetActivationPredictionKey(), TargetData, FGameplayTag(), ASC->ScopedPredictionKey);
}

void UPX_GameplayAbility_GunFire::WaitForServerFireTargetData()
{
    UAbilitySystemComponent* ASC = GetAbilitySystemComponent(CurrentActorInfo);
    if ( !ASC )
    {
        PX_LOG(Warning, TEXT("Invalid ASC"));
        EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
        return;
    }

    const FPredictionKey ActivationPredictionKey = CurrentActivationInfo.GetActivationPredictionKey();
    ASC->AbilityTargetDataSetDelegate(CurrentSpecHandle, ActivationPredictionKey).RemoveAll(this);
    ASC->AbilityTargetDataSetDelegate(CurrentSpecHandle, ActivationPredictionKey).AddUObject(this, &UPX_GameplayAbility_GunFire::OnServerFireTargetDataReady);
    bWaitingForServerFireTargetData = true;

    ASC->CallReplicatedTargetDataDelegatesIfSet(CurrentSpecHandle, ActivationPredictionKey);
}

void UPX_GameplayAbility_GunFire::OnServerFireTargetDataReady(const FGameplayAbilityTargetDataHandle& Data, FGameplayTag ActivationTag)
{
    FGameplayAbilityTargetDataHandle MutableData = Data;

    UAbilitySystemComponent* ASC = GetAbilitySystemComponent(CurrentActorInfo);
    if ( ASC )
    {
        ASC->AbilityTargetDataSetDelegate(CurrentSpecHandle, CurrentActivationInfo.GetActivationPredictionKey()).RemoveAll(this);
        ASC->ConsumeClientReplicatedTargetData(CurrentSpecHandle, CurrentActivationInfo.GetActivationPredictionKey());
    }

    bWaitingForServerFireTargetData = false;

    if ( bStopFireRequested )
    {
        EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
        return;
    }

    FireOnce(&MutableData);
}

void UPX_GameplayAbility_GunFire::StopWaitingForServerFireTargetData()
{
    if ( !bWaitingForServerFireTargetData )
    {
        return;
    }

    if ( UAbilitySystemComponent* ASC = GetAbilitySystemComponent(CurrentActorInfo) )
    {
        ASC->AbilityTargetDataSetDelegate(CurrentSpecHandle, CurrentActivationInfo.GetActivationPredictionKey()).RemoveAll(this);
        ASC->ConsumeClientReplicatedTargetData(CurrentSpecHandle, CurrentActivationInfo.GetActivationPredictionKey());
    }

    bWaitingForServerFireTargetData = false;
}

void UPX_GameplayAbility_GunFire::Predict_OnFireOnceEnd()
{
    PX_LOG(Log, TEXT(""));

    UPX_WeaponSystemComponent* WeaponSystemComponent = GetWeaponSystemComponent(CurrentActorInfo);
    if ( !WeaponSystemComponent )
    {
        PX_LOG(Warning, TEXT("Invalid Weapon System Component"));
        return;
    }

    UPX_WeaponItemInstance* WeaponItemInstance = WeaponSystemComponent->GetWeaponInstance();
    if ( !WeaponItemInstance )
    {
        PX_LOG(Warning, TEXT("Invalid WeaponItemInstance"));
        return;
    }

    if ( !ShouldUseClientPrediction(CurrentActorInfo, &CurrentActivationInfo) ) return;

    if ( !WeaponSystemComponent->Predict_FireOnceEnd() )
    {
        CancelFire(CurrentActorInfo);
        return;
    }

    if ( ShouldFireAgain() )
    {
        FireOnce();
        return;
    }

    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, false);
}

void UPX_GameplayAbility_GunFire::Authoritative_OnFireOnceEnd()
{
    PX_LOG(Log, TEXT(""));

    UPX_WeaponSystemComponent* WeaponSystemComponent = GetWeaponSystemComponent(CurrentActorInfo);
    if ( !WeaponSystemComponent )
    {
        PX_LOG(Warning, TEXT("Invalid Weapon System Component"));
        return;
    }

    UPX_WeaponItemInstance* WeaponItemInstance = WeaponSystemComponent->GetWeaponInstance();
    if ( !WeaponItemInstance )
    {
        PX_LOG(Warning, TEXT("Invalid WeaponItemInstance"));
        return;
    }

    if ( !HasAuthority(&CurrentActivationInfo) ) return;

    if ( !WeaponSystemComponent->Authoritative_FireOnceEnd() )
    {
        CancelFire(CurrentActorInfo);
        return;
    }

    if ( ShouldFireAgain() )
    {
        FireOnce();
        return;
    }

    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UPX_GameplayAbility_GunFire::CancelFire(const FGameplayAbilityActorInfo* ActorInfo)
{
    PX_LOG(Log, TEXT(""));

    UPX_WeaponSystemComponent* WSC = GetWeaponSystemComponent(ActorInfo);
    if ( !WSC ) return;

    if ( HasAuthority(&CurrentActivationInfo) )
    {
        // Server Authoritative
        PX_LOG(Log, TEXT("Cancel Server Fire"));

        if ( GetWorld() )
        {
            GetWorld()->GetTimerManager().ClearTimer(ServerFireTimerHandle);
        }

        ServerBurstRemaining = 0;

        WSC->Client_CancelFireOnce();
        WSC->Authoritative_CancelFireOnce();
    }
    else if ( ShouldUseClientPrediction(ActorInfo, &CurrentActivationInfo) )
    {
        // Local Prediction
        PX_LOG(Log, TEXT("Cancel Client Predicted Fire"));

        if ( GetWorld() )
        {
            GetWorld()->GetTimerManager().ClearTimer(PredictFireTimerHandle);
        }

        PredictBurstRemaining = 0;

        WSC->Predict_CancelFireOnce();
    }

    //EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UPX_GameplayAbility_GunFire::InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
    Super::InputReleased(Handle, ActorInfo, ActivationInfo);

    PX_LOG(Log, TEXT(""));

    UPX_WeaponSystemComponent* WSC = GetWeaponSystemComponent(ActorInfo);
    if ( !WSC )
    {
        PX_LOG(Warning, TEXT("Invalid Weapon System Component"));
        return;
    }

    bStopFireRequested = true;

    if ( HasAuthority(&ActivationInfo) && bWaitingForServerFireTargetData )
    {
        StopWaitingForServerFireTargetData();
        EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
    }
}

bool UPX_GameplayAbility_GunFire::ShouldFireAgain()
{
    if ( bStopFireRequested )
    {
        return false;
    }

    UPX_WeaponSystemComponent* WeaponSystemComponent = GetWeaponSystemComponent(CurrentActorInfo);
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

    UPX_AbilitySystemComponent* PX_AbilitySystemComponent = Cast<UPX_AbilitySystemComponent>(GetAbilitySystemComponent(CurrentActorInfo));
    if ( !PX_AbilitySystemComponent )
    {
        PX_LOG(Warning, TEXT("Invalid PX_AbilitySystemComponent"));
        return false;
    }

    FGameplayTag FireModeTag = WeaponSystemComponent->GetCurrentAttackModeTag();
    // 연사 모드
    const bool bUsePredictedAmmo = ShouldUseClientPrediction(CurrentActorInfo, &CurrentActivationInfo);
    const int32 AmmoInMag = bUsePredictedAmmo ? WeaponSystemComponent->GetCurrentAmmoInMag() : WeaponItemInstance->GetAmmo();

    if ( FireModeTag == PX_GameplayTags::Weapon_AttackMode_Auto )
    {
        if ( !PX_AbilitySystemComponent->IsAbilitySpecHandleHeld(CurrentSpecHandle) ) return false;

        if ( AmmoInMag > 0 ) return true;

        if ( bPendingDryFire )
        {
            bPendingDryFire = false;
            return true;
        }

        return false;
    }

    // 점사 모드
    if ( FireModeTag == PX_GameplayTags::Weapon_AttackMode_Burst )
    {
        // BurstRemaining이 있는 상태에서 잔탄이 0이 되어 DryFire 발동
        if ( bPendingDryFire && AmmoInMag <= 0 )
        {
            bPendingDryFire = false;
            return true;
        }

        // DryFire가 아니면 잔탄이 0인 순간 즉시 종료
        if ( AmmoInMag <= 0 )
        {
            return false;
        }

        if ( HasAuthority(&CurrentActivationInfo) )
        {
            PX_LOG(Log, TEXT("ServerBurstRemaining: %d"), ServerBurstRemaining);
            return ServerBurstRemaining > 0;
        }
        else if ( ShouldUseClientPrediction(CurrentActorInfo, &CurrentActivationInfo) )
        {
            PX_LOG(Log, TEXT("PredictBurstRemaining: %d"), PredictBurstRemaining);
            return PredictBurstRemaining > 0;
        }
    }

    return false;
}

/*
bool UPX_GameplayAbility_GunFire::HasEquippedGunWeapon(const FGameplayAbilityActorInfo* ActorInfo) const
{
    APX_Character* Character = GetPXCharacter(ActorInfo);
    if ( !Character )
    {
        return false;
    }

    UPX_WeaponSystemComponent* WSC = Character->GetWeaponSystemComponent();
    if ( !WSC )
    {
        return false;
    }

    UPX_WeaponItemInstance* WeaponItemInstance = WSC->GetWeaponInstance();
    if ( !WeaponItemInstance )
    {
        return false;
    }

    UPX_EquippableItemDataAsset* EquippableItemData = WeaponItemInstance->GetEquippableItemDataAsset();
    if ( !EquippableItemData )
    {
        return false;
    }

    return EquippableItemData->GrantedTags.HasTag(PX_GameplayTags::Weapon_Type_Gun);
}
*/

/*
APX_Character* UPX_GameplayAbility_GunFire::GetPXCharacter(const FGameplayAbilityActorInfo* ActorInfo) const
{
    if ( !ActorInfo || !ActorInfo->AvatarActor.IsValid() )
    {
        return nullptr;
    }

    return Cast<APX_Character>(ActorInfo->AvatarActor.Get());
}

UPX_WeaponSystemComponent* UPX_GameplayAbility_GunFire::GetWeaponSystemComponent(const FGameplayAbilityActorInfo* ActorInfo) const
{
    APX_Character* Character = GetPXCharacter(ActorInfo);
    if ( !Character )
    {
        return nullptr;
    }

    return Character->GetWeaponSystemComponent();
}

UAbilitySystemComponent* UPX_GameplayAbility_GunFire::GetASC(const FGameplayAbilityActorInfo* ActorInfo) const
{
    if ( !ActorInfo )
    {
        return nullptr;
    }

    return ActorInfo->AbilitySystemComponent.Get();
}
*/
