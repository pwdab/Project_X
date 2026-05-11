// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Inventory/PX_InventoryWidget.h"
#include "UI/Inventory/PX_InventoryItemSlotsWidget.h"
#include "UI/Inventory/PX_InventoryWeaponSlotsWidget.h"
#include "Component/Inventory/PX_InventoryComponent.h"
#include "Component/Inventory/PX_ItemInstance.h"

namespace
{
    constexpr int32 PX_InventoryDeferredRefreshMaxAttempts = 10;
    constexpr float PX_InventoryDeferredRefreshInterval = 0.1f;
}

void UPX_InventoryWidget::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    //InitInventory();
}

void UPX_InventoryWidget::NativeDestruct()
{
    if ( UWorld* World = GetWorld() )
    {
        World->GetTimerManager().ClearTimer(DeferredRefreshTimerHandle);
    }

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
    if ( UWorld* World = GetWorld() )
    {
        World->GetTimerManager().ClearTimer(DeferredRefreshTimerHandle);
    }

    if ( Inventory && bInventoryBounded )
    {
        // 델리게이트 해제
        Inventory->OnInventoryReady.RemoveAll(this);
        Inventory->OnInventorySlotUpdated.RemoveAll(this);
        //Inventory->OnInventorySlotsReset.RemoveAll(this);
    }

    Inventory = nullptr;
    bInventoryBounded = false;
    DeferredRefreshAttemptCount = 0;
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
    RequestDeferredRefresh();
}

//void UPX_InventoryWidget::HandleSlotUpdated(EPXInventorySlotTarget Target, int32 SlotIndex, const FPXInventorySlot& InventorySlot)
void UPX_InventoryWidget::HandleSlotUpdated(FGameplayTag Target, int32 SlotIndex, const FPXInventorySlot& InventorySlot)
{
    //PX_LOG(Log, TEXT(""));
    //if ( Target == EPXInventorySlotTarget::Item )
    if ( Target == PX_GameplayTags::Item_Inventory_General )
    {
        if ( ItemSlotsWidget )
        {
            ItemSlotsWidget->UpdateSlot(SlotIndex, InventorySlot);
        }
    }
    //else if ( Target == EPXInventorySlotTarget::Weapon )
    else if ( Target == PX_GameplayTags::Item_Inventory_Weapon )
    {
        if ( WeaponSlotsWidget )
        {
            WeaponSlotsWidget->UpdateSlot(SlotIndex, InventorySlot);
        }
    }

    RequestDeferredRefresh();
}

void UPX_InventoryWidget::RequestDeferredRefresh()
{
    if ( !Inventory || !Inventory->IsInventoryReady() )
    {
        return;
    }

    UWorld* World = GetWorld();
    if ( !World )
    {
        RefreshInventory();
        return;
    }

    DeferredRefreshAttemptCount = 0;
    World->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateWeakLambda(this, [this]()
    {
        HandleDeferredRefresh();
    }));

    World->GetTimerManager().SetTimer(DeferredRefreshTimerHandle, this, &UPX_InventoryWidget::HandleDeferredRefresh, PX_InventoryDeferredRefreshInterval, false);
}

void UPX_InventoryWidget::HandleDeferredRefresh()
{
    RefreshInventory();

    if ( !Inventory || !Inventory->IsInventoryReady() )
    {
        return;
    }

    if ( !HasPendingInventoryReplication() )
    {
        return;
    }

    ++DeferredRefreshAttemptCount;
    if ( DeferredRefreshAttemptCount >= PX_InventoryDeferredRefreshMaxAttempts )
    {
        return;
    }

    if ( UWorld* World = GetWorld() )
    {
        World->GetTimerManager().SetTimer(DeferredRefreshTimerHandle, this, &UPX_InventoryWidget::HandleDeferredRefresh, PX_InventoryDeferredRefreshInterval, false);
    }
}

bool UPX_InventoryWidget::HasPendingInventoryReplication() const
{
    if ( !Inventory )
    {
        return false;
    }

    const auto HasPendingSlot = [](const TArray<FPXInventorySlot>& Slots)
    {
        for ( const FPXInventorySlot& Slot : Slots )
        {
            if ( !Slot.ItemInstanceId.IsValid() )
            {
                continue;
            }

            if ( !Slot.ItemInstance || !Slot.ItemInstance->GetItemDataAsset() )
            {
                return true;
            }
        }

        return false;
    };

    return HasPendingSlot(Inventory->GetItemSlots()) || HasPendingSlot(Inventory->GetWeaponSlots());
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
