// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Project_X.h"
#include "Component/Inventory/PX_ItemInstance.h"
#include "Component/Weapon/PX_WeaponTypes.h"
#include "PX_WeaponItemInstance.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(PX_WeaponItemInstance, Log, All);

//class UPX_WeaponDataAsset;
//enum class EPXWeaponAttackMode;
struct FPX_ItemData;

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnAmmoUpdated, int32/*MagSize*/, int32/*AmmoInMag*/);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnAttackModeUpdated, EPXWeaponAttackMode/*AttackMode*/);

/**
 * 
 */
UCLASS()
class PROJECT_X_API UPX_WeaponItemInstance : public UPX_ItemInstance
{
	GENERATED_BODY()
	
public:
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    // --- Server Functions ---------------------------------------------------
    //virtual UPX_WeaponItemInstance* ServerClone(UObject* NewOuter) const override;
    virtual FPX_ItemData MakeDropData() const override;
    virtual void ApplyDropData(const FPX_ItemData& Data) override;
    void SetAmmo(int32 Amount);
    void ConsumeAmmo(int32 Amount);
    void SwitchFireMode();
    void SetFireMode(EPXWeaponAttackMode NewMode);

    // --- Getters -------------------------------------------------
    FORCEINLINE int32 GetAmmo() const { return AmmoInMag; }
    FORCEINLINE EPXWeaponAttackMode GetAttackMode() const { return AttackMode; }
    FORCEINLINE float GetDurability() const { return Durability; }

    // --- Delegate Variables -----------------------------------------------------
    FOnAmmoUpdated OnAmmoUpdated;
    FOnAttackModeUpdated OnAttackModeUpdated;

protected:
    // --- Server Functions ---------------------------------------------------
    virtual void InitializeFromData(UPX_ItemDataAsset* InItemDataAsset, int32 Quantity) override final;

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
    // Current weapon's fire mode.
    UPROPERTY(ReplicatedUsing = OnRep_AttackModeUpdated, EditAnywhere)
    EPXWeaponAttackMode AttackMode = EPXWeaponAttackMode::None;
    // Current weapon's durability.
    UPROPERTY(Replicated, EditAnywhere, BlueprintReadOnly, Category = "Weapon|State", meta = (AllowPrivateAccess = "true"))
    float Durability = -1.0f; // -1 means infinite
};
