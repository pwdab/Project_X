// Fill out your copyright notice in the Description page of Project Settings.


#include "PX_BowActionHandler.h"
#include "Component/Weapon/PX_WeaponSystemComponent.h"
#include "Entity/PX_Weapon.h"

/*
FPXWeaponActionAnimContext UPX_BowActionHandler::Execute(const FPXWeaponActionContext& Action, UPX_WeaponSystemComponent& WeaponSystem)
{
    FPXWeaponActionAnimContext Out;

    const UPX_WeaponDataAsset* Data = WeaponSystem.GetWeapon()->GetWeaponData();
    if ( !Data ) { return Out; }

    Out.AnimMode = Data->AnimMode;
    Out.Priority = 10;

    switch ( Action.Type )
    {
    case EPXWeaponActionType::Equip:
        break;
    case EPXWeaponActionType::AttackPressed:
        Out.bSetIsAiming = true;
        Out.bIsAiming = true;
        Out.bSetIsDrawing = true;
        Out.bIsDrawing = true;
        break;

    case EPXWeaponActionType::AttackReleased:
        Out.bSetIsAiming = true;
        Out.bIsAiming = false;
        Out.bSetIsDrawing = true;
        Out.bIsDrawing = false;
        Out.bSetIsReloading = true;
        Out.bIsReloading = true;
        break;
    case EPXWeaponActionType::AttackOnce:
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
