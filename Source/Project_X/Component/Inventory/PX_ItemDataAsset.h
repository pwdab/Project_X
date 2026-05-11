// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Project_X.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "PX_ItemDataAsset.generated.h"

class UPX_ItemInstance;

/**
 * 
 */
/*  ItemTypeTag로 대체
UENUM(BlueprintType)
enum class EPXItemKind : uint8
{
    Generic         UMETA(DisplayName = "Generic"),
    Weapon          UMETA(DisplayName = "Weapon"),
    Ammo            UMETA(DisplayName = "Ammo"),
    Consumable      UMETA(DisplayName = "Consumable"),
    Material        UMETA(DisplayName = "Material"),
    Quest           UMETA(DisplayName = "Quest"),
};
*/

UCLASS()
class PROJECT_X_API UPX_ItemDataAsset : public UDataAsset
{
	GENERATED_BODY()
	
public:
    // --- Functions ---------------------------------------------------
    // PX_ItemInstance Factory Function
    UPX_ItemInstance* CreateItemInstance(UObject* Outer, int32 InQuantity, const FGuid& InInstanceId = FGuid());
    bool IsEquippableItem() const;
    bool IsWeaponItem() const;
    bool IsArmorItem() const;
    bool IsAccessoryItem() const;
    bool IsConsumableItem() const;

    bool PrefersWeaponInventory() const;
    bool PrefersArmorInventory() const;
    bool PrefersAccessoryInventory() const;
    bool IsGeneralSlotOnly() const;

    bool IsStackableItem() const;
    bool IsDroppableItem() const;
    bool IsExchangeableItem() const;

    // --- Data Variables ---------------------------------------------------
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
    TSubclassOf<class UPX_ItemInstance> InstanceClass;

    /* ItemTypeTag로 대체
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
    EPXItemKind Kind = EPXItemKind::Generic;
    */

    // 아이템 종류 태그
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
    FGameplayTag ItemCategoryTag;

    // 어떤 인벤토리에 들어가야 하는지
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
    FGameplayTag ItemInventoryTag;

    // 어떤 인벤토리에 들어가야 하는지
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
    FGameplayTagContainer ItemTypeTags;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
    FText ItemName;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
    TObjectPtr<UTexture2D> Icon = nullptr;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
    TObjectPtr<UTexture2D> Background = nullptr;

    // 월드에 배치될 때 보여줄 메시(간단 버전)
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
    TObjectPtr<UStaticMesh> WorldStaticMesh = nullptr;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
    bool bStackable = true;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item", meta = (ClampMin = "1"))
    int32 MaxStack = 1000;	
	
};
