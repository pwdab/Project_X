// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Inventory/PX_InventoryWeaponSlotsWidget.h"
#include "UI/Inventory/PX_InventorySlotWidget.h"
#include "Component/Inventory/PX_InventoryComponent.h"
#include "Components/PanelWidget.h"

void UPX_InventoryWeaponSlotsWidget::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    InitSlotMeta();
}

void UPX_InventoryWeaponSlotsWidget::InitSlotMeta()
{
    if ( !SlotsPanel ) return;

    const int32 Count = SlotsPanel->GetChildrenCount();
    for ( int32 i = 0; i < Count; ++i )
    {
        if ( UPX_InventorySlotWidget* SlotWidget = Cast<UPX_InventorySlotWidget>(SlotsPanel->GetChildAt(i)) )
        {
            SlotWidget->SetSlotMeta(EPXInventorySlotTarget::Weapon, i);
        }
    }
}

void UPX_InventoryWeaponSlotsWidget::UpdateSlot(int32 SlotIndex, const FPXInventorySlot& InSlot)
{
    //PX_LOG(Log, TEXT(""));
    if ( !SlotsPanel ) return;

    if ( UPX_InventorySlotWidget* SlotWidget = Cast<UPX_InventorySlotWidget>(SlotsPanel->GetChildAt(SlotIndex)) )
    {
        SlotWidget->UpdateFromSlot(InSlot);
    }
}

void UPX_InventoryWeaponSlotsWidget::UpdateSlots(const TArray<FPXInventorySlot>& InSlots)
{
    for ( const FPXInventorySlot& WeaponSlot : InSlots )
    {
        UpdateSlot(WeaponSlot.SlotIndex, WeaponSlot);
    }
}

void UPX_InventoryWeaponSlotsWidget::ClearSlot(int32 SlotIndex)
{
    if ( !SlotsPanel ) return;

    if ( UPX_InventorySlotWidget* SlotWidget = Cast<UPX_InventorySlotWidget>(SlotsPanel->GetChildAt(SlotIndex)) )
    {
        SlotWidget->ClearSlot();
    }
}

void UPX_InventoryWeaponSlotsWidget::ClearSlots()
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
