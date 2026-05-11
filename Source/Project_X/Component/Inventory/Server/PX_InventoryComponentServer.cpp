// Fill out your copyright notice in the Description page of Project Settings.

#include "../PX_InventoryComponent.h"

#include "../PX_WeaponItemInstance.h"
#include "Component/Inventory/PX_ItemInstance.h"
#include "Component/Inventory/PX_ItemDataAsset.h"
#include "Entity/PX_Item.h"
#include "AbilitySystem/Tags/PX_GameplayTags.h"

bool UPX_InventoryComponent::ServerCreateBareHandItemInstance_Validate()
{
	return true;
}

void UPX_InventoryComponent::ServerCreateBareHandItemInstance_Implementation()
{
	if ( !GetOwner() || !GetOwner()->HasAuthority() ) return;
    if ( !BareHandDataAsset ) return;

    if ( !WeaponSlots.Slots.IsValidIndex(4) ) return;
    if ( !WeaponSlots.Slots[4].IsEmpty() ) return;

    //UPX_WeaponItemInstance* BareHandInstance = NewObject<UPX_WeaponItemInstance>(this);
    //BareHandInstance->ServerInitializeFromData(BareHandDataAsset, 1);

    UPX_WeaponItemInstance* BareHandInstance = Cast<UPX_WeaponItemInstance>(BareHandDataAsset->CreateItemInstance(this, 1));
    if ( !BareHandInstance ) return;

    WeaponSlots.Slots[4].SlotIndex = 4;
    WeaponSlots.Slots[4].ItemInstance = BareHandInstance;
    WeaponSlots.Slots[4].ItemInstanceId = BareHandInstance->GetInstanceId();
    WeaponSlots.MarkItemDirty(WeaponSlots.Slots[4]);

    bInventoryReady = true;
    OnInventoryReady.Broadcast();
    //PX_LOG(Log, TEXT("BareHand Item Instance Create Complete. Inventory Ready"));
}

void UPX_InventoryComponent::InitializeTargetInventorySlots(FPXInventorySlotArray& TargetArray, int32 SlotSize)
{
    if ( !GetOwner() || !GetOwner()->HasAuthority() ) return;

    //PX_LOG(Log, TEXT(""));

    // 1) 배열 구조(Num) 맞추기
    const int32 OldSize = TargetArray.Slots.Num();
    const bool bSizeChanged = (OldSize != SlotSize);

    if ( bSizeChanged )
    {
        TargetArray.Slots.SetNum(SlotSize);
    }

    // 2) 각 슬롯 SlotIndex 세팅 + 기본 상태 보정
    //    - SlotIndex가 -1이면 UI에서 Slot -1 Added 같은 로그가 뜸
    //    - ItemInstance는 이미 값이 있을 수도 있으니, nullptr로 강제 초기화는 하지 않음
    bool bAnyItemDirty = false;

    for ( int32 i = 0; i < TargetArray.Slots.Num(); ++i )
    {
        FPXInventorySlot& Slot = TargetArray.Slots[i];

        if ( Slot.SlotIndex != i )
        {
            Slot.SlotIndex = i;
            // SlotIndex가 바뀌는 것도 "내용 변화"이므로 item dirty 처리
            TargetArray.MarkItemDirty(Slot);
            bAnyItemDirty = true;
        }

        // 필요하다면 여기서 "절대 비어있어야 하는 슬롯" 같은 정책도 적용 가능
        // (예: 무기 0번은 Barehand 예약이므로 나중에 따로 세팅)
    }

    // 3) 구조 변경이 있었다면 FastArray에 구조 변경을 알리기
    //    - 이게 없으면 OldMap size 경고가 뜸
    if ( bSizeChanged )
    {
        TargetArray.MarkArrayDirty();
    }

    PX_LOG(Log, TEXT("%s Initialize Complete with Size : %d"), *TargetArray.DebugName.ToString(), SlotSize);
}

