// Fill out your copyright notice in the Description page of Project Settings.

#include "../PX_InventoryComponent.h"
#include "Entity/PX_Item.h"

void UPX_InventoryComponent::CreateBareHandItemInstance()
{
	ServerCreateBareHandItemInstance();
}

/*
void UPX_InventoryComponent::AddItemInstance(UPX_ItemInstance* InItemInstance)
{
	if ( !GetOwner() || GetOwner()->HasAuthority() ) return;
	if ( !InItemInstance ) return;

	ServerAddItemInstance(InItemInstance);
}

void UPX_InventoryComponent::DropItemInstance(int32 Index, FVector WorldLocation)
{
	if ( !GetOwner() || GetOwner()->HasAuthority() ) return;

	ServerDropItemInstance(Index, WorldLocation);
}
*/

void UPX_InventoryComponent::AddItemFromData(FPX_ItemData InItemData)
{
	if ( !GetOwner() || GetOwner()->HasAuthority() ) return;
	if ( !InItemData.ItemDataAsset ) return;

	ServerAddItemFromData(InItemData);
}

void UPX_InventoryComponent::DropItemData(int32 Index, FVector WorldLocation)
{
	if ( !GetOwner() || GetOwner()->HasAuthority() ) return;

	ServerDropItemData(Index, WorldLocation);
}

/*
void UPX_InventoryComponent::OnRep_WeaponSlots()
{
	PX_LOG(Log, TEXT(""))
}

void UPX_InventoryComponent::OnRep_ItemSlots()
{

}
*/

void UPX_InventoryComponent::OnRep_InventoryReady()
{
	if ( bInventoryReady )
	{
		OnInventoryReady.Broadcast();
	}
}

static const TCHAR* SafeName(const UPX_ItemInstance* Item)
{
	return Item ? *Item->GetName() : TEXT("None");
}

// 개별 슬롯 변경 감지
void FPXInventorySlotArray::PostReplicatedChange(const TArrayView<int32>& ChangedIndices, int32 FinalSize)
{
	for ( int32 Index : ChangedIndices )
	{
		if ( Owner && Slots.IsValidIndex(Index) )
		{
			//PX_LOG(Log, TEXT(""));
			Owner->OnSlotUpdated(Index, Target);
			Owner->OnInventorySlotUpdated.Broadcast(Target, Index, Slots[Index]);
		}
	}
}

// 추가 감지
void FPXInventorySlotArray::PostReplicatedAdd(const TArrayView<int32>& AddedIndices, int32 FinalSize)
{
	for ( int32 Index : AddedIndices )
	{
		if ( Owner && Slots.IsValidIndex(Index) )
		{
			//PX_LOG(Log, TEXT(""));
			Owner->OnSlotAdded(Index, Target);
			Owner->OnInventorySlotUpdated.Broadcast(Target, Index, Slots[Index]);
		}
	}
}

// 삭제 감지
void FPXInventorySlotArray::PostReplicatedRemove(const TArrayView<int32>& RemovedIndices, int32 FinalSize)
{
	for ( int32 Index : RemovedIndices )
	{
		if ( Owner && Slots.IsValidIndex(Index) )
		{
			Owner->OnSlotRemoved(Index, Target);
			//Owner->OnInventorySlotsReset.Broadcast();
		}
	}
}

void UPX_InventoryComponent::OnSlotUpdated(int32 SlotIndex, EPXInventorySlotTarget Target)
{
	//PX_LOG(Log, TEXT("%s[%d] Updated"), *SlotName.ToString(), SlotIndex);
	
	FPXInventorySlotArray& TargetInventory = (Target == EPXInventorySlotTarget::Weapon) ? WeaponSlots : ItemSlots;

	TMap<int32, TObjectPtr<UPX_ItemInstance>>& PrevInventory = GetPrevInventoryState(Target);

	UPX_ItemInstance* OldItem = PrevInventory.Contains(SlotIndex) ? PrevInventory[SlotIndex] : nullptr;
	UPX_ItemInstance* NewItem = TargetInventory.Slots.IsValidIndex(SlotIndex) ? TargetInventory.Slots[SlotIndex].ItemInstance : nullptr;

	const FString OldName = OldItem ? OldItem->GetSafeName() : TEXT("None");
	const FString NewName = NewItem ? NewItem->GetSafeName() : TEXT("None");

	PX_LOG(Log, TEXT("%s[%d] Changed | Old: %s -> New: %s"), *TargetInventory.DebugName.ToString(), SlotIndex, *OldName, *NewName);

	PrevInventory.Add(SlotIndex, NewItem);
	// UI 갱신
}

void UPX_InventoryComponent::OnSlotAdded(int32 SlotIndex, EPXInventorySlotTarget Target)
{
	FPXInventorySlotArray& TargetInventory = (Target == EPXInventorySlotTarget::Weapon) ? WeaponSlots : ItemSlots;

	TMap<int32, TObjectPtr<UPX_ItemInstance>>& PrevInventory = GetPrevInventoryState(Target);

	UPX_ItemInstance* OldItem = PrevInventory.Contains(SlotIndex) ? PrevInventory[SlotIndex] : nullptr;
	UPX_ItemInstance* NewItem = TargetInventory.Slots.IsValidIndex(SlotIndex) ? TargetInventory.Slots[SlotIndex].ItemInstance : nullptr;

	const FString OldName = OldItem ? OldItem->GetSafeName() : TEXT("None");
	const FString NewName = NewItem ? NewItem->GetSafeName() : TEXT("None");

	PX_LOG(Log, TEXT("%s[%d] Add | Old: %s -> New: %s"), *TargetInventory.DebugName.ToString(), SlotIndex, *OldName, *NewName);

	PrevInventory.Add(SlotIndex, NewItem);
}

void UPX_InventoryComponent::OnSlotRemoved(int32 SlotIndex, EPXInventorySlotTarget Target)
{
	FPXInventorySlotArray& TargetInventory = (Target == EPXInventorySlotTarget::Weapon) ? WeaponSlots : ItemSlots;

	TMap<int32, TObjectPtr<UPX_ItemInstance>>& PrevInventory = GetPrevInventoryState(Target);

	UPX_ItemInstance* OldItem = PrevInventory.Contains(SlotIndex) ? PrevInventory[SlotIndex] : nullptr;

	const FString OldName = OldItem ? OldItem->GetSafeName() : TEXT("None");

	PX_LOG(Log, TEXT("%s[%d] Remove | Old: %s -> None"), *TargetInventory.DebugName.ToString(), SlotIndex, *OldName);

	PrevInventory.Remove(SlotIndex);
}