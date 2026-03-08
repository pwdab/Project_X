// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Project_X.h"
#include "Blueprint/UserWidget.h"
#include "PX_CurrentWeaponWidget.generated.h"

class UPX_KeyIconWidget;
class UPX_WeaponIconWidget;

USTRUCT(BlueprintType)
struct FPXCurrentWeaponHUDData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly) int32 SlotIndex = INDEX_NONE;
	UPROPERTY(BlueprintReadOnly) FText WeaponName;
	UPROPERTY(BlueprintReadOnly) TObjectPtr<UTexture2D> WeaponIcon = nullptr;
	UPROPERTY(BlueprintReadOnly) FText AmmoType;
	UPROPERTY(BlueprintReadOnly) FText AttackMode;
	UPROPERTY(BlueprintReadOnly) int32 AmmoInMag = 0;
	UPROPERTY(BlueprintReadOnly) int32 Reserved = 0;

	bool IsValid() const { return SlotIndex != INDEX_NONE; }
};

/**
 * 
 */
UCLASS()
class PROJECT_X_API UPX_CurrentWeaponWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION()
	void UpdateWidget(const FPXCurrentWeaponHUDData& Data);
	UFUNCTION()
	void UpdateAttackMode(const FText& InAttackMode);
	UFUNCTION()
	void UpdateAmmo(int32 InAmmoInMag, int32 InReserved);
	UFUNCTION()
	void ClearWidget();

protected:
	// KeyIcon Panel
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPX_KeyIconWidget> KeyIcon;
	// WeaponIcon Panel
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPX_WeaponIconWidget> WeaponIcon;

	// 단일 슬롯 갱신
	UFUNCTION(BlueprintImplementableEvent, Category = "PX|Inventory|Slot")
	void BP_UpdateWidget(const FPXCurrentWeaponHUDData& Data);
	UFUNCTION(BlueprintImplementableEvent, Category = "PX|Inventory|Slot")
	void BP_UpdateAttackMode(const FText& InAttackMode);
	// 단일 슬롯 갱신
	UFUNCTION(BlueprintImplementableEvent, Category = "PX|Inventory|Slot")
	void BP_UpdateAmmo(int32 InAmmoInMag, int32 InReserved);
	// 단일 슬롯 초기화
	UFUNCTION(BlueprintImplementableEvent, Category = "PX|Inventory|Slot")
	void BP_ClearWidget();
	
};
