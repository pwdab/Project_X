// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Project_X.h"
#include "Blueprint/UserWidget.h"
#include "PX_KeyIconWidget.generated.h"

class UTextBlock;

/**
 * 
 */
UCLASS()
class PROJECT_X_API UPX_KeyIconWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// 단일 슬롯 갱신
	void UpdateWidget(int32 InSlotIndex);
	UFUNCTION(BlueprintCallable, Category = "Key Icon")
	void UpdateText(const FText& InText);
	// 단일 슬롯 초기화
	void ClearWidget();
	void HighlightWidget(bool bHighlighted);
	
protected:
	// 단일 슬롯 갱신
	UFUNCTION(BlueprintImplementableEvent)
	void BP_UpdateWidget(int32 InSlotIndex);
	// 단일 슬롯 초기화
	UFUNCTION(BlueprintImplementableEvent)
	void BP_ClearWidget();
	
	
private:
	UTextBlock* ResolveKeyTextBlock() const;
};
