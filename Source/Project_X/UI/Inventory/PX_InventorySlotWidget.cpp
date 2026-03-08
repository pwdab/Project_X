// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Inventory/PX_InventorySlotWidget.h"
//#include "Component/Inventory/PX_InventoryComponent.h"
#include "Component/Inventory/PX_ItemInstance.h"

void UPX_InventorySlotWidget::SetSlotMeta(EPXInventorySlotTarget InTarget, int32 InSlotIndex)
{
    Target = InTarget;
    SlotIndex = InSlotIndex;
}

void UPX_InventorySlotWidget::UpdateFromSlot(const FPXInventorySlot& InSlot)
{
    //PX_LOG(Log, TEXT(""));
    SlotIndex = InSlot.SlotIndex;
    ItemInstance = InSlot.ItemInstance;

    if ( !ItemInstance )
    {
        ClearSlot();
        return;
    }

    BP_UpdateSlot(ItemInstance);
}

void UPX_InventorySlotWidget::ClearSlot()
{
    ItemInstance = nullptr;
    BP_ClearSlot();
}



