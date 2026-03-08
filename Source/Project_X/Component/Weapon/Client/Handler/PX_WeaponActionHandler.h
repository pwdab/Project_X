// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Project_X.h"
#include "UObject/Object.h"
#include "PX_WeaponActionHandler.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(PX_WeaponActionHandler, Log, All);

class UPX_WeaponSystemComponent;
class UPX_WeaponAnimDriver;
class UPX_WeaponDataAsset;
struct FPXWeaponActionContext;
struct FPXWeaponActionAnimContext;
struct FPXWeaponActionMontageData;

/**
 *
 */
UCLASS(BlueprintType, Blueprintable)
class PROJECT_X_API UPX_WeaponActionHandler : public UObject
{
    GENERATED_BODY()

public:
    void Initialize(UPX_WeaponSystemComponent* InWeaponSystemComponent, UPX_WeaponAnimDriver* InDriver, UPX_WeaponDataAsset* InDataAsset);
    void CommitCosmetic(const FPXWeaponActionContext& ActionContext, bool bIsLocal);

protected:
    // ActionContext를 보고 AnimContext를 구성
    bool BuildAnimContext(const FPXWeaponActionContext& ActionContext, FPXWeaponActionAnimContext& OutAnimContext, bool bIsLocal) const;
    // MontageData를 AnimContext로 변환
    static bool CopyMontageDataToAnimContext(const FPXWeaponActionMontageData& In, FPXWeaponActionAnimContext& Out);

    UPROPERTY()
    TObjectPtr<UPX_WeaponSystemComponent> WeaponSystemComponent;
    UPROPERTY()
    TObjectPtr<UPX_WeaponAnimDriver> AnimDriver;
    UPROPERTY()
    TObjectPtr<UPX_WeaponDataAsset> WeaponData;
};

