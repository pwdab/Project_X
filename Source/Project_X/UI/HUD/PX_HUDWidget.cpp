// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/HUD/PX_HUDWidget.h"
#include "Entity/PX_Character.h"
#include "Component/Inventory/PX_InventoryComponent.h"
#include "Component/Inventory/PX_WeaponItemInstance.h"
#include "Component/Weapon/PX_WeaponSystemComponent.h"
#include "UI/HUD/PX_CurrentWeaponWidget.h"
#include "UI/HUD/PX_WeaponListsWidget.h"
#include "Component/Weapon/PX_WeaponDataAsset.h"


void UPX_HUDWidget::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    //InitHUD();

    if ( !BottomRightWidget ) return;

    CurrentWeaponWidget = Cast<UPX_CurrentWeaponWidget>(BottomRightWidget->GetWidgetFromName(TEXT("WBP_HUD_CurrentWeapon")));
    WeaponListsWidget = Cast<UPX_WeaponListsWidget>(BottomRightWidget->GetWidgetFromName(TEXT("WBP_HUD_WeaponLists")));
}

void UPX_HUDWidget::NativeDestruct()
{
    UnbindHUD();

    Super::NativeDestruct();
}

void UPX_HUDWidget::BindHUD(APawn* InPawn)
{
    //PX_LOG(Log, TEXT(""));
    if ( !InPawn ) return;

    // 중복 바인딩 처리
    if ( InPawn == OwnerPawn && bHUDBounded )
    {
        RefreshHUD();
        return;
    }

    // 기존 바인딩 해제
    UnbindHUD();

    // HUD 초기화
    OwnerPawn = InPawn;
    APX_Character* PX_Character = Cast<APX_Character>(OwnerPawn);
    if ( !PX_Character ) return;

    Inventory = PX_Character->GetInventoryComponent();
    WeaponSystem = PX_Character->GetWeaponSystemComponent();

    InitHUD();

    if ( !Inventory || !WeaponSystem )
    {
        OwnerPawn = nullptr;
        Inventory = nullptr;
        WeaponSystem = nullptr;
        bHUDBounded = false;
        return;
    }

    // 델리게이트 바인딩
    Inventory->OnInventoryReady.AddUObject(this, &UPX_HUDWidget::HandleInventoryReady);
    Inventory->OnInventorySlotUpdated.AddUObject(this, &UPX_HUDWidget::HandleWeaponSlotChanged);
    WeaponSystem->OnCurrentWeaponUpdated.AddUObject(this, &UPX_HUDWidget::HandleCurrentWeaponChanged);
    WeaponSystem->OnCurrentAmmoUpdated.AddUObject(this, &UPX_HUDWidget::HandleAmmoChanged);
    WeaponSystem->OnAttackModeUpdated.AddUObject(this, &UPX_HUDWidget::HandleAttackModeChanged);
    // WeaponSystem->OnWeaponSlotsChanged.AddUObject(this, &UPX_HUDWidget::HandleWeaponSlotsChanged);
    bHUDBounded = true;

    // InventoryReady 처리
    if ( Inventory->IsInventoryReady() ) HandleInventoryReady();
}

void UPX_HUDWidget::UnbindHUD()
{
    if ( Inventory && WeaponSystem && bHUDBounded )
    {
        // 델리게이트 해제

    }

    OwnerPawn = nullptr;
    Inventory = nullptr;
    WeaponSystem = nullptr;
    bHUDBounded = false;
}

void UPX_HUDWidget::RefreshHUD()
{
    if ( !bHUDBounded || !OwnerPawn || !Inventory || !WeaponSystem )
    {
        return;
    }

    // “전체 UI를 현재 상태로 다시 그리는” 용도 (초기 1회/복구용)
    // 실제 데이터 Pull은 네 컴포넌트 API에 맞춰 작성해야 함.

    // (예시) 슬롯 변경/무기리스트/탄약을 각각 다시 갱신하는 흐름
    //HandleWeaponSlotChanged();

    // 현재 슬롯 인덱스가 있다면 같이 갱신
    // TODO: WeaponSystem의 현재 슬롯 인덱스 getter로 수정
    // int32 CurrentSlot = WeaponSystem->GetCurrentWeaponSlotIndex();
    // HandleCurrentWeaponChanged(CurrentSlot);
}

