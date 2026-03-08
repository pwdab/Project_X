// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Project_X.h"
#include "Blueprint/UserWidget.h"
#include "PX_WeaponIconWidget.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_X_API UPX_WeaponIconWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	// 단일 슬롯 갱신
	void UpdateWidget(TObjectPtr<UTexture2D> InTexture);
	// 단일 슬롯 초기화
	void ClearWidget();
	void HighlightWidget(bool bHighlighted);

protected:
	// 단일 슬롯 갱신
	UFUNCTION(BlueprintImplementableEvent)
	void BP_UpdateWidget(UTexture2D* InTexture);
	// 단일 슬롯 초기화
	UFUNCTION(BlueprintImplementableEvent)
	void BP_ClearWidget();
	
	
};
