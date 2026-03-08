// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Inventory/PX_InventoryWidget.h"
#include "UI/Inventory/PX_InventoryItemSlotsWidget.h"
#include "UI/Inventory/PX_InventoryWeaponSlotsWidget.h"
#include "Component/Inventory/PX_InventoryComponent.h"

void UPX_InventoryWidget::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    //InitInventory();
}

void UPX_InventoryWidget::NativeDestruct()
{
    UnbindInventory();

    Super::NativeDestruct();
}

void UPX_InventoryWidget::BindInventory(UPX_InventoryComponent* InInventory)
{
    if ( !InInventory ) return;

    // 중복 바인딩 처리
    if ( Inventory == InInventory && bInventoryBounded )
    {
        RefreshInventory();
        return;
    }

    // 기존 바인딩 해제
    UnbindInventory();

    // 인벤토리 초기화
    Inventory = InInventory;
    InitInventory();

    if ( !Inventory ) return;

    // 델리게이트 바인딩
    Inventory->OnInventoryReady.AddUObject(this, &UPX_InventoryWidget::HandleInventoryReady);
    Inventory->OnInventorySlotUpdated.AddUObject(this, &UPX_InventoryWidget::HandleSlotUpdated);
    //Inventory->OnInventorySlotsReset.AddUObject(this, &UPX_InventoryWidget::HandleSlotsReset);
    bInventoryBounded = true;

    // InventoryReady 처리
    if ( Inventory->IsInventoryReady() ) HandleInventoryReady();
}

void UPX_InventoryWidget::UnbindInventory()
{
    if ( Inventory && bInventoryBounded )
    {
        // 델리게이트 해제
        Inventory->OnInventoryReady.RemoveAll(this);
        Inventory->OnInventorySlotUpdated.RemoveAll(this);
        //Inventory->OnInventorySlotsReset.RemoveAll(this);
    }

    Inventory = nullptr;
    bInventoryBounded = false;
}

void UPX_InventoryWidget::RefreshInventory()
{
    //PX_LOG(Log, TEXT(""));
    if ( !Inventory || !Inventory->IsInventoryReady() )
    {
        InitInventory();
        return;
    }

    if ( ItemSlotsWidget ) ItemSlotsWidget->UpdateSlots(Inventory->GetItemSlots());
    if ( WeaponSlotsWidget ) WeaponSlotsWidget->UpdateSlots(Inventory->GetWeaponSlots());
}

void UPX_InventoryWidget::InitInventory()
{
    //PX_LOG(Log, TEXT(""));
    if ( ItemSlotsWidget ) ItemSlotsWidget->ClearSlots();
    if ( WeaponSlotsWidget ) WeaponSlotsWidget->ClearSlots();
}

void UPX_InventoryWidget::HandleInventoryReady()
{
    if ( !Inventory || !Inventory->IsInventoryReady() ) return;

    RefreshInventory();
}

void UPX_InventoryWidget::HandleSlotUpdated(EPXInventorySlotTarget Target, int32 SlotIndex, const FPXInventorySlot& InventorySlot)
{
    //PX_LOG(Log, TEXT(""));
    if ( Target == EPXInventorySlotTarget::Item )
    {
        if ( ItemSlotsWidget )
        {
            ItemSlotsWidget->UpdateSlot(SlotIndex, InventorySlot);
        }
    }
    else if ( Target == EPXInventorySlotTarget::Weapon )
    {
        if ( WeaponSlotsWidget )
        {
            WeaponSlotsWidget->UpdateSlot(SlotIndex, InventorySlot);
        }
    }
}

/*
void UPX_InventoryWidget::HandleSlotsReset(EPXInventorySlotTarget Target)
{
    if ( Target == EPXInventorySlotTarget::Item )
        if ( ItemSlotsWidget ) ItemSlotsWidget->ApplyAll(Inventory->GetItemSlotArray());
    else if ( Target == EPXInventorySlotTarget::Weapon )
        if ( WeaponSlotsWidget ) WeaponSlotsWidget->ApplyAll(Inventory->GetWeaponSlotArray());
}
*/
