// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Project_X.h"
#include "Blueprint/UserWidget.h"
#include "PX_InventoryItemSlotsWidget.generated.h"

struct FPXInventorySlot;
struct FPXInventorySlot;
class UPX_InventorySlotWidget;

/**
 * 
 */
UCLASS()
class PROJECT_X_API UPX_InventoryItemSlotsWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeOnInitialized() override;
	
public:
	// 단일 슬롯 갱신
	void UpdateSlot(int32 SlotIndex, const FPXInventorySlot& InSlot);
	// 전체 슬롯 갱신
	void UpdateSlots(const TArray<FPXInventorySlot>& InSlots);
	// 단일 슬롯 초기화
	void ClearSlot(int32 SlotIndex);
	// 전체 슬롯 초기화
	void ClearSlots();

protected:
	// 슬롯들이 들어있는 패널 (WBP에서 이름 SlotsPanel로 만들고 BindWidget)
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPanelWidget> SlotsPanel;

private:
	void InitSlotMeta();
};
