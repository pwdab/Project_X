// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Project_X.h"
#include "Blueprint/UserWidget.h"
#include "PX_WeaponListsWidget.generated.h"

class UPX_WeaponListWidget;

/**
 * 
 */
UCLASS()
class PROJECT_X_API UPX_WeaponListsWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION()
	void UpdateWidget(int32 SlotIndex, UTexture2D* InTexture);
	UFUNCTION()
	void ClearWidget(int32 SlotIndex);
	UFUNCTION()
	void ClearWidgets();
	UFUNCTION()
	void HighlightWidgets(int32 SlotIndex);

protected:
	virtual void NativeOnInitialized() override;

	// PX_WeaponListWidget Panel
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UPX_WeaponListWidget> WeaponList_0;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UPX_WeaponListWidget> WeaponList_1;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UPX_WeaponListWidget> WeaponList_2;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UPX_WeaponListWidget> WeaponList_3;

private:
	// Cached PX_WeaponListWidget Array
	UPROPERTY()
	TArray<TObjectPtr<UPX_WeaponListWidget>> WeaponLists;
	
	
};