/*
bool UPX_InventoryComponent::ServerAddItemInstance_Validate(UPX_ItemInstance* InItemInstance)
{
    return true;
}

void UPX_InventoryComponent::ServerAddItemInstance_Implementation(UPX_ItemInstance* InItemInstance)
{
    if ( !GetOwner() || !GetOwner()->HasAuthority() ) return;
    if ( !InItemInstance ) return;
    //PX_LOG(Log, TEXT(""));

    // 아이템 종류에 따라 WeapoinSlot에 들어갈지, ItemSlot에 들어갈지 결정해야 함. WeaponSlot이 모두 꽉 찼으면 ItemSlot에 들어가도록 해야 함.
    PX_LOG(Log, TEXT("AddItem: InPtr=%p InName=%s"), InItemInstance, *InItemInstance->GetName());
    UPX_ItemInstance* NewInstance = InItemInstance->ServerClone(this);
    PX_LOG(Log, TEXT("AddItem: NewPtr=%p NewName=%s"), NewInstance, *NewInstance->GetName());
    ServerAddToSlot(NewInstance);
}
*/

bool UPX_InventoryComponent::ServerAddItemFromData_Validate(FPX_ItemData InItemData)
{
    return true;
}

void UPX_InventoryComponent::ServerAddItemFromData_Implementation(FPX_ItemData InItemData)
{
    if ( !GetOwner() || !GetOwner()->HasAuthority() ) return;
    if ( !InItemData.ItemDataAsset ) return;

    UPX_ItemInstance* NewInstance = InItemData.ItemDataAsset->CreateItemInstance(this, 1, InItemData.InstanceId);
    if ( !NewInstance ) return;

    NewInstance->ApplyDropData(InItemData);
    
    PX_LOG(Log, TEXT("Add Item Name : %s, Id : %s"), *NewInstance->GetSafeName(), *NewInstance->GetInstanceId().ToString());

    AddToSlot(NewInstance);
}

bool UPX_InventoryComponent::AddWeaponItemFromDataToSlot(UPX_ItemDataAsset* InItemDataAsset, int32 SlotIndex)
{
    if ( !GetOwner() || !GetOwner()->HasAuthority() ) return false;
    if ( !InItemDataAsset ) return false;
    if ( !InItemDataAsset->IsWeaponItem() && !InItemDataAsset->PrefersWeaponInventory() ) return false;
    if ( !WeaponSlots.Slots.IsValidIndex(SlotIndex) ) return false;
    if ( !WeaponSlots.Slots[SlotIndex].IsEmpty() ) return true;

    UPX_ItemInstance* NewInstance = InItemDataAsset->CreateItemInstance(this, 1);
    if ( !NewInstance ) return false;

    return AddToSlotIndex(NewInstance, PX_GameplayTags::Item_Inventory_Weapon, SlotIndex);
}

/*
bool UPX_InventoryComponent::ServerDropItemInstance_Validate(int32 Index, FVector WorldLocation)
{
    return true;
}

void UPX_InventoryComponent::ServerDropItemInstance_Implementation(int32 Index, FVector WorldLocation)
{
    if ( !GetOwner() || !GetOwner()->HasAuthority() ) return;
    if ( !ItemSlots.Slots.IsValidIndex(Index) ) return;
    if ( ItemSlots.Slots[Index].IsEmpty() ) return;

    UPX_ItemInstance* Instance = ItemSlots.Slots[Index].ItemInstance;
    if ( !Instance ) return;

    UWorld* World = GetWorld();
    if ( !World ) return;

    // Spawn Dropped Item
    FActorSpawnParameters Params;
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
    Params.Owner = GetOwner();

    APX_Item* Dropped = World->SpawnActor<APX_Item>(APX_Item::StaticClass(), WorldLocation, FRotator::ZeroRotator, Params);
    if ( !Dropped ) return;

    //Dropped->ServerInitFromInstance(Instance);

    // Clear Inventory
    ItemSlots.Slots[Index].Clear();
    ItemSlots.MarkItemDirty(ItemSlots.Slots[Index]);
    //GetOwner()->ForceNetUpdate();              // (원하면)
}
*/

bool UPX_InventoryComponent::ServerDropItemData_Validate(int32 Index, FVector WorldLocation)
{
    return true;
}

void UPX_InventoryComponent::ServerDropItemData_Implementation(int32 Index, FVector WorldLocation)
{
    /*
    if ( !GetOwner() || !GetOwner()->HasAuthority() ) return;
    if ( !ItemSlots.Slots.IsValidIndex(Index) ) return;
    if ( ItemSlots.Slots[Index].IsEmpty() ) return;

    UPX_ItemInstance* Instance = ItemSlots.Slots[Index].ItemInstance;
    if ( !Instance ) return;

    UWorld* World = GetWorld();
    if ( !World ) return;

    // Spawn Dropped Item
    FActorSpawnParameters Params;
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
    Params.Owner = GetOwner();

    APX_Item* Dropped = World->SpawnActor<APX_Item>(APX_Item::StaticClass(), WorldLocation, FRotator::ZeroRotator, Params);
    if ( !Dropped ) return;

    //Dropped->ServerInitFromInstance(Instance);

    // Clear Inventory
    ItemSlots.Slots[Index].Clear();
    ItemSlots.MarkItemDirty(ItemSlots.Slots[Index]);
    //GetOwner()->ForceNetUpdate();              // (원하면)
    */
}

bool UPX_InventoryComponent::AddToSlot(UPX_ItemInstance* NewInstance)
{
    if ( !GetOwner() || !GetOwner()->HasAuthority() ) return false;
    if ( !NewInstance ) return false;

    UPX_ItemDataAsset* Data = NewInstance->GetItemDataAsset();
    if ( !Data ) return false;

    //PX_LOG(Log, TEXT(""));
    
    bool bStackable = Data->bStackable;
    int32 Remaining = FMath::Max(1, NewInstance->GetQuantity());

    // 나중에 컨테이너 추가되면 switch로 바꿔야 함
    //FPXInventorySlotArray& TargetArray = (Data->Kind == EPXItemKind::Weapon) ? WeaponSlots : ItemSlots;
    //const EPXInventorySlotTarget Target = (Data->Kind == EPXItemKind::Weapon) ? EPXInventorySlotTarget::Weapon : EPXInventorySlotTarget::Item;
    PX_LOG(Log, TEXT(""));
    const FGameplayTag PrimaryTargetTag = ResolveSlotTargetTag(Data);
    PX_LOG(Log, TEXT(""));
    FPXInventorySlotArray* TargetArray = FindInventoryArrayByTag(PrimaryTargetTag);
    PX_LOG(Log, TEXT(""));
    if ( !TargetArray ) return false;

    // 스택 불가능이면 항상 새 인스턴스를 빈 슬롯에 추가
    if ( !bStackable )
    {
        /*
        FPXInventorySlotSearchResult EmptySlot = FindFirstEmptySlot(Data->Kind);
        if ( !EmptySlot.IsValid() ) return false;

        return AddToSlotIndex(NewInstance, EmptySlot.Target, EmptySlot.SlotIndex);
        */
        FPXInventorySlotSearchResult EmptySlot = FindFirstEmptySlot(Data);
        if ( !EmptySlot.IsValid() ) return false;

        return AddToSlotIndex(NewInstance, EmptySlot.TargetTag, EmptySlot.SlotIndex);
    }

    // 스택 가능이면 기존 아이템의 스택 채우고, 남으면 새 인스턴스 생성
    // 기존 스택 채우기. 나중에 Hash로 바꿔야 함.
    const int32 MaxStack = FMath::Max(1, Data->MaxStack);
    for ( int32 i = 0; i < TargetArray->Slots.Num() && Remaining > 0; ++i )
    {
        FPXInventorySlot& Slot = TargetArray->Slots[i];
        UPX_ItemInstance* Existing = Slot.ItemInstance;
        if ( !Existing ) continue;

        if ( Existing->GetItemDataAsset() != Data ) continue;

        const int32 CurrentQuantity = Existing->GetQuantity();
        if ( CurrentQuantity >= MaxStack ) continue;

        const int32 CanAdd = MaxStack - CurrentQuantity;
        const int32 AddNow = FMath::Min(CanAdd, Remaining);

        Existing->SetQuantity(CurrentQuantity + AddNow);
        Remaining -= AddNow;

        TargetArray->MarkItemDirty(Slot);
        if ( AActor* OwnerActor = GetOwner() )
        {
            OwnerActor->ForceNetUpdate();
        }
    }

    // 남은 수량이 없으면 종료
    if ( Remaining <= 0 ) return true;

    // 기존 스택을 모두 채우고도 남은 첫 스택은 기존 Instance를 재사용
    {
        FPXInventorySlotSearchResult Empty = FindFirstEmptySlot(Data);
        if ( !Empty.IsValid() ) return false;

        const int32 FirstQty = FMath::Min(MaxStack, Remaining);
        NewInstance->SetQuantity(FirstQty);

        if ( !AddToSlotIndex(NewInstance, Empty.TargetTag, Empty.SlotIndex) ) return false;

        Remaining -= FirstQty;
    }

    // 그래도 수량이 남았으면 새로운 인스턴스를 생성
    while ( Remaining > 0 )
    {
        FPXInventorySlotSearchResult Empty = FindFirstEmptySlot(Data);
        if ( !Empty.IsValid() ) return false;

        const int32 MakeQuantity = FMath::Min(MaxStack, Remaining);

        UPX_ItemInstance* NewItemInstance = Data->CreateItemInstance(this, MakeQuantity);
        if ( !NewItemInstance ) return false;

        if ( !AddToSlotIndex(NewItemInstance, Empty.TargetTag, Empty.SlotIndex) ) return false;

        Remaining -= MakeQuantity;
    }

    return true;
}

