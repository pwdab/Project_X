#pragma once

#include "Project_X.h"
#include "PX_WeaponTypes.generated.h"

/** How the weapon visuals are driven while an action is playing. */
UENUM(BlueprintType)
enum class EPXWeaponAnimMode : uint8
{
    /** No weapon animation: character montage only (e.g., melee sword). */
    None UMETA(DisplayName = "None"),

    /** Weapon has its own AnimBP/Montage to play (e.g., pistol/rifle/shotgun). */
    Montage UMETA(DisplayName = "Montage"),

    /** Weapon visuals require constraints/IK/driver updates (e.g., bow). */
    Constraint UMETA(DisplayName = "Constraint/Driver"),
};

/** How the weapon visuals are driven while an action is playing. */
UENUM(BlueprintType)
enum class EPXWeaponAttackMode : uint8
{
    None UMETA(DisplayName = "None"),
    Single UMETA(DisplayName = "Single"),
    Combo UMETA(DisplayName = "Combo"),
    Burst UMETA(DisplayName = "Burst"),
    Auto UMETA(DisplayName = "Auto"),
};

static const TCHAR* WeaponAttackModeToString(EPXWeaponAttackMode Mode)
{
    switch ( Mode )
    {
    case EPXWeaponAttackMode::None:   return TEXT("None");
    case EPXWeaponAttackMode::Single: return TEXT("Single");
    case EPXWeaponAttackMode::Combo: return TEXT("Combo");
    case EPXWeaponAttackMode::Burst:  return TEXT("Burst");
    case EPXWeaponAttackMode::Auto:   return TEXT("Auto");
    default:                        return TEXT("Unknown");
    }
}

/** Ammo Type */
UENUM(BlueprintType)
enum class EPXAmmoType : uint8
{
    None        UMETA(DisplayName = "None"),
    Pistol      UMETA(DisplayName = "Pistol"),
    Rifle       UMETA(DisplayName = "Rifle"),
    Shotgun     UMETA(DisplayName = "Shotgun"),
    Sniper      UMETA(DisplayName = "Sniper"),
    Arrow       UMETA(DisplayName = "Arrow"),
    Bolt        UMETA(DisplayName = "Bolt"),
    Energy      UMETA(DisplayName = "Energy"),
};

static const TCHAR* AmmoTypeToString(EPXAmmoType AmmoType)
{
    switch ( AmmoType )
    {
    case EPXAmmoType::Pistol:   return TEXT("Pistol");
    case EPXAmmoType::Rifle:    return TEXT("Rifle");
    case EPXAmmoType::Shotgun:  return TEXT("Shotgun");
    case EPXAmmoType::Sniper:   return TEXT("Sniper");
    case EPXAmmoType::Arrow:    return TEXT("Arrow");
    case EPXAmmoType::Bolt:     return TEXT("Bolt");
    case EPXAmmoType::Energy:   return TEXT("Energy");
    default:                    return TEXT("None");
    }
}

/** High-level action types a weapon system can execute. */
UENUM(BlueprintType)
enum class EPXWeaponActionType : uint8
{
    None   UMETA(DisplayName = "None"),
    BeginEquip  UMETA(DisplayName = "BeginEquip"),
    EndEquip  UMETA(DisplayName = "EndEquip"),
    AttackPressed   UMETA(DisplayName = "AttackPressed"),
    AttackReleased   UMETA(DisplayName = "AttackReleased"),
    AttackOnce   UMETA(DisplayName = "AttackOnce"),
    BeginReload UMETA(DisplayName = "BeginReload"),
    EndReload UMETA(DisplayName = "EndReload"),
    Aim_On UMETA(DisplayName = "Aim On"),
    Aim_Off UMETA(DisplayName = "Aim Off"),
    SetFireMode  UMETA(DisplayName = "SetFireMode"),
};



USTRUCT(BlueprintType)
struct FPXWeaponActionContext
{
    GENERATED_BODY()

    // --- Debug only ---
    UPROPERTY(BlueprintReadOnly)
    //TObjectPtr<AActor> Owner = nullptr;
    FString InstigatorDebugName = "";

    UPROPERTY(BlueprintReadOnly)
    int32 InstigatorId = -1;

    UPROPERTY(BlueprintReadOnly)
    int32 ClientSequence = INDEX_NONE;

    UPROPERTY(BlueprintReadOnly)
    int32 ServerSequence = INDEX_NONE;

    UPROPERTY(BlueprintReadOnly)
    EPXWeaponActionType Type = EPXWeaponActionType::None;

    UPROPERTY(BlueprintReadOnly)
    int32 SlotIndex = INDEX_NONE;

    UPROPERTY(BlueprintReadOnly)
    bool bShouldDryFire = false;

    UPROPERTY(BlueprintReadOnly)
    bool bActionApproved = false;

    UPROPERTY(BlueprintReadOnly)
    FString ActionDenyReason = "";

    // Debug Print
    FString WeaponActionTypeToString(EPXWeaponActionType InType) const
    {
        const UEnum* EnumPtr = StaticEnum<EPXWeaponActionType>();
        if ( !EnumPtr )
        {
            return TEXT("Invalid");
        }

        return EnumPtr->GetNameStringByValue((int64)InType);
    }

    // Debug Print
    FString ToString() const
    {
        if ( SlotIndex != INDEX_NONE ) return FString::Printf(TEXT("FPXWeaponActionContext { Instigator = %s, ClientSeq = %d, ServerSeq = %d, Type = %s, Index = %d }"), *InstigatorDebugName, ClientSequence, ServerSequence, *WeaponActionTypeToString(Type), SlotIndex);
        return FString::Printf(TEXT("FPXWeaponActionContext { Instigator = %s(#%d), ClientSeq = %d, ServerSeq = %d, Type = %s }"), *InstigatorDebugName, InstigatorId, ClientSequence, ServerSequence, *WeaponActionTypeToString(Type));
    }

