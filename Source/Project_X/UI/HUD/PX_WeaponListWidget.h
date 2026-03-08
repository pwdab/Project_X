// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Project_X.h"
#include "Blueprint/UserWidget.h"
#include "PX_WeaponListWidget.generated.h"

class UPX_WeaponIconWidget;
class UPX_KeyIconWidget;

/**
 * 
 */
UCLASS()
class PROJECT_X_API UPX_WeaponListWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION()
	void UpdateWidget(int32 SlotIndex, UTexture2D* InTexture);
	UFUNCTION()
	void ClearWidget();
	UFUNCTION()
	void HighlightWidget(bool bHighlighted);

protected:
	// WeaponIcon Panel
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPX_WeaponIconWidget> WeaponIcon;
	// KeyIcon Panel
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPX_KeyIconWidget> KeyIcon;
	
	
};
