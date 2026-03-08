// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/Inventory/PX_ItemInstance.h"
#include "../PX_ItemDataAsset.h"
#include "Entity/PX_Item.h"

void UPX_ItemInstance::InitializeFromData(UPX_ItemDataAsset* InDataAsset, int32 InQuantity)
{
    //if ( !HasServerAuthority() ) return;
    if ( !InDataAsset ) return;
    
	ItemDataAsset = InDataAsset;
	Quantity = ItemDataAsset->bStackable ? FMath::Clamp(InQuantity, 1, FMath::Max(1, ItemDataAsset->MaxStack)) : 1;
}

/*
UPX_ItemInstance* UPX_ItemInstance::ServerClone(UObject* NewOuter) const
{
	UPX_ItemInstance* NewItemInstance = NewObject<UPX_ItemInstance>(NewOuter, GetClass());
	NewItemInstance->ServerInitializeFromData(ItemDataAsset, Quantity);
	return NewItemInstance;
}
*/

FPX_ItemData UPX_ItemInstance::MakeDropData() const
{
    FPX_ItemData ItemData;
	ItemData.InstanceId = InstanceId;
	ItemData.Quantity = Quantity;
	ItemData.ItemDataAsset = ItemDataAsset;

    return ItemData;
}

void UPX_ItemInstance::ApplyDropData(const FPX_ItemData& Data)
{
    //ServerInitializeFromData(Data.ItemDataAsset, Data.Quantity);
    //if ( !HasServerAuthority() ) return;

    InstanceId = Data.InstanceId;
    Quantity = Data.Quantity;
    ItemDataAsset = Data.ItemDataAsset;

}

void UPX_ItemInstance::SetQuantity(const int32 InQuantity)
{
    if ( InQuantity < 0 ) return;
    if ( !ItemDataAsset->bStackable || ItemDataAsset->MaxStack < InQuantity ) return;

    Quantity = InQuantity;
}