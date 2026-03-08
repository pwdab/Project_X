// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Project_X.h"
#include "UObject/Object.h"
#include "PX_WeaponAnimDriver.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(PX_WeaponAnimDriver, Log, All);

class UAnimInstance;
struct FPXWeaponActionAnimContext;

/**
 *
 */
//UCLASS(Abstract, Blueprintable, EditInlineNew, DefaultToInstanced)
UCLASS(Blueprintable, EditInlineNew, DefaultToInstanced)
class PROJECT_X_API UPX_WeaponAnimDriver : public UObject
{
    GENERATED_BODY()

public:
    void Initialize(UAnimInstance* InCharacterAnim, UAnimInstance* InWeaponAnim);
    void PlayCommittedAction(const FPXWeaponActionAnimContext& AnimContext, bool bIsLocal);

private:
    UPROPERTY()
    TObjectPtr<UAnimInstance> CharacterAnim;
    UPROPERTY()
    TObjectPtr<UAnimInstance> WeaponAnim;
};