    static FPXWeaponActionContext Builder(EPXWeaponActionType InType, int32 SlotIndex = INDEX_NONE)
    {
        FPXWeaponActionContext Context;
        Context.Type = InType;
        Context.SlotIndex = SlotIndex;
        return Context;
    }

    /*
    static FPXWeaponActionContext None(TObjectPtr<AActor> Owner)
    {
        FPXWeaponActionContext action;
        action.Owner = Owner;
        action.Type = EPXWeaponActionType::None;
        return action;
    }

    static FPXWeaponActionContext Equip(TObjectPtr<AActor> Owner, int32 Index = 0)
    {
        FPXWeaponActionContext action;
        action.Owner = Owner;
        action.Type = EPXWeaponActionType::Equip;
        //action.SlotIndex = Index;
        return action;
    }

    static FPXWeaponActionContext FirePressed(TObjectPtr<AActor> Owner)
    {
        FPXWeaponActionContext action;
        action.Owner = Owner;
        action.Type = EPXWeaponActionType::AttackPressed;
        return action;
    }

    static FPXWeaponActionContext FireReleased(TObjectPtr<AActor> Owner)
    {
        FPXWeaponActionContext action;
        action.Owner = Owner;
        action.Type = EPXWeaponActionType::AttackReleased;
        return action;
    }

    static FPXWeaponActionContext FireOnce(TObjectPtr<AActor> Owner)
    {
        FPXWeaponActionContext action;
        action.Owner = Owner;
        action.Type = EPXWeaponActionType::AttackOnce;
        return action;
    }

    static FPXWeaponActionContext Reload(TObjectPtr<AActor> Owner)
    {
        FPXWeaponActionContext action;
        action.Owner = Owner;
        action.Type = EPXWeaponActionType::Reload;
        return action;
    }

    static FPXWeaponActionContext Aim_On(TObjectPtr<AActor> Owner)
    {
        FPXWeaponActionContext action;
        action.Owner = Owner;
        action.Type = EPXWeaponActionType::Aim_On;
        return action;
    }

    static FPXWeaponActionContext Aim_Off(TObjectPtr<AActor> Owner)
    {
        FPXWeaponActionContext action;
        action.Owner = Owner;
        action.Type = EPXWeaponActionType::Aim_Off;
        return action;
    }
    */
};

/** Action definition that can drive character+weapon visuals together. */
USTRUCT(BlueprintType)
struct FPXWeaponActionMontageData
{
    GENERATED_BODY()

    /** Character montage for this action. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TObjectPtr<UAnimMontage> CharacterMontage = nullptr;

    /** Optional weapon montage for this action (for EPXWeaponAnimMode::Montage). */
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TObjectPtr<UAnimMontage> WeaponMontage = nullptr;

    /** Optional named section to start on the character montage. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FName CharacterSection;

    /** Optional named section to start on the weapon montage. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FName WeaponSection;

    /** Montage play rate. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float PlayRate = 1.0f;

    /** Name of notify event used to "commit" gameplay (spawn bullet, melee trace, etc.). */
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FName CommitNotifyName = TEXT("PX_ActionCommit");

    /** Name of notify event used to mark end of action for cleanup. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FName EndNotifyName = TEXT("PX_ActionEnd");

    /** Optional profile name to activate driver/constraint behavior for this action (bows, special weapons). */
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FName ConstraintProfile;
};

USTRUCT(BlueprintType)
struct FPXWeaponActionAnimContext
{
    GENERATED_BODY()

    // 어떤 방식으로 무기 비주얼을 구동할지
    UPROPERTY(BlueprintReadOnly)
    EPXWeaponAnimMode AnimMode = EPXWeaponAnimMode::None;

    // 재생할 몽타주
    UPROPERTY(BlueprintReadOnly)
    FPXWeaponActionMontageData MontageToPlay;

    // Layered Anim Instance state changes (부분 변경 안전)
    bool bSetIsAiming = false;
    bool bIsAiming = false;

    bool bSetIsDrawing = false;
    bool bIsDrawing = false;

    bool bSetIsReloading = false;
    bool bIsReloading = false;

    UPROPERTY(BlueprintReadOnly)
    int32 Priority = 0; // 높을수록 우선

    // Debug Print
    FString ToString() const
    {
        auto BoolToStr = [](bool b) {
            return b ? TEXT("true") : TEXT("false");
            };

        auto AnimModeToStr = [](EPXWeaponAnimMode Mode) {
            const UEnum* EnumPtr = StaticEnum<EPXWeaponAnimMode>();
            return EnumPtr
                ? EnumPtr->GetNameStringByValue(static_cast<int64>(Mode))
                : TEXT("Invalid");
            };

        const FString CharacterMontageName = MontageToPlay.CharacterMontage ? MontageToPlay.CharacterMontage->GetName() : TEXT("None");
        const FString WeaponMontageName = MontageToPlay.WeaponMontage ? MontageToPlay.WeaponMontage->GetName() : TEXT("None");

        return FString::Printf(
            TEXT(
                "ActionAnimContext  { AnimMode=%s, Priority=%d, Montage={ Character=%s, Weapon=%s }, SetAiming=%s -> %s, SetDrawing=%s -> %s, SetReloading=%s -> %s }"
            ),
            *AnimModeToStr(AnimMode),
            Priority,
            *CharacterMontageName,
            *WeaponMontageName,
            BoolToStr(bSetIsAiming), BoolToStr(bIsAiming),
            BoolToStr(bSetIsDrawing), BoolToStr(bIsDrawing),
            BoolToStr(bSetIsReloading), BoolToStr(bIsReloading)
        );
    }

};