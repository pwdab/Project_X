// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Project_X.h"
#include "Engine/DataAsset.h"
#include "PX_ItemDataAsset.generated.h"

class UPX_WeaponDataAsset;
class UPX_ItemInstance;

/**
 * 
 */

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

UCLASS()
class PROJECT_X_API UPX_ItemDataAsset : public UDataAsset
{
	GENERATED_BODY()
	
public:
    // PX_ItemInstance Factory Function
    UPX_ItemInstance* CreateItemInstance(UObject* Outer, int32 InQuantity, const FGuid& InInstanceId = FGuid());

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
    TSubclassOf<class UPX_ItemInstance> InstanceClass;

    // Data Variables
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
    EPXItemKind Kind = EPXItemKind::Generic;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
    FText DisplayName;

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

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item", meta = (EditCondition = "Kind == EPXItemKind::Weapon"))
    TObjectPtr<UPX_WeaponDataAsset> WeaponData = nullptr;

    // (선택) 메시 스케일/회전 오프셋 같은 것도 여기서
	
	
};
