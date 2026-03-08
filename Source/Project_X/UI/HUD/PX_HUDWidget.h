// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Project_X.h"
#include "Blueprint/UserWidget.h"
#include "PX_HUDWidget.generated.h"

class UPX_InventoryComponent;
class UPX_WeaponSystemComponent;
class UPX_CurrentWeaponWidget;
class UPX_WeaponListsWidget;
class APawn;
enum class EPXInventorySlotTarget : uint8;
struct FPXInventorySlot;
struct FPXCurrentWeaponHUDData;
enum class EPXWeaponAttackMode : uint8;

/**
 * 
 */
UCLASS()
class PROJECT_X_API UPX_HUDWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
    // --- Blueprint Functions -----------------------------------------------------
    // Pawn을 주입하고 바인딩
    UFUNCTION(BlueprintCallable, Category = "PX_InventoryWidget")
    void BindHUD(APawn* InPawn);
    // HUD 바인딩 해제
    UFUNCTION(BlueprintCallable, Category = "PX_InventoryWidget")
    void UnbindHUD();
    // HUD 새로고침
    UFUNCTION(BlueprintCallable, Category = "PX_InventoryWidget")
    void RefreshHUD();

protected:
    virtual void NativeOnInitialized() override;
    virtual void NativeDestruct() override;

private:
    // HUD 초기화
    void InitHUD();

    // --- Handler Functions -----------------------------------------------------
    void HandleInventoryReady();
    void HandleCurrentWeaponChanged(int32 NewSlot);
    void HandleAmmoChanged(int32 InAmmoInMag, int32 InReserved);
    void HandleAttackModeChanged(EPXWeaponAttackMode InAttackMode);
    void HandleWeaponSlotChanged(EPXInventorySlotTarget Target, int32 SlotIndex, const FPXInventorySlot& InventorySlot);

    // --- Helper Functions -----------------------------------------------------
    bool BuildCurrentWeaponHUDDataFromSlot(int32 SlotIndex, FPXCurrentWeaponHUDData& OutData) const;

    // --- Child Widget Variables -----------------------------------------------------
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UUserWidget> BottomRightWidget;
    UPROPERTY()
    TObjectPtr<UPX_CurrentWeaponWidget> CurrentWeaponWidget;
    UPROPERTY()
    TObjectPtr<UPX_WeaponListsWidget> WeaponListsWidget;

    // --- Variables -----------------------------------------------------
    // 현재 바인딩된 Pawn
    UPROPERTY(Transient)
    TObjectPtr<APawn> OwnerPawn;
    // 현재 바인딩된 InventoryComponent
    UPROPERTY(Transient)
    TObjectPtr<UPX_InventoryComponent> Inventory;
    // 현재 바인딩된 WeaponSystemComponent
    UPROPERTY(Transient)
    TObjectPtr<UPX_WeaponSystemComponent> WeaponSystem;
    // 중복 바인딩 방지용
    UPROPERTY(Transient)
    bool bHUDBounded = false;


};
