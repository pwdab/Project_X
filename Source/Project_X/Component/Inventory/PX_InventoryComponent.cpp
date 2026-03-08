// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/Inventory/PX_InventoryComponent.h"
#include "Net/UnrealNetwork.h"
#include "Engine/ActorChannel.h"
#include "Component/Weapon/PX_WeaponDataAsset.h"
#include "PX_ItemInstance.h"
#include "PX_ItemDataAsset.h"

DEFINE_LOG_CATEGORY(PX_InventoryComponent);

// Sets default values for this component's properties
UPX_InventoryComponent::UPX_InventoryComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);

	/*
	WeaponSlots.Owner = this;
	ItemSlots.Owner = this;

	WeaponSlots.DebugName = FName("Weapon Slot");
	ItemSlots.DebugName = FName("Item Slot");
	*/

	// ...
	/*
	if ( WeaponSlots.Slots.Num() != 5 )
	{
		WeaponSlots.Slots.SetNum(5);
	}

	if ( ItemSlots.Slots.Num() != 30 )
	{
		ItemSlots.Slots.SetNum(30);
	}
	*/

	static ConstructorHelpers::FObjectFinder<UPX_ItemDataAsset> BardHand(TEXT("/Game/Project_X/Character/Weapon/BareHand/Data/DA_PX_ItemData_BareHand.DA_PX_ItemData_BareHand"));
	//static ConstructorHelpers::FObjectFinder<UPX_ItemDataAsset> BardHand(TEXT("/Game/Project_X/Character/Weapon/Lyra/Rifle/Data/DA_PX_ItemData_Rifle.DA_PX_ItemData_Rifle"));
	if ( BardHand.Succeeded() )
	{
		BareHandDataAsset = BardHand.Object;
	}
}


// Called when the game starts
void UPX_InventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...

	WeaponSlots.Owner = this;
	ItemSlots.Owner = this;

	WeaponSlots.Target = EPXInventorySlotTarget::Weapon;
	ItemSlots.Target = EPXInventorySlotTarget::Item;

	WeaponSlots.DebugName = FName("Weapon Slot");
	ItemSlots.DebugName = FName("Item Slot");

	if ( GetOwner() && GetOwner()->HasAuthority() )
	{
		// WeaponSlots: 5칸 (0번 Barehand reserved)
		InitializeTargetInventorySlots(WeaponSlots, 5);

		// ItemSlots: 30칸
		InitializeTargetInventorySlots(ItemSlots, 30);

		//GetOwner()->ForceNetUpdate();

		ServerCreateBareHandItemInstance();
	}

	if ( GetOwner() && !GetOwner()->HasAuthority() )
	{
		//CreateBareHandItemInstance();
	}
}

void UPX_InventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UPX_InventoryComponent, WeaponSlots);
	DOREPLIFETIME(UPX_InventoryComponent, ItemSlots);
	DOREPLIFETIME(UPX_InventoryComponent, bInventoryReady);
	//DOREPLIFETIME(UPX_InventoryComponent, );
}


// Called every frame
void UPX_InventoryComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

bool UPX_InventoryComponent::ReplicateSubobjects(class UActorChannel* Channel, class FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
	bool bDirty = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);

	// 슬롯/인벤토리에 들어있는 UObject 인스턴스들을 직접 복제해줘야 함
	for ( const FPXInventorySlot& Slot : WeaponSlots.Slots )
	{
		const TObjectPtr<UPX_ItemInstance>& Instance = Slot.ItemInstance;
		if ( Instance )
		{
			bDirty |= Channel->ReplicateSubobject(Instance.Get(), *Bunch, *RepFlags);
		}
	}

	for ( const FPXInventorySlot& Slot : ItemSlots.Slots )
	{
		const TObjectPtr<UPX_ItemInstance>& Instance = Slot.ItemInstance;
		if ( Instance )
		{
			bDirty |= Channel->ReplicateSubobject(Instance.Get(), *Bunch, *RepFlags);
		}
	}

	return bDirty;
}

FPXInventorySlotSearchResult UPX_InventoryComponent::FindFirstEmptySlot(EPXItemKind InKind) const
{
	FPXInventorySlotSearchResult Result;

	if ( !GetOwner() || !GetOwner()->HasAuthority() ) return Result;

	if ( InKind == EPXItemKind::Weapon )
	{
		for ( int32 i = 0; i < WeaponSlots.Slots.Num(); ++i )
		{
			if ( WeaponSlots.Slots[i].IsEmpty() )
			{
				Result.Target = EPXInventorySlotTarget::Weapon;
				Result.SlotIndex = i;
				return Result;
			}
		}
	}

	for ( int32 i = 0; i < ItemSlots.Slots.Num(); ++i )
	{
		if ( ItemSlots.Slots[i].IsEmpty() )
		{
			Result.Target = EPXInventorySlotTarget::Item;
			Result.SlotIndex = i;
			return Result;
		}
	}
	return Result;
}

/*
int32 UPX_InventoryComponent::FindFirstEmptyItemSlot() const
{
	if ( !GetOwner() || !GetOwner()->HasAuthority() ) return INDEX_NONE;

	for ( int32 i = 0; i < ItemSlots.Slots.Num(); ++i )
	{
		if ( ItemSlots.Slots[i].IsEmpty() )
		{
			return i;
		}
	}
	return INDEX_NONE;
}

int32 UPX_InventoryComponent::FindFirstEmptyWeaponSlot() const
{
	if ( !GetOwner() || !GetOwner()->HasAuthority() ) return INDEX_NONE;

	// Index 4 is reserved for Barehand
	for ( int32 i = 0; i < WeaponSlots.Slots.Num(); ++i )
	{
		if ( WeaponSlots.Slots[i].IsEmpty() )
		{
			return i;
		}
	}
	return INDEX_NONE;
}
*/

TMap<int32, TObjectPtr<UPX_ItemInstance>>& UPX_InventoryComponent::GetPrevInventoryState(EPXInventorySlotTarget InTarget)
{
	// 나중에 인벤토리가 늘어나면 switch로 바꿔야 함.
	return (InTarget == EPXInventorySlotTarget::Weapon) ? PreviousWeaponSlotState : PreviousItemSlotState;
}