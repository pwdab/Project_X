// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/Inventory/PX_ItemDataAsset.h"
#include "Component/Inventory/PX_ItemInstance.h"
#include "AbilitySystem/Tags/PX_GameplayTags.h"

UPX_ItemInstance* UPX_ItemDataAsset::CreateItemInstance(UObject* Outer, int32 InQuantity, const FGuid& InInstanceId)
{
    if ( !InstanceClass )
    {
        PX_LOG(Warning, TEXT("InstanceClass is null in %s"), *GetName());
        return nullptr;
    }

    UPX_ItemInstance* NewInstance = NewObject<UPX_ItemInstance>(Outer, InstanceClass);
    
    if ( !NewInstance ) return nullptr;

    if ( InInstanceId.IsValid() )
    {
        NewInstance->InstanceId = InInstanceId;
    }
    else
    {
        NewInstance->InstanceId = FGuid::NewGuid();   // 서버에서만
        PX_LOG(Log, TEXT("New Item Instance is Created. Name : %s, Guid : %s"), *ItemName.ToString(), *NewInstance->InstanceId.ToString());
    }
    
    NewInstance->InitializeFromData(this, InQuantity);

    return NewInstance;
}

bool UPX_ItemDataAsset::IsEquippableItem() const
{
    return ItemCategoryTag.MatchesTag(PX_GameplayTags::Item_Category_Equippable);
}

bool UPX_ItemDataAsset::IsWeaponItem() const
{
    return ItemCategoryTag.MatchesTag(PX_GameplayTags::Item_Category_Equippable_Weapon);
}

bool UPX_ItemDataAsset::IsArmorItem() const
{
    return ItemCategoryTag.MatchesTag(PX_GameplayTags::Item_Category_Equippable_Armor);
}

bool UPX_ItemDataAsset::IsAccessoryItem() const
{
    return ItemCategoryTag.MatchesTag(PX_GameplayTags::Item_Category_Equippable_Accessory);
}

bool UPX_ItemDataAsset::IsConsumableItem() const
{
    return ItemCategoryTag.MatchesTag(PX_GameplayTags::Item_Category_Consumable);
}

bool UPX_ItemDataAsset::PrefersWeaponInventory() const
{
    return ItemInventoryTag.MatchesTag(PX_GameplayTags::Item_Inventory_Weapon);
}

bool UPX_ItemDataAsset::PrefersArmorInventory() const
{
    return ItemInventoryTag.MatchesTag(PX_GameplayTags::Item_Inventory_Armor);
}

bool UPX_ItemDataAsset::PrefersAccessoryInventory() const
{
    return ItemInventoryTag.MatchesTag(PX_GameplayTags::Item_Inventory_Accessory);
}

bool UPX_ItemDataAsset::IsGeneralSlotOnly() const
{
    return ItemInventoryTag.MatchesTag(PX_GameplayTags::Item_Inventory_General);
}

bool UPX_ItemDataAsset::IsStackableItem() const
{
    return ItemTypeTags.HasTag(PX_GameplayTags::Item_Type_Stackable);
}

bool UPX_ItemDataAsset::IsDroppableItem() const
{
    if ( ItemTypeTags.HasTag(PX_GameplayTags::Item_Type_NotDroppable) )
    {
        return false;
    }

    return ItemTypeTags.HasTag(PX_GameplayTags::Item_Type_Droppable);
}

bool UPX_ItemDataAsset::IsExchangeableItem() const
{
    if ( ItemTypeTags.HasTag(PX_GameplayTags::Item_Type_NotExchangeable) )
    {
        return false;
    }

    return ItemTypeTags.HasTag(PX_GameplayTags::Item_Type_Exchangeable);
}


