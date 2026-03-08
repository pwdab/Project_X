// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Project_X.h"
#include "Blueprint/UserWidget.h"
#include "Component/Inventory/PX_InventoryComponent.h"
#include "PX_InventorySlotWidget.generated.h"

//enum class EPXInventorySlotTarget : uint8;
//struct FPXInventorySlot;
class UPX_ItemInstance;

/**
 * 
 */
UCLASS()
class PROJECT_X_API UPX_InventorySlotWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
    UFUNCTION(BlueprintCallable, Category = "PX|Inventory|Slot")
    void SetSlotMeta(EPXInventorySlotTarget InTarget, int32 InSlotIndex);

    UFUNCTION(BlueprintCallable, Category = "PX|Inventory|Slot")
    void UpdateFromSlot(const FPXInventorySlot& InSlot);

    UFUNCTION(BlueprintCallable, Category = "PX|Inventory|Slot")
    void ClearSlot();

    UFUNCTION(BlueprintPure, Category = "PX|Inventory|Slot")
    FORCEINLINE bool IsEmpty() const { return ItemInstance == nullptr; }

    UFUNCTION(BlueprintPure, Category = "PX|Inventory|Slot")
    FORCEINLINE int32 GetSlotIndex() const { return SlotIndex; }

    UFUNCTION(BlueprintPure, Category = "PX|Inventory|Slot")
    FORCEINLINE EPXInventorySlotTarget GetTarget() const { return Target; }

    UFUNCTION(BlueprintPure, Category = "PX|Inventory|Slot")
    FORCEINLINE UPX_ItemInstance* GetItemInstance() const { return ItemInstance; }

protected:
    UFUNCTION(BlueprintImplementableEvent, Category = "PX|Inventory|Slot")
    void BP_UpdateSlot(UPX_ItemInstance* InItemInstance);

    UFUNCTION(BlueprintImplementableEvent, Category = "PX|Inventory|Slot")
    void BP_ClearSlot();

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PX|Inventory|Slot")
    EPXInventorySlotTarget Target = EPXInventorySlotTarget::None;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PX|Inventory|Slot")
    int32 SlotIndex = INDEX_NONE;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PX|Inventory|Slot")
    TObjectPtr<UPX_ItemInstance> ItemInstance = nullptr;
	
	
};
