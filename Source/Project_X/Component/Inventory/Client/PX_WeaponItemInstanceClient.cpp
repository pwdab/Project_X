// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/Inventory/PX_WeaponItemInstance.h"
#include "Component/Inventory/PX_ItemDataAsset.h"
#include "Component/Weapon/PX_WeaponDataAsset.h"

void UPX_WeaponItemInstance::OnRep_AmmoUpdated()
{
	OnAmmoUpdated.Broadcast(AmmoInMag, 1000);
}

void UPX_WeaponItemInstance::OnRep_AttackModeUpdated()
{
	//PX_LOG(Log, TEXT("AttackMode: %s"), WeaponAttackModeToString(AttackMode));
	OnAttackModeUpdated.Broadcast(AttackModeTag);
}




