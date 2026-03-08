// Fill out your copyright notice in the Description page of Project Settings.


#include "PX_GunActionHandler.h"
#include "Component/Weapon/PX_WeaponSystemComponent.h"
#include "Entity/PX_Weapon.h"

/*
FPXWeaponActionAnimContext UPX_GunActionHandler::Execute(const FPXWeaponActionContext& Action, UPX_WeaponSystemComponent& WeaponSystem)
{
    FPXWeaponActionAnimContext Out;

    const UPX_WeaponDataAsset* Data = WeaponSystem.GetWeapon()->GetWeaponData();
    if ( !Data ) { return Out; }

    Out.AnimMode = Data->AnimMode;
    Out.Priority = 10;

    switch ( Action.Type )
    {
    case EPXWeaponActionType::Equip:
        Out.MontageToPlay = Data->EquipAction;
        break;
    case EPXWeaponActionType::AttackPressed:
        break;
    case EPXWeaponActionType::AttackReleased:
        break;
    case EPXWeaponActionType::AttackOnce:
        Out.MontageToPlay = Action.bShouldDryFire ? Data->DryFireAction : Data->FireAction;
        Out.bSetIsAiming = false;
        break;
    case EPXWeaponActionType::Reload:
        Out.bSetIsReloading = true;
        Out.bIsReloading = true;
        Out.MontageToPlay = Data->ReloadAction;
        break;
    case EPXWeaponActionType::Aim_On:
        Out.bSetIsAiming = true;
        Out.bIsAiming = true;
        break;
    case EPXWeaponActionType::Aim_Off:
        Out.bSetIsAiming = true;
        Out.bIsAiming = false;
        break;
    }

    return Out;
}
*/


