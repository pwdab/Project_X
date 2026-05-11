// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/Inventory/PX_WeaponItemInstance.h"
#include "Net/UnrealNetwork.h"

DEFINE_LOG_CATEGORY(PX_WeaponItemInstance);

void UPX_WeaponItemInstance::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(UPX_WeaponItemInstance, AmmoInMag);
    DOREPLIFETIME(UPX_WeaponItemInstance, AttackModeTag);
    DOREPLIFETIME(UPX_WeaponItemInstance, Durability);
}


