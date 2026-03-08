// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Project_X.h"
#include "PX_WeaponTypes.h"
#include "PX_WeaponSystemComponent.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(PX_WeaponSystemComponent, Log, All);

class APX_Character;
class APX_Weapon;
class UPX_WeaponItemInstance;
class UPX_WeaponDataAsset;
class UPX_WeaponActionHandler;
class UPX_WeaponAnimDriver;
class UPX_InventoryComponent;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnCurrentWeaponUpdated, int32/*CurrentWeaponSlotIndex*/);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnCurrentAmmoUpdated, int32/*MagSize*/, int32/*AmmoInMag*/);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnAttackModeUpdated, EPXWeaponAttackMode/*AttackMode*/);

UCLASS(/*ClassGroup = (Custom), meta = (BlueprintSpawnableComponent)*/)
class PROJECT_X_API UPX_WeaponSystemComponent : public UActorComponent
{
    GENERATED_BODY()
    friend class UPX_WeaponCommitSubsystem;

public:
    UPX_WeaponSystemComponent();

    /*
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Debug Only
    UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "WeaponSystem", meta = (AllowPrivateAccess = "true"))
    TArray<TObjectPtr<APX_Weapon>> Weapons;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "WeaponSystem", meta = (AllowPrivateAccess = "true"))
    TArray<TObjectPtr<UPX_WeaponDataAsset>> WeaponSlots;
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    */

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    virtual void TickComponent(float DeltaSeconds, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
    //void Server_TickComponent(float DeltaSeconds, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction);
    //void Client_TickComponent(float DeltaSeconds, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction);

public:
    // --- Weapon -------------------------------------------------
    /** Equip weapon described by the DataAsset. */
    //void EquipWeapon(APX_Weapon* InWeapon);
    // 나중에 무기 스폰을 GameState로 옮겨야 함
    //void EquipWeapon(UPX_WeaponDataAsset* NewWeaponData);
    /** Equip weapon described by the Slot Index. */
    void EquipWeaponBySlot(int32 SlotIndex);
    
    // --- Actions -------------------------------------------------
    /** Submit Actions at Client. */
    void ClientSubmitAction(FPXWeaponActionContext ActionContext);

    // --- Getters -------------------------------------------------
    FORCEINLINE ACharacter* GetCharacter() const { return Character.Get(); }
    FORCEINLINE APX_Weapon* GetWeapon() const { return CurrentWeapon.Get(); }
    FORCEINLINE UPX_WeaponItemInstance* GetWeaponInstance() const { return CurrentWeaponItemInstance.Get(); }
    
    // --- Delegate Variables -----------------------------------------------------
    FOnCurrentWeaponUpdated OnCurrentWeaponUpdated;
    FOnCurrentAmmoUpdated OnCurrentAmmoUpdated;
    FDelegateHandle AmmoUpdatedHandle;
    FOnAttackModeUpdated OnAttackModeUpdated;
    FDelegateHandle AttackModeUpdatedHandle;

private:
    // --- Server RPCs -----------------------------------------------------
    // --- Weapon -------------------------------------------------
    /*
    UFUNCTION(Server, Reliable, WithValidation)
    void ServerEquipWeapon(APX_Weapon* InWeapon);
    */
    UFUNCTION(Server, Reliable, WithValidation)
    void ServerEquipBySlot(int32 SlotIndex);
    /*
    // 나중에 무기 스폰을 GameState로 옮겨야 함
    UFUNCTION(Server, Reliable, WithValidation)
    void ServerEquipWeaponWithData(UPX_WeaponDataAsset* NewWeaponData);
    */
    UFUNCTION(Server, Reliable, WithValidation)
    // 나중에 무기 스폰을 GameState로 옮겨야 함
    void ServerDestroyWeapon(APX_Weapon* InWeapon);
    // --- Actions -------------------------------------------------
    UFUNCTION(Server, Reliable, WithValidation)
    void ServerSubmitAction(const FPXWeaponActionContext& ActionContext);

    // --- Client RPCs -----------------------------------------------------
    // --- Actions -------------------------------------------------
    UFUNCTION(Client, Reliable)
    void ClientCommitAction(const FPXWeaponActionContext& ActionContext, bool bActionApproved);

    // --- Multicast RPCs -----------------------------------------------------
    // --- Actions -------------------------------------------------
    UFUNCTION(NetMulticast, Reliable)
    void MulticastCommitAction(const FPXWeaponActionContext& ActionContext);

    // --- OnRep Functions -----------------------------------------------------
    /*
    UFUNCTION()
    void OnRep_WeaponUpdated();
    UFUNCTION()
    void OnRep_WeaponDataUpdated();
    */
    UFUNCTION()
    void OnRep_WeaponSlotUpdated();
    UFUNCTION()
    void OnRep_WeaponInstanceIdUpdated();
    UFUNCTION()
    void OnRep_WeaponUpdated();

    // --- Common Functions -----------------------------------------------------
    //void SpawnWeapons();
    //const UObject* GetKeyFromContext(const FPXWeaponActionContext& ActionContext) const;

    // --- Server Functions -----------------------------------------------------
    /** Commit Actions at Server by Fixed Step. */
    void ServerCommitAction(float DeltaTime);
    /** Update Server State based on Commited Actions by Fixed Step. */
    bool ServerUpdateState(const FPXWeaponActionContext& ActionContext);
    /** Try Generate AttackOnce Action based on Updated Server State. */
    void ServerTryAttackOnce();
    /** Check if AttackOnce Action is Approved. Generate Deny Reason if Denied. */
    bool ServerAttackOnceApproved(FString* OutReason) const;

    // --- Server Functions -----------------------------------------------------
    /** Update Client State based on Commited Actions by Client Submit. */
    bool ClientUpdateState(const FPXWeaponActionContext& ActionContext);

    // --- Client Functions -----------------------------------------------------
    UFUNCTION()
    void HandleInventoryReady();
    UFUNCTION()
    void HandleAmmoUpdated(int32 InAmmoInMag, int32 InReserved);
    UFUNCTION()
    void HandleAttackModeUpdated(EPXWeaponAttackMode InAttackMode);

    void ClientBuildCosmetics();
    void TryResolveAndBindCurrentWeaponInstance();
    void UnbindFromCurrentWeaponInstance();

    

    // --- Replicated Variables ---------------------------------------------------
    // --- Weapon ---------------------------------------------------
    // Current weapon index.
    UPROPERTY(ReplicatedUsing = OnRep_WeaponSlotUpdated)
    int32 CurrentWeaponSlotIndex = INDEX_NONE;
    // Current weapon index.
    UPROPERTY(ReplicatedUsing = OnRep_WeaponInstanceIdUpdated)
    FGuid CurrentWeaponInstanceId = FGuid();
    // Current weapon actor.
    UPROPERTY(ReplicatedUsing = OnRep_WeaponUpdated)
    TObjectPtr<APX_Weapon> CurrentWeapon;
    /*
    // Current weapon's data.
    UPROPERTY(ReplicatedUsing = OnRep_WeaponDataUpdated)
    TObjectPtr<UPX_WeaponDataAsset> WeaponData;
    */

    // --- Common Variables ---------------------------------------------------
    /** Current weapon's Character. */
    UPROPERTY()
    TObjectPtr<ACharacter> Character = nullptr;
    // Current Character's Inventory. 
    UPROPERTY()
    TObjectPtr<UPX_InventoryComponent> Inventory = nullptr;
    // Current weapon item instance (Non-replicated).
    UPROPERTY(Transient, EditAnywhere, Category = "WeaponSystem")
    TObjectPtr<UPX_WeaponItemInstance> CurrentWeaponItemInstance;
    // Cached Spawned-Weapon Container.
    UPROPERTY()
    TMap<int32, TObjectPtr<APX_Weapon>> SpawnedWeapons;

    // --- Server Variables ---------------------------------------------------
    // --- Weapon ---------------------------------------------------
    // AttackPressed이면 AttackReleased가 되기 전까지 Attack을 지속
    bool bAttackPressed_Server = false;
    // 점사 모드의 한 번의 발사에서 남은 탄 수
    int32 BurstRemaining_Server = 0;
    // Interval을 고려한 다음 Attack 시간
    float NextAttackTime_Server = 0.f;
    // 서버 상태 변수
    //bool bActionLocked = false;
    bool bIsReloading_Server = false;
    bool bIsEquipping_Server = false;
    // --- Actions ---------------------------------------------------
    // 이 컴포넌트를 가지고 있는 클라이언트에 대해 기대하는 다음 시퀀스 번호와 처리 대기 중인 액션 버퍼를 감싼 구조체
    struct FSubmittedBuffer
    {
        // 서버가 지금까지 처리한 클라이언트의 시퀀스 번호
        uint32 ExpectedSequence = 0;
        // 클라이언트가 보낸 액션 중 아직 처리되지 않은 것들
        TMap<uint32, FPXWeaponActionContext> SubmittedBuffer;
    } Buffer;
    /*
    // 서버가 모든 입력 주체(Character, AI)의 버퍼를 관리하는 테이블
    TMap<const UObject*, FSubmittedBuffer> PendingActions;
    */
    // 서버의 한 Step 마다 하나의 WeaponSystemComponent에서 처리하게 할 Action의 개수
    const uint32 MaxActionsPerStep = 1;
    // 서버가 액션을 생성할 때 붙이는 단조 증가 번호
    uint32 ServerSequence = 0;
    // 다른 액션의 재생을 막는 액션의 재생 시작 시간
    float BlockingActionTime_Server = 0.f;

    
    
    // --- Client Variables ---------------------------------------------------
    // --- Weapon ---------------------------------------------------
    // AttackPressed이면 AttackReleased가 되기 전까지 Attack을 지속
    bool bAttackPressed_Local = false;
    // 점사 모드의 한 번의 발사에서 남은 탄 수
    int32 BurstRemaining_Local = 0;
    // Interval을 고려한 다음 Attack 시간
    float NextAttackTime_Local = 0.f;
    // --- Actions ---------------------------------------------------
    // 클라이언트가 액션을 생성할 때 붙이는 단조 증가 번호
    uint32 ClientSequence = 0;
    /** Current weapon's action handler. */
    UPROPERTY(Transient, EditAnywhere, Category = "WeaponSystem")
    TObjectPtr<UPX_WeaponActionHandler> CurrentActionHandler = nullptr;
    /** Current weapon's anim driver. */
    UPROPERTY(Transient, EditAnywhere, Category = "WeaponSystem")
    TObjectPtr<UPX_WeaponAnimDriver> CurrentAnimDriver = nullptr;
    // 클라이언트 상태 변수
    bool bIsReloading_Local = false;
    bool bIsEquipping_Equip = false;
    // BeginEquip에서 ClientBuildCosmetics()가 완료된 이후에 실행할 명령을 저장하는 임시 버퍼
    TOptional<FPXWeaponActionContext> PendingCosmeticAction;
};
