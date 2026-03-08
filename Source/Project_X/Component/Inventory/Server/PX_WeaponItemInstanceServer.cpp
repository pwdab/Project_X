// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/Inventory/PX_WeaponItemInstance.h"
#include "Component/Inventory/PX_ItemDataAsset.h"
#include "Component/Weapon/PX_WeaponDataAsset.h"
#include "Entity/PX_Item.h"

void UPX_WeaponItemInstance::InitializeFromData(UPX_ItemDataAsset* InItemDataAsset, int32 InQuantity)
{
	Super::InitializeFromData(InItemDataAsset, InQuantity);

	if ( !InItemDataAsset || !InItemDataAsset->WeaponData ) return;

	AmmoInMag = 0;
	AttackMode = InItemDataAsset->WeaponData->DefaultAttackMode;
	//Durability = InItemDataAsset->WeaponData->Durability;
}

/*
UPX_WeaponItemInstance* UPX_WeaponItemInstance::ServerClone(UObject* NewOuter) const
{

	UPX_WeaponItemInstance* NewItemInstance = NewObject<UPX_WeaponItemInstance>(NewOuter, GetClass());
	NewItemInstance->ServerInitializeFromData(ItemDataAsset, Quantity);

	// 동적 데이터는 현재값 복사
	NewItemInstance->AmmoInMag = AmmoInMag;
	NewItemInstance->AttackMode = AttackMode;
	NewItemInstance->Durability = Durability;

	return NewItemInstance;
}
*/

FPX_ItemData UPX_WeaponItemInstance::MakeDropData() const
{
	FPX_ItemData ItemData = Super::MakeDropData();
	ItemData.AmmoInMag = AmmoInMag;
	ItemData.AttackMode = AttackMode;
	ItemData.Durability = Durability;

	return ItemData;
}

void UPX_WeaponItemInstance::ApplyDropData(const FPX_ItemData& Data)
{
	//ServerInitializeFromData(Data.ItemDataAsset, Data.Quantity);

	InstanceId = Data.InstanceId;
	Quantity = Data.Quantity;
	ItemDataAsset = Data.ItemDataAsset;
	AmmoInMag = Data.AmmoInMag;
	AttackMode = Data.AttackMode;
	Durability = Data.Durability;
}

void UPX_WeaponItemInstance::SetAmmo(int32 Amount)
{
	if ( !ItemDataAsset || !ItemDataAsset->WeaponData ) return;

	AmmoInMag = FMath::Clamp(Amount, 0, ItemDataAsset->WeaponData->MagSize);
}

void UPX_WeaponItemInstance::ConsumeAmmo(int32 Amount)
{
	if ( Amount <= 0 ) return;
	if ( !ItemDataAsset || !ItemDataAsset->WeaponData ) return;

	AmmoInMag = FMath::Max(0, AmmoInMag - Amount);
}

void UPX_WeaponItemInstance::SwitchFireMode()
{
	if ( !ItemDataAsset || !ItemDataAsset->WeaponData ) return;
	if ( ItemDataAsset->WeaponData->SupportedAttackModes.Num() == 0 ) return;

	const int32 CurrentIndex = ItemDataAsset->WeaponData->SupportedAttackModes.IndexOfByKey(AttackMode);
	const int32 NextIndex = (CurrentIndex == INDEX_NONE) ? 0 : (CurrentIndex + 1) % ItemDataAsset->WeaponData->SupportedAttackModes.Num();

	SetFireMode(ItemDataAsset->WeaponData->SupportedAttackModes[NextIndex]);
}

void UPX_WeaponItemInstance::SetFireMode(EPXWeaponAttackMode NewMode)
{
	if ( AttackMode == NewMode ) return;
	if ( !ItemDataAsset->WeaponData || !ItemDataAsset->WeaponData->SupportedAttackModes.Contains(NewMode) ) return;
	//PX_LOG(Log, TEXT("SetFireMode %s -> %s"), WeaponAttackModeToString(AttackMode), WeaponAttackModeToString(NewMode));

	AttackMode = NewMode;
}