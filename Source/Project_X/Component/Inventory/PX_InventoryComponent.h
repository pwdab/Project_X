// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Project_X.h"
#include "Components/ActorComponent.h"
#include "Component/Inventory/PX_ItemInstance.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "PX_InventoryComponent.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(PX_InventoryComponent, Log, All);

DECLARE_MULTICAST_DELEGATE(FPXOnInventoryReady);
// 어떤 슬롯 배열이 바뀌었는지 구분: Target(Weapon/Item) + SlotIndex + SlotData
DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnInventorySlotUpdated, FGameplayTag /*TargetTag*/, int32 /*SlotIndex*/, const FPXInventorySlot& /*Slot*/);
//DECLARE_MULTICAST_DELEGATE_OneParam(FOnInventorySlotsReset,EPXInventorySlotTarget /*Target*/);

class UPX_ItemDataAsset;
class UPX_WeaponItemInstance;

/* TargetTag로 대체
// 아이템을 추가할 타겟 컨테이너
UENUM(BlueprintType)
enum class EPXInventorySlotTarget : uint8
{
	None,
	Weapon,
	Item
};
*/

// 아이템 개별 슬롯 하나
USTRUCT(BlueprintType)
struct FPXInventorySlot : public FFastArraySerializerItem
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	int32 SlotIndex = INDEX_NONE;

	//UPROPERTY(EditAnywhere, Instanced)
	UPROPERTY(EditAnywhere)
	TObjectPtr<UPX_ItemInstance> ItemInstance = nullptr;

	UPROPERTY(VisibleAnywhere)
	FGuid ItemInstanceId = FGuid();

	bool IsEmpty() const { return ItemInstance == nullptr; }
	void Clear() { ItemInstance = nullptr; }
};

// 아이템 슬롯 배열
USTRUCT()
struct FPXInventorySlotArray : public FFastArraySerializer
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	TArray<FPXInventorySlot> Slots;

	// 역참조용 (UI 호출 등)
	UPROPERTY(NotReplicated)
	class UPX_InventoryComponent* Owner = nullptr;

	// 이 배열이 어떤 Slot 배열인지
	/* TargetTag로 대체
	UPROPERTY(NotReplicated)
	EPXInventorySlotTarget Target = EPXInventorySlotTarget::None;
	*/ 
	UPROPERTY(NotReplicated)
	FGameplayTag TargetTag;

	// Debug String
	UPROPERTY(NotReplicated)
	FName DebugName;

	// Unreal에 FastArray로 등록
	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParams) { return FFastArraySerializer::FastArrayDeltaSerialize<FPXInventorySlot, FPXInventorySlotArray>(Slots, DeltaParams, *this); }

	// 개별 슬롯 변경 감지
	void PostReplicatedChange(const TArrayView<int32>& ChangedIndices, int32 FinalSize);
	// 추가 감지
	void PostReplicatedAdd(const TArrayView<int32>& AddedIndices, int32 FinalSize);
	// 삭제 감지
	void PostReplicatedRemove(const TArrayView<int32>& RemovedIndices, int32 FinalSize);
};

// 검색 결과. 어떤 인벤토리의 몇 번째 인덱스인지
USTRUCT()
struct FPXInventorySlotSearchResult
{
	GENERATED_BODY()

public:
	/* TargetTag로 대체
	UPROPERTY()
	EPXInventorySlotTarget Target = EPXInventorySlotTarget::None;
	*/
	UPROPERTY()
	FGameplayTag TargetTag;

	UPROPERTY()
	int32 SlotIndex = INDEX_NONE;

	bool IsValid() const
	{
		//return Target != EPXInventorySlotTarget::None && SlotIndex != INDEX_NONE;
		return TargetTag.IsValid() && SlotIndex != INDEX_NONE;
	}
};

// Unreal에게 이 Struct는 NetDeltaSerialize를 사용한다고 알림
template<>
struct TStructOpsTypeTraits<FPXInventorySlotArray> : public TStructOpsTypeTraitsBase2<FPXInventorySlotArray>
{
	enum
	{
		WithNetDeltaSerializer = true,
	};
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECT_X_API UPX_InventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UPX_InventoryComponent();

	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual bool ReplicateSubobjects(class UActorChannel* Channel, class FOutBunch* Bunch, FReplicationFlags* RepFlags) override;

	// --- Server RPCs -----------------------------------------------------
	/*
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerAddItemInstance(UPX_ItemInstance* InItemInstance);
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerDropItemInstance(int32 Index, FVector WorldLocation);
	*/
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerAddItemFromData(FPX_ItemData InItemData);
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerDropItemData(int32 Index, FVector WorldLocation);

	// --- Client Functions -----------------------------------------------------
	/* TargetTag로 대체
	void OnSlotUpdated(int32 SlotIndex, EPXInventorySlotTarget Target);
	void OnSlotAdded(int32 SlotIndex, EPXInventorySlotTarget Target);
	void OnSlotRemoved(int32 SlotIndex, EPXInventorySlotTarget Target);
	*/
	void OnSlotUpdated(int32 SlotIndex, FGameplayTag TargetTag);
	void OnSlotAdded(int32 SlotIndex, FGameplayTag TargetTag);
	void OnSlotRemoved(int32 SlotIndex, FGameplayTag TargetTag);

	// --- Common Functions -----------------------------------------------------

	// --- Getter Functions -----------------------------------------------------
	FORCEINLINE bool IsInventoryReady() const { return bInventoryReady; }
	FORCEINLINE const TArray<FPXInventorySlot>& GetItemSlots() const { return ItemSlots.Slots; }
	FORCEINLINE const TArray<FPXInventorySlot>& GetWeaponSlots() const { return WeaponSlots.Slots; }
	FORCEINLINE UPX_WeaponItemInstance* GetWeaponItemInstanceBySlot(int32 SlotIndex) const;
	FORCEINLINE UPX_ItemInstance* GetItemInstanceBySlot(int32 SlotIndex) const { return ItemSlots.Slots.IsValidIndex(SlotIndex) ? ItemSlots.Slots[SlotIndex].ItemInstance : nullptr; }
	bool AddWeaponItemFromDataToSlot(UPX_ItemDataAsset* InItemDataAsset, int32 SlotIndex);

	// --- Delegate Variables -----------------------------------------------------
	FPXOnInventoryReady OnInventoryReady;
	FOnInventorySlotUpdated OnInventorySlotUpdated;
	//FOnInventorySlotsReset OnInventorySlotsReset;

private:
	// --- Server RPCs -----------------------------------------------------
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerCreateBareHandItemInstance();

	// --- OnRep Functions -----------------------------------------------------
	/*
	UFUNCTION()
	void OnRep_WeaponSlots();
	UFUNCTION()
	void OnRep_ItemSlots();
	*/
	UFUNCTION()
	void OnRep_InventoryReady();

	// --- Common Functions -----------------------------------------------------
	// FPXInventorySlotSearchResult FindFirstEmptySlot(EPXItemKind InKind) const;
	FPXInventorySlotSearchResult FindFirstEmptySlot(const UPX_ItemDataAsset* InItemDataAsset) const;
	FGameplayTag ResolveSlotTargetTag(const UPX_ItemDataAsset* InItemDataAsset) const;
	FPXInventorySlotArray* FindInventoryArrayByTag(const FGameplayTag& InTargetTag);
	const FPXInventorySlotArray* FindInventoryArrayByTag(const FGameplayTag& InTargetTag) const;
	//int32 FindFirstEmptyItemSlot() const;
	//int32 FindFirstEmptyWeaponSlot() const;
	//TMap<int32, TObjectPtr<UPX_ItemInstance>>& GetPrevInventoryState(EPXInventorySlotTarget InTarget);
	TMap<int32, TObjectPtr<UPX_ItemInstance>>& GetPrevInventoryState(const FGameplayTag& InTargetTag);
	const FName& GetInventoryDebugName(const FGameplayTag& InTargetTag) const;
	
	// --- Server Functions -----------------------------------------------------
	void InitializeTargetInventorySlots(FPXInventorySlotArray& TargetArray, int32 SlotSize);
	bool AddToSlot(UPX_ItemInstance* NewInstance);
	//bool AddToSlotIndex(UPX_ItemInstance* NewInstance, EPXInventorySlotTarget Target, int32 InSlot);
	bool AddToSlotIndex(UPX_ItemInstance* NewInstance, const FGameplayTag& TargetTag, int32 InSlot);

	// --- Client Functions -----------------------------------------------------
	void CreateBareHandItemInstance();
	//void AddItemInstance(UPX_ItemInstance* InItemInstance);
	//void DropItemInstance(int32 Index, FVector WorldLocation);
	void AddItemFromData(FPX_ItemData InItemData);
	void DropItemData(int32 Index, FVector WorldLocation);
	
	// --- Replicated Variables ---------------------------------------------------
	// Weapon Inventory. Size : 5, Slot 4 is reserved for barehand Only.
	UPROPERTY(Replicated, EditAnywhere, Category = "Inventory")
	FPXInventorySlotArray WeaponSlots;
	// Item Inventory. Size : 30
	UPROPERTY(Replicated, EditAnywhere, Category = "Inventory")
	FPXInventorySlotArray ItemSlots;
	UPROPERTY(ReplicatedUsing = OnRep_InventoryReady, EditAnywhere, Category = "Inventory")
	bool bInventoryReady = false;

	// --- Common Variables ---------------------------------------------------
	// BareHand Data Asset
	TObjectPtr<UPX_ItemDataAsset> BareHandDataAsset;
	// For debugging and tracing previous Inventory
	TMap<int32, TObjectPtr<UPX_ItemInstance>> PreviousWeaponSlotState;
	TMap<int32, TObjectPtr<UPX_ItemInstance>> PreviousItemSlotState;
	
	// --- Client Variables ---------------------------------------------------
	
};
