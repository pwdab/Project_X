// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Project_X.h"
#include "GameplayTagContainer.h"
#include "AbilitySystem/Tags/PX_GameplayTags.h"
#include "Blueprint/UserWidget.h"
#include "TimerManager.h"
#include "PX_InventoryWidget.generated.h"

/* TargetTag로 대체
enum class EPXInventorySlotTarget : uint8;
*/
struct FPXInventorySlot;
class UPX_InventoryItemSlotsWidget;
class UPX_InventoryWeaponSlotsWidget;
class UPX_InventoryComponent;

/**
 * 
 */
UCLASS()
class PROJECT_X_API UPX_InventoryWidget : public UUserWidget
{
	GENERATED_BODY()

public:
    // --- Blueprint Functions -----------------------------------------------------
    // 인벤토리 컴포넌트를 주입하고 바인딩
    UFUNCTION(BlueprintCallable, Category = "PX_InventoryWidget")
    void BindInventory(UPX_InventoryComponent* InInventory);
    // 인벤토리 바인딩 해제
    UFUNCTION(BlueprintCallable, Category = "PX_InventoryWidget")
    void UnbindInventory();
    // 인벤토리 새로고침
    UFUNCTION(BlueprintCallable, Category = "PX_InventoryWidget")
    void RefreshInventory();
    
protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeDestruct() override;

private:
    // 인벤토리 초기화
    void InitInventory();
    void RequestDeferredRefresh();
    void HandleDeferredRefresh();
    bool HasPendingInventoryReplication() const;

    // --- Inventory Component Function Handlers -----------------------------------------------------
    void HandleInventoryReady();
	/* TargetTag로 대체
    void HandleSlotUpdated(EPXInventorySlotTarget Target, int32 SlotIndex, const FPXInventorySlot& InventorySlot);
    */
    void HandleSlotUpdated(FGameplayTag Target, int32 SlotIndex, const FPXInventorySlot& InventorySlot);
    //void HandleSlotsReset(EPXInventorySlotTarget Target);

    // --- Child Widget Variables -----------------------------------------------------
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UPX_InventoryItemSlotsWidget> ItemSlotsWidget;
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UPX_InventoryWeaponSlotsWidget> WeaponSlotsWidget;
    // --- Variables -----------------------------------------------------
    // 현재 바인딩된 InventoryComponent
    UPROPERTY(Transient)
    TObjectPtr<UPX_InventoryComponent> Inventory;
    // 중복 바인딩 방지용
    UPROPERTY(Transient)
    bool bInventoryBounded = false;

    FTimerHandle DeferredRefreshTimerHandle;
    int32 DeferredRefreshAttemptCount = 0;
    

};
