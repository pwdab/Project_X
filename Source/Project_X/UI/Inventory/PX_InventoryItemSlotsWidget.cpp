// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Inventory/PX_InventoryItemSlotsWidget.h"
#include "UI/Inventory/PX_InventorySlotWidget.h"
#include "Component/Inventory/PX_InventoryComponent.h"
#include "Components/PanelWidget.h"

void UPX_InventoryItemSlotsWidget::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    InitSlotMeta();
}

void UPX_InventoryItemSlotsWidget::InitSlotMeta()
{
    if ( !SlotsPanel ) return;

    const int32 Count = SlotsPanel->GetChildrenCount();
    for ( int32 i = 0; i < Count; ++i )
    {
        if ( UPX_InventorySlotWidget* SlotWidget = Cast<UPX_InventorySlotWidget>(SlotsPanel->GetChildAt(i)) )
        {
            SlotWidget->SetSlotMeta(EPXInventorySlotTarget::Item, i);
        }
    }
}

void UPX_InventoryItemSlotsWidget::UpdateSlot(int32 SlotIndex, const FPXInventorySlot& InSlot)
{
    //PX_LOG(Log, TEXT(""));
    if ( !SlotsPanel ) return;

    if ( UPX_InventorySlotWidget* SlotWidget = Cast<UPX_InventorySlotWidget>(SlotsPanel->GetChildAt(SlotIndex)) )
    {
        SlotWidget->UpdateFromSlot(InSlot);
    }
}

void UPX_InventoryItemSlotsWidget::UpdateSlots(const TArray<FPXInventorySlot>& InSlots)
{
    for ( const FPXInventorySlot& ItemSlot : InSlots )
    {
        UpdateSlot(ItemSlot.SlotIndex, ItemSlot);
    }
}

void UPX_InventoryItemSlotsWidget::ClearSlot(int32 SlotIndex)
{
    if ( !SlotsPanel ) return;

    if ( UPX_InventorySlotWidget* SlotWidget = Cast<UPX_InventorySlotWidget>(SlotsPanel->GetChildAt(SlotIndex)) )
    {
        SlotWidget->ClearSlot();
    }
}

void UPX_InventoryItemSlotsWidget::ClearSlots()
{
    //PX_LOG(Log, TEXT(""));
    if ( !SlotsPanel ) return;

    const int32 Count = SlotsPanel->GetChildrenCount();
    for ( int32 i = 0; i < Count; ++i )
    {
        if ( UPX_InventorySlotWidget* SlotWidget = Cast<UPX_InventorySlotWidget>(SlotsPanel->GetChildAt(i)) )
        {
            SlotWidget->ClearSlot();
        }
    }
}

