// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Project_X.h"
#include "GameplayTagContainer.h"
#include "Abilities/GameplayAbilityTargetTypes.h"
#include "PX_WeaponSystemComponent.generated.h"

class ACharacter;
class APX_Weapon;
class UPX_WeaponItemInstance;
class UPX_InventoryComponent;
class UGameplayEffect;
struct FGameplayTag;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnCurrentWeaponUpdated, int32/*CurrentWeaponSlotIndex*/);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnCurrentAmmoUpdated, int32 /*AmmoInMag*/, int32 /*ReservedAmmo*/);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnAttackModeTagUpdated, FGameplayTag /*AttackMode*/);

UCLASS()
class PROJECT_X_API UPX_WeaponSystemComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UPX_WeaponSystemComponent();

protected:
    virtual void BeginPlay() override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    virtual void TickComponent(float DeltaSeconds, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
    //void Server_TickComponent(float DeltaSeconds, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction);
    //void Client_TickComponent(float DeltaSeconds, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction);

public:
    // --- Getters -------------------------------------------------
    FORCEINLINE ACharacter* GetCharacter() const { return Character.Get(); }
    FORCEINLINE APX_Weapon* GetWeapon() const { return CurrentWeapon.Get(); }
    FORCEINLINE int32 GetWeaponSlotIndex() const { return CurrentWeaponSlotIndex; }
    FORCEINLINE UPX_WeaponItemInstance* GetWeaponInstance() const { return CurrentWeaponItemInstance.Get(); }
	FORCEINLINE FGuid GetWeaponItemInstanceId() const { return CurrentWeaponItemInstanceId; }
    int32 GetCurrentAmmoInMag() const;
    FGameplayTag GetCurrentAttackModeTag() const;
    void QueueNextAttackStatusEffect(TSubclassOf<UGameplayEffect> StatusEffectClass);
    TArray<TSubclassOf<UGameplayEffect>> ConsumeNextAttackStatusEffects();

    void SetIsEquipCancelable(bool InValue);
    
    // --- Delegate Variables -----------------------------------------------------
    FOnCurrentWeaponUpdated OnCurrentWeaponUpdated;
    FOnCurrentAmmoUpdated OnCurrentAmmoUpdated;
    FOnAttackModeTagUpdated OnAttackModeTagUpdated;

    FDelegateHandle AmmoUpdatedHandle;
    FDelegateHandle AttackModeUpdatedHandle;

public:
    // --- Client RPCs -----------------------------------------------------
    // --- Equip -------------------------------------------------
    UFUNCTION(Client, Reliable)
    void Client_CancelFireOnce();

    // --- Multicast RPCs -----------------------------------------------------
    // --- Equip -------------------------------------------------
    UFUNCTION(NetMulticast, Reliable)
    void Multicast_EquipBySlot(int32 SlotIndex);
    UFUNCTION(NetMulticast, Reliable)
    void Multicast_EquipBySlotEnd();
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_CancelEquipBySlot();
    // --- Reload -------------------------------------------------
    UFUNCTION(NetMulticast, Reliable)
    void Multicast_Reload();
    UFUNCTION(NetMulticast, Reliable)
    void Multicast_ReloadEnd();
    UFUNCTION(NetMulticast, Reliable)
    void Multicast_CancelReload();
    // --- Fire -------------------------------------------------
    UFUNCTION(NetMulticast, Reliable)
    void Multicast_FireOnce();
    UFUNCTION(NetMulticast, Reliable)
    void Multicast_FireOnceEnd();
    UFUNCTION(NetMulticast, Reliable)
    void Multicast_CancelFireOnce();
    // --- SwitchAttackMode -------------------------------------------------
    UFUNCTION(NetMulticast, Reliable)
    void Multicast_SwitchAttackMode();
    UFUNCTION(NetMulticast, Reliable)
    void Multicast_SwitchAttackModeEnd();
    UFUNCTION(NetMulticast, Reliable)
    void Multicast_CancelSwitchAttackMode();

    // --- Server Functions -----------------------------------------------------
    // --- Equip -------------------------------------------------
    bool Authoritative_EquipBySlot(int32 SlotIndex);
    bool Authoritative_EquipBySlotEnd();
    void Authoritative_CancelEquipBySlot();
    // --- Reload -------------------------------------------------
    bool Authoritative_Reload();
    bool Authoritative_ReloadEnd();
    void Authoritative_CancelReload();
    // --- Fire -------------------------------------------------
    bool Authoritative_FireOnce(const FGameplayAbilityTargetDataHandle* FireTargetData = nullptr);
    bool Authoritative_FireOnceEnd();
    void Authoritative_CancelFireOnce();
    // --- SwitchAttackMode -------------------------------------------------
    bool Authoritative_SwitchAttackMode();
    bool Authoritative_SwitchAttackModeEnd();
    void Authoritative_CancelSwitchAttackMode();

    // --- Client Functions -----------------------------------------------------
    // --- Equip -------------------------------------------------
    bool Predict_EquipBySlot(int32 SlotIndex);
    bool Predict_EquipBySlotEnd();
    void Predict_CancelEquipBySlot();
    // --- Reload -------------------------------------------------
    bool Predict_Reload();
    bool Predict_ReloadEnd();
    void Predict_CancelReload();
    // --- Fire -------------------------------------------------
    bool MakePredictedProjectileFireTargetData(FGameplayAbilityTargetDataHandle& OutTargetData) const;
    bool Predict_FireOnce();
    bool Predict_FireOnceEnd();
    void Predict_RejectFireOnce();
    void Predict_CancelFireOnce();
    // --- SwitchAttackMode -------------------------------------------------
    bool Predict_SwitchAttackMode();
    bool Predict_SwitchAttackModeEnd();
    void Predict_CancelSwitchAttackMode();

    // --- Equip Change -------------------------------------------------
    bool Local_ChangeEquipBySlot(int32 SlotIndex);

    // --- Montage -------------------------------------------------
    void Local_PlayCharacterMontage(TSubclassOf<UAnimInstance> AnimClass, UAnimMontage* Montage, float PlayRate);
    void Local_StopCharacterMontage(UAnimMontage* Montage);
    void Local_PlayWeaponMontage(TSubclassOf<UAnimInstance> AnimClass, UAnimMontage* Montage, float PlayRate);
    void Local_StopWeaponMontage(UAnimMontage* Montage);


    // --- Common Functions -----------------------------------------------------
    bool Local_IsEquipCancelable() const;

private:
    // --- Server RPCs -----------------------------------------------------
    // --- Equip -------------------------------------------------
    UFUNCTION(Server, Reliable)
    void ServerEquipBySlot(int32 SlotIndex);
    UFUNCTION(Server, Reliable, WithValidation)
    void ServerDestroyWeapon(APX_Weapon* InWeapon);

    // --- Equip Change -------------------------------------------------
    UFUNCTION(Server, Reliable)
    void ServerChangeEquipBySlot(int32 SlotIndex);
    UFUNCTION(Server, Reliable)
    void ServerSetIsEquipCancelable(bool InValue);
    // --- Client RPCs -----------------------------------------------------
    // --- Equip -------------------------------------------------

    // --- Multicast RPCs -----------------------------------------------------
    // --- Equip -------------------------------------------------

    // --- OnRep Functions -----------------------------------------------------
    UFUNCTION()
    void OnRep_WeaponSlotIndexUpdated();
    UFUNCTION()
    void OnRep_WeaponItemInstanceIdUpdated();
    UFUNCTION()
    void OnRep_WeaponUpdated();

    // --- Common Functions -----------------------------------------------------
    void Local_SetIsEquipCancelable(bool InValue);

    // --- Server Functions -----------------------------------------------------
    

    // --- Client Functions -----------------------------------------------------
    void HandleInventoryReady();
    void HandleAmmoUpdated(int32 InAmmoInMag, int32 InReserved);
    void HandleAttackModeUpdated(FGameplayTag InAttackModeTag);
    void SyncPredictedAmmoFromCurrent();
    void SyncPredictedAttackModeFromCurrent();

    // --- Equip -------------------------------------------------
    

    // --- Replicated Variables ---------------------------------------------------
    // --- Equip ---------------------------------------------------
    // Current weapon slot index.
    UPROPERTY(ReplicatedUsing = OnRep_WeaponSlotIndexUpdated)
    int32 CurrentWeaponSlotIndex = INDEX_NONE;
    // Current weapon item instance id.
    UPROPERTY(ReplicatedUsing = OnRep_WeaponItemInstanceIdUpdated)
    FGuid CurrentWeaponItemInstanceId = FGuid();
    // Current weapon actor.
    UPROPERTY(ReplicatedUsing = OnRep_WeaponUpdated)
    TObjectPtr<APX_Weapon> CurrentWeapon = nullptr;

    // --- Common Variables ---------------------------------------------------
    // Current weapon's Character.
    UPROPERTY()
    TObjectPtr<ACharacter> Character = nullptr;
    // Current Character's Inventory. 
    UPROPERTY()
    TObjectPtr<UPX_InventoryComponent> Inventory = nullptr;
    // Current weapon item instance (Non-replicated).
    UPROPERTY(Transient, EditAnywhere, Category = "WeaponSystem")
    TObjectPtr<UPX_WeaponItemInstance> CurrentWeaponItemInstance = nullptr;
    UPROPERTY(Transient)
    TArray<TSubclassOf<UGameplayEffect>> PendingNextAttackStatusEffects;


    // --- Server Variables ---------------------------------------------------
    // --- Weapon ---------------------------------------------------
    /*
    // Equipping weapon index.
    UPROPERTY(Transient, EditAnywhere, Category = "WeaponSystem")
    int32 ServerEquippingWeaponSlotIndex = INDEX_NONE;
    // Equipping weapon instance Id.
    UPROPERTY(Transient, EditAnywhere, Category = "WeaponSystem")
    FGuid ServerEquippingWeaponItemInstanceId = FGuid();
    // Equipping weapon actor.
    UPROPERTY(Transient, EditAnywhere, Category = "WeaponSystem")
    TObjectPtr<APX_Weapon> ServerEquippingWeapon = nullptr;
    // Equipping weapon item instance.
    UPROPERTY(Transient, EditAnywhere, Category = "WeaponSystem")
    TObjectPtr<UPX_WeaponItemInstance> ServerEquippingWeaponItemInstance = nullptr;
    */
    // Cached Spawned-Weapon Container.
    UPROPERTY()
    TMap<FGuid, TObjectPtr<APX_Weapon>> ServerSpawnedWeapons; // { WeaponInstanceId, WeaponActor }

	//FTimerHandle ServerEquipTimerHandle;

    // 점사 모드의 한 번의 발사에서 남은 탄 수
    int32 BurstRemaining_Server = 0;
    // Interval을 고려한 다음 Attack 시간
    float NextAttackTime_Server = 0.f;

    // --- Local Client Variables ---------------------------------------------------
    // --- Weapon ---------------------------------------------------

    // Current weapon index. (Predicted)
    UPROPERTY(Transient, EditAnywhere, Category = "WeaponSystem")
    int32 PredictedCurrentWeaponSlotIndex = INDEX_NONE;
    // Current weapon instance Id.  (Predicted)
    UPROPERTY(Transient, EditAnywhere, Category = "WeaponSystem")
    FGuid PredictedCurrentWeaponItemInstanceId = FGuid();
    // Current weapon actor.  (Predicted)
    UPROPERTY(Transient, EditAnywhere, Category = "WeaponSystem")
	TObjectPtr<APX_Weapon> PredictedCurrentWeapon = nullptr;
    // Current weapon item instance.  (Predicted)
    UPROPERTY(Transient, EditAnywhere, Category = "WeaponSystem")
	TObjectPtr<UPX_WeaponItemInstance> PredictedCurrentWeaponItemInstance = nullptr;
    /*
	// Equipping weapon index. (Predicted)
    UPROPERTY(Transient, EditAnywhere, Category = "WeaponSystem")
	int32 PredictedEquippingWeaponSlotIndex = INDEX_NONE;
	// Equipping weapon instance Id.  (Predicted)
	UPROPERTY(Transient, EditAnywhere, Category = "WeaponSystem")
	FGuid PredictedEquippingWeaponItemInstanceId = FGuid();
	// Equipping weapon actor.  (Predicted)
    UPROPERTY(Transient, EditAnywhere, Category = "WeaponSystem")
	TObjectPtr<APX_Weapon> PredictedEquippingWeapon = nullptr;
	// Equipping weapon item instance.  (Predicted)
    UPROPERTY(Transient, EditAnywhere, Category = "WeaponSystem")
	TObjectPtr<UPX_WeaponItemInstance> PredictedEquippingWeaponItemInstance = nullptr;
    */
    // Cached Spawned-Weapon Container.
    UPROPERTY()
    TMap<FGuid, TObjectPtr<APX_Weapon>> PredictedSpawnedWeapons; // { WeaponInstanceId, WeaponActor }

    // 점사 모드의 한 번의 발사에서 남은 탄 수
    int32 BurstRemaining_Local = 0;
    // Interval을 고려한 다음 Attack 시간
    float NextAttackTime_Local = 0.f;

    bool bHasPredictedFireAmmoRollback = false;
    int32 PredictedFireAmmoRollback = INDEX_NONE;

    bool bHasPredictedCurrentAmmo = false;
    int32 PredictedCurrentAmmoInMag = INDEX_NONE;

    bool bHasPredictedAttackModeRollback = false;
    FGameplayTag PredictedAttackModeRollback;

    bool bHasPredictedCurrentAttackMode = false;
    FGameplayTag PredictedCurrentAttackModeTag;

    // --- Remote Client Variables ---------------------------------------------------
    // --- Weapon ---------------------------------------------------
    // // Current weapon index. (Remote)
    UPROPERTY(Transient, EditAnywhere, Category = "WeaponSystem")
    int32 RemoteCurrentWeaponSlotIndex = INDEX_NONE;
    // Current weapon instance Id.  (Remote)
    UPROPERTY(Transient, EditAnywhere, Category = "WeaponSystem")
    FGuid RemoteCurrentWeaponItemInstanceId = FGuid();
    // Current weapon actor.  (Remote)
    UPROPERTY(Transient, EditAnywhere, Category = "WeaponSystem")
    TObjectPtr<APX_Weapon> RemoteCurrentWeapon = nullptr;
    // Current weapon item instance.  (Remote)
    UPROPERTY(Transient, EditAnywhere, Category = "WeaponSystem")
    TObjectPtr<UPX_WeaponItemInstance> RemoteCurrentWeaponItemInstance = nullptr;
    /*
    // Equipping weapon index. (Remote)
    UPROPERTY(Transient, EditAnywhere, Category = "WeaponSystem")
    int32 RemoteEquippingWeaponSlotIndex = INDEX_NONE;
    // Equipping weapon instance Id.  (Remote)
    UPROPERTY(Transient, EditAnywhere, Category = "WeaponSystem")
    FGuid RemoteEquippingWeaponItemInstanceId = FGuid();
    // Equipping weapon actor.  (Remote)
    UPROPERTY(Transient, EditAnywhere, Category = "WeaponSystem")
    TObjectPtr<APX_Weapon> RemoteEquippingWeapon = nullptr;
    UPROPERTY(Transient, EditAnywhere, Category = "WeaponSystem")
    // Equipping weapon item instance.  (Remote)
    TObjectPtr<UPX_WeaponItemInstance> RemoteEquippingWeaponItemInstance = nullptr;
    */
    // Cached Spawned-Weapon Container. (Remote)
    TMap<FGuid, TObjectPtr<APX_Weapon>> RemoteSpawnedWeapons;
};