//bool UPX_InventoryComponent::AddToSlotIndex(UPX_ItemInstance* NewInstance, EPXInventorySlotTarget Target,int32 InSlot)
bool UPX_InventoryComponent::AddToSlotIndex(UPX_ItemInstance* NewInstance, const FGameplayTag& TargetTag, int32 InSlot)
{
    //PX_LOG(Log, TEXT(""));
    if ( !GetOwner() || !GetOwner()->HasAuthority() ) return false;
    if ( !NewInstance ) return false;
    if ( InSlot == INDEX_NONE ) return false;

    // 나중에 컨테이너 추가되면 switch로 바꿔야 함
    //FPXInventorySlotArray& TargetArray = (Target == EPXInventorySlotTarget::Weapon) ? WeaponSlots : ItemSlots;
    FPXInventorySlotArray* TargetArray = FindInventoryArrayByTag(TargetTag);
    if ( !TargetArray ) return false;

    if ( !TargetArray->Slots.IsValidIndex(InSlot) ) return false;
    if ( !TargetArray->Slots[InSlot].IsEmpty() ) return false;

    //const bool bIsWeapon = (NewInstance->GetItemDataAsset()->Kind == EPXItemKind::Weapon);
    PX_LOG(Log, TEXT("Add to %sSlots[%d]"), *TargetTag.ToString(), InSlot);

    TargetArray->Slots[InSlot].SlotIndex = InSlot;
    TargetArray->Slots[InSlot].ItemInstance = NewInstance;
    TargetArray->Slots[InSlot].ItemInstanceId = NewInstance->GetInstanceId();
    TargetArray->MarkItemDirty(TargetArray->Slots[InSlot]);
    if ( AActor* OwnerActor = GetOwner() )
    {
        OwnerActor->ForceNetUpdate();
    }

    return true;
}
