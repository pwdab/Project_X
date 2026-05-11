// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Project_X.h"
#include "UObject/Object.h"
#include "Component/Inventory/PX_ItemDataAsset.h"
#include "PX_ItemInstance.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(PX_ItemInstance, Log, All);

struct FPX_ItemData;
class UPX_EquippableItemDataAsset;
class UPX_WeaponDataAsset;

/**
 * 
 */
UCLASS()
class PROJECT_X_API UPX_ItemInstance : public UObject
{
	GENERATED_BODY()

	friend class UPX_ItemDataAsset;	// DataAsset에서 FGuid 생성

public:
	virtual void GetLifetimeReplicatedProps(TArray< class FLifetimeProperty>& OutLifetimeProps) const override;
	virtual bool IsSupportedForNetworking() const override { return true; }

	// --- Server Functions ---------------------------------------------------
	virtual FPX_ItemData MakeDropData() const;
	virtual void ApplyDropData(const FPX_ItemData& Data);
	void SetQuantity(const int32 InQuantity);

	// --- Getters -------------------------------------------------
	FORCEINLINE FGuid GetInstanceId() const { return InstanceId; }
	FORCEINLINE int32 GetQuantity() const { return Quantity; }
	FORCEINLINE UPX_ItemDataAsset* GetItemDataAsset() const { return ItemDataAsset.Get(); }
	//FORCEINLINE UPX_EquippableItemDataAsset* GetEquippableItemDataAsset() const { Cast<UPX_EquippableItemDataAsset>(ItemDataAsset); }
	UPX_EquippableItemDataAsset* GetEquippableItemDataAsset() const;
	//FORCEINLINE UPX_WeaponDataAsset* GetWeaponDataAsset() const { Cast<UPX_WeaponDataAsset>(ItemDataAsset); }
	UPX_WeaponDataAsset* GetWeaponDataAsset() const;
	FORCEINLINE bool IsWeaponItem() const { return ItemDataAsset && ItemDataAsset->IsWeaponItem(); }
	FORCEINLINE bool IsEquippableItem() const { return ItemDataAsset && ItemDataAsset->IsEquippableItem(); }
	FORCEINLINE FString GetSafeName() const { return (ItemDataAsset && !ItemDataAsset->ItemName.IsEmpty()) ? *ItemDataAsset->ItemName.ToString() : TEXT("None"); }

protected:
	// --- Server Functions ---------------------------------------------------
	virtual void InitializeFromData(UPX_ItemDataAsset* InDataAsset, int32 Quantity);

	// --- OnRep Functions -----------------------------------------------------
	UFUNCTION()
	virtual void OnRep_Quantity();

	// --- Replicated Variables -----------------------------------------------------
	// Item's Unique ID
	UPROPERTY(Replicated, EditAnywhere)
	FGuid InstanceId;
	// Item's Quantity
	UPROPERTY(ReplicatedUsing = OnRep_Quantity, EditAnywhere, BlueprintReadOnly, Category = "Item", meta = (AllowPrivateAccess = "true"))
	int32 Quantity = 1;
	// Item's Data Asset
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadOnly, Category = "Item", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UPX_ItemDataAsset> ItemDataAsset;

private:
	// --- Common Functions -----------------------------------------------------
	//bool HasServerAuthority() const;


};
