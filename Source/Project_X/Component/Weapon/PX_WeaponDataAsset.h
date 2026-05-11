// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Project_X.h"
#include "Component/Inventory/PX_EquippableItemDataAsset.h"
//#include "Entity/PX_Weapon.h"
#include "Entity/PX_Projectile.h"
#include "Component/Weapon/PX_WeaponTypes.h"
#include "PX_WeaponDataAsset.generated.h"

class UPX_WeaponDataAsset;
class USkeletalMesh;
class UAnimInstance;
class UPX_WeaponActionHandler;
class UPX_WeaponAnimDriver;

UCLASS(BlueprintType)
class PROJECT_X_API UPX_WeaponDataAsset : public UPX_EquippableItemDataAsset
{
    GENERATED_BODY()

public:
    // --- Weapon ---------------------------------------------------
    /** Actor class to spawn for this weapon (Default: APX_Weapon). */
    //UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Weapon Data")
    //TSubclassOf<APX_Weapon> WeaponClass = APX_Weapon::StaticClass();
    /** Skeletal mesh to assign to spawned weapon. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Weapon Data")
    TObjectPtr<USkeletalMesh> WeaponMesh;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Data")
    float MaxDurability = -1.0f; // -1 means infinite

    // --- Anim ---------------------------------------------------
    /** AnimBP class to assign to Character mesh. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Data")
    TSubclassOf<UAnimInstance> CharacterAnimClass;
    /** AnimBP class to assign to Weapon mesh. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Weapon Data")
    TSubclassOf<UAnimInstance> WeaponAnimClass;
    /** How the weapon visuals are driven. */
    /* AnimModeTag로 대체
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Weapon Data")
    EPXWeaponAnimMode AnimMode = EPXWeaponAnimMode::None;
    */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon Data")
    FGameplayTag AnimModeTag;

    // --- Socket ---------------------------------------------------
    /** Socket on character mesh to attach weapon to. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Weapon Data")
    FName AttachSocketName = TEXT("Weapon_R");
    /** Deprecated for projectile spawning; weapon meshes are expected to expose a socket named "Muzzle". */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Weapon Data")
    FName MuzzleSocketName = TEXT("Muzzle");

    // --- Ammo ---------------------------------------------------
    /** Projectile class. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Data")
    TSubclassOf<APX_Projectile> ProjectileClass;
    /** Ammo Type */
    /* AmmoTypeTag로 대체
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Data", meta = (EditCondition = "ProjectileClass != nullptr"))
    EPXAmmoType AmmoType = EPXAmmoType::None;
    */
    UPROPERTY(EditDefaultsOnly)
    FGameplayTag AmmoTypeTag;
    /** Magazine size (0 means infinite / not used). */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Weapon Data", meta = (EditCondition = "ProjectileClass != nullptr"))
    int32 MagSize = 0;
    /** Fire interval (seconds). 0 means no gating. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Weapon Data")
    float FireInterval = 0.1f;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon|Fire", meta = (ClampMin = "1"), meta = (EditCondition = "ProjectileClass != nullptr"))
    int32 BurstCount = 3;
    /* SupportedAttackModeTags로 대체
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon|Fire", meta = (EditCondition = "ProjectileClass != nullptr"))
    TArray<EPXWeaponAttackMode> SupportedAttackModes;
    */
    UPROPERTY(EditDefaultsOnly)
    TArray<FGameplayTag> SupportedAttackModeTags;
	/* DefaultAttackModeTag로 대체
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon|Fire", meta = (EditCondition = "ProjectileClass != nullptr"))
    EPXWeaponAttackMode DefaultAttackMode = EPXWeaponAttackMode::Single;
    */
    UPROPERTY(EditDefaultsOnly)
    FGameplayTag DefaultAttackModeTag;

    // --- Action ---------------------------------------------------
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Data")
    TSubclassOf<UPX_WeaponActionHandler> ActionHandlerClass;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Data")
    TSubclassOf<UPX_WeaponAnimDriver> AnimDriverClass;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Weapon Data")
    FPXWeaponActionMontageData EquipAction;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Weapon Data")
    FPXWeaponActionMontageData FireAction;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Data")
    FPXWeaponActionMontageData DryFireAction;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Weapon Data")
    FPXWeaponActionMontageData ReloadAction;

};