void UPX_HUDWidget::InitHUD()
{
    if ( CurrentWeaponWidget ) CurrentWeaponWidget->ClearWidget();
    if ( WeaponListsWidget ) WeaponListsWidget->ClearWidgets();
}

void UPX_HUDWidget::HandleInventoryReady()
{
    // Inventory 준비 완료 시점에 한 번 더 갱신하면 안전
    RefreshHUD();
}

void UPX_HUDWidget::HandleCurrentWeaponChanged(int32 NewSlot)
{
    if ( !bHUDBounded || !CurrentWeaponWidget || !WeaponListsWidget ) return;

    FPXCurrentWeaponHUDData Data;
    if ( BuildCurrentWeaponHUDDataFromSlot(NewSlot, Data) )
    {
        CurrentWeaponWidget->UpdateWidget(Data);
    }
    else
    {
        CurrentWeaponWidget->ClearWidget();
    }

    WeaponListsWidget->HighlightWidgets(NewSlot);
}

void UPX_HUDWidget::HandleAmmoChanged(int32 InAmmoInMag, int32 InReserved)
{
    if ( !bHUDBounded || !CurrentWeaponWidget ) return;

    CurrentWeaponWidget->UpdateAmmo(InAmmoInMag, InReserved);
}

void UPX_HUDWidget::HandleAttackModeChanged(EPXWeaponAttackMode InAttackMode)
{
    //PX_LOG(Log, TEXT(""));
    if ( !bHUDBounded || !CurrentWeaponWidget ) return;

    CurrentWeaponWidget->UpdateAttackMode(FText::FromString(WeaponAttackModeToString(InAttackMode)));
}

void UPX_HUDWidget::HandleWeaponSlotChanged(EPXInventorySlotTarget Target, int32 SlotIndex, const FPXInventorySlot& InventorySlot)
{
    if ( Target != EPXInventorySlotTarget::Weapon ) return;
    if ( !bHUDBounded || !WeaponListsWidget ) return;

    UTexture2D* Texture = nullptr;
    if ( InventorySlot.ItemInstance )
    {
        if (UPX_ItemDataAsset* ItemData = InventorySlot.ItemInstance->GetItemDataAsset() )
        {
            Texture = ItemData->Icon;
        }
    }

    if ( Texture )
    {
        WeaponListsWidget->UpdateWidget(SlotIndex, Texture);
    }
    else
    {
        WeaponListsWidget->ClearWidget(SlotIndex);
    }
}

bool UPX_HUDWidget::BuildCurrentWeaponHUDDataFromSlot(int32 SlotIndex, FPXCurrentWeaponHUDData& OutData) const
{
    if ( SlotIndex == 4 ) return false; // BareHand

    //PX_LOG(Log, TEXT(""));
    OutData = FPXCurrentWeaponHUDData{};
    OutData.SlotIndex = SlotIndex;

    UPX_WeaponItemInstance* WeaponItemInstance = Cast<UPX_WeaponItemInstance>(Inventory->GetWeaponInstanceBySlot(SlotIndex));
    if ( !WeaponItemInstance ) return false;

    const UPX_ItemDataAsset* ItemData = WeaponItemInstance->GetItemDataAsset();
    if ( !ItemData ) return false;

    const UPX_WeaponDataAsset* WeaponData = ItemData->WeaponData;
    if ( !WeaponData ) return false;

    OutData.WeaponName = ItemData->DisplayName;
    OutData.WeaponIcon = ItemData->Icon;
    OutData.AmmoType = FText::FromString(AmmoTypeToString(WeaponData->AmmoType));
    OutData.AttackMode = FText::FromString(WeaponAttackModeToString(WeaponItemInstance->GetAttackMode()));
    OutData.AmmoInMag = WeaponItemInstance->GetAmmo();
    //OutData.Reserved = WeaponItemInstance->GetAmmoReserve();
    OutData.Reserved = 1000;

    return true;
}


