// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Project_X.h"
#include "Component/Inventory/PX_EquippableItemInstance.h"
#include "GameplayTagContainer.h"
//#include "Component/Weapon/PX_WeaponTypes.h"
#include "PX_WeaponItemInstance.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(PX_WeaponItemInstance, Log, All);

//class UPX_WeaponDataAsset;
//enum class EPXWeaponAttackMode;
struct FPX_ItemData;
class UGameplayAbility;
class UPX_WeaponDataAsset;

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnAmmoUpdated, int32/*AmmoInMag*/, int32/*AmmoInInventory*/);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnCurrentAttackModeUpdated, FGameplayTag/*AttackModeTag*/);

/**
 * 
 */
UCLASS()
class PROJECT_X_API UPX_WeaponItemInstance : public UPX_EquippableItemInstance
{
	GENERATED_BODY()
	
public:
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    // --- Server Functions ---------------------------------------------------
    //virtual UPX_WeaponItemInstance* ServerClone(UObject* NewOuter) const override;
    virtual FPX_ItemData MakeDropData() const override;
    virtual void ApplyDropData(const FPX_ItemData& Data) override;
    virtual void GiveAbilities(UAbilitySystemComponent* ASC) override;
    void SetAmmo(int32 Amount);
    void ConsumeAmmo(int32 Amount);
    bool SwitchAttackMode();
    bool SetAttackMode(FGameplayTag NewAttackModeTag);

    // --- Getters -------------------------------------------------
    FORCEINLINE int32 GetAmmo() const { return AmmoInMag; }
    FORCEINLINE FGameplayTag GetAttackMode() const { return AttackModeTag; }
    FORCEINLINE float GetDurability() const { return Durability; }

    // --- Delegate Variables -----------------------------------------------------
    FOnAmmoUpdated OnAmmoUpdated;
    FOnCurrentAttackModeUpdated OnAttackModeUpdated;

protected:
    // --- Server Functions ---------------------------------------------------
    virtual void InitializeFromData(UPX_ItemDataAsset* InItemDataAsset, int32 Quantity) override;

private:
    TSubclassOf<UGameplayAbility> GetWeaponStatusImbueAbilityClass() const;

private:
    // --- OnRep Functions -----------------------------------------------------
    UFUNCTION()
    void OnRep_AmmoUpdated();
    UFUNCTION()
    void OnRep_AttackModeUpdated();

    // --- Replicated Variables ---------------------------------------------------
    // Ammo in magazine.
    UPROPERTY(ReplicatedUsing = OnRep_AmmoUpdated, EditAnywhere)
    int32 AmmoInMag = 0;
    // Current weapon's attack mode.
    /* AttackModeTag로 대체
    UPROPERTY(ReplicatedUsing = OnRep_AttackModeUpdated, EditAnywhere)
    EPXWeaponAttackMode AttackMode = EPXWeaponAttackMode::None;
    */
    UPROPERTY(ReplicatedUsing = OnRep_AttackModeUpdated, EditAnywhere)
    FGameplayTag AttackModeTag;
    // Current weapon's durability.
    UPROPERTY(Replicated, EditAnywhere, BlueprintReadOnly, Category = "Weapon|State", meta = (AllowPrivateAccess = "true"))
    float Durability = -1.0f; // -1 means infinite
};
