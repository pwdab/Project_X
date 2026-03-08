// Fill out your copyright notice in the Description page of Project Settings.

#include "PX_WeaponActionHandler.h"
#include "Entity/PX_Weapon.h"
#include "Component/Weapon/PX_WeaponDataAsset.h"
#include "Component/Weapon/PX_WeaponSystemComponent.h"
#include "Component/Weapon/Client/Driver/PX_WeaponAnimDriver.h"

DEFINE_LOG_CATEGORY(PX_WeaponActionHandler);

void UPX_WeaponActionHandler::Initialize(UPX_WeaponSystemComponent* InWeaponSystemComponent, UPX_WeaponAnimDriver* InAnimDriver, UPX_WeaponDataAsset* InWeaponData)
{
    if ( !InWeaponSystemComponent || !InAnimDriver || !InWeaponData ) return;

    WeaponSystemComponent = InWeaponSystemComponent;
    AnimDriver = InAnimDriver;
    WeaponData = InWeaponData;
    //PX_LOG(Log, TEXT("ActionHandler Init WeaponData=%s"), WeaponData ? *WeaponData->GetPathName() : TEXT("None"));
    //PX_LOG(Log, TEXT("Weapon Action Handler Init Complete"));
}

void UPX_WeaponActionHandler::CommitCosmetic(const FPXWeaponActionContext& ActionContext, bool bIsLocal)
{
    if ( !AnimDriver ) return;
    if ( !WeaponData ) return;
    //PX_LOG(Log, TEXT("AnimDriver and WeaponData are not null"));

    FPXWeaponActionAnimContext AnimContext;
    if ( !BuildAnimContext(ActionContext, AnimContext, bIsLocal) ) return;
    //PX_LOG(Log, TEXT(""));

    AnimDriver->PlayCommittedAction(AnimContext, bIsLocal);
}

bool UPX_WeaponActionHandler::BuildAnimContext(const FPXWeaponActionContext& ActionContext, FPXWeaponActionAnimContext& OutAnimContext, bool bIsLocal) const
{
    if ( !WeaponData ) return false;
    //if ( !WeaponSlots ) return false;

    OutAnimContext = FPXWeaponActionAnimContext {};

    switch ( ActionContext.Type )
    {
    case EPXWeaponActionType::BeginEquip:
    {
        
        bool rtn = CopyMontageDataToAnimContext(WeaponData->EquipAction, OutAnimContext);
        //PX_LOG(Log, TEXT("Build BeginEquip AnimContext : %s"), rtn ? TEXT("true") : TEXT("false"));
        //return CopyMontageDataToAnimContext(WeaponData->EquipAction, OutAnimContext);
        return rtn;
    }

    case EPXWeaponActionType::BeginReload:
        return CopyMontageDataToAnimContext(WeaponData->ReloadAction, OutAnimContext);

    case EPXWeaponActionType::AttackOnce:
    {
        // Use Server's bShouldDryFire
        if ( ActionContext.bShouldDryFire )
        {
            //PX_LOG(Log, TEXT("bShouldDryFire is true"));
            return CopyMontageDataToAnimContext(WeaponData->DryFireAction, OutAnimContext);
        }

        /*
        // Use Weapon's Ammo
        if ( WeaponSystemComponent )
        {
            if ( const APX_Weapon* Weapon = WeaponSystemComponent->GetWeapon() )
            {
                const bool bUsesAmmo = (WeaponData->ProjectileClass != nullptr && WeaponData->MagSize > 0);
                if ( bUsesAmmo && Weapon->GetAmmo() == 0 )
                {
                    PX_LOG(Log, TEXT("Weapon Ammo Should Dry Fire"));
                    return CopyMontageDataToAnimContext(WeaponData->DryFireAction, OutAnimContext);
                }
            }
        }
        */

        //PX_LOG(Log, TEXT("bShouldDryFire is false"));
        return CopyMontageDataToAnimContext(WeaponData->FireAction, OutAnimContext);
    }

    default:
        return false;
    }
}

bool UPX_WeaponActionHandler::CopyMontageDataToAnimContext(const FPXWeaponActionMontageData& In, FPXWeaponActionAnimContext& Out)
{
    /*
    if ( In.CharacterMontage == nullptr )
    {
        PX_LOG(Log, TEXT("CharacterMontage is null"));
    }
    else
    {
        PX_LOG(Log, TEXT("CharacterMontage is not null"));
    }
    if ( In.WeaponMontage == nullptr )
    {
        PX_LOG(Log, TEXT("WeaponMontage is null"));
    }
    else
    {
        PX_LOG(Log, TEXT("WeaponMontage is not null"));
    }
    */
    const bool bHasAny = (In.CharacterMontage != nullptr) || (In.WeaponMontage != nullptr);
    if ( !bHasAny ) return false;

    Out.MontageToPlay.CharacterMontage = In.CharacterMontage;
    Out.MontageToPlay.CharacterSection = In.CharacterSection;
    Out.MontageToPlay.WeaponMontage = In.WeaponMontage;
    Out.MontageToPlay.WeaponSection = In.WeaponSection;
    Out.MontageToPlay.PlayRate = (In.PlayRate > 0.f) ? In.PlayRate : 1.f;
    return true;
}