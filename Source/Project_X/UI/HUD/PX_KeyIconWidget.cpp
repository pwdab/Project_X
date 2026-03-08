// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/HUD/PX_KeyIconWidget.h"

void UPX_KeyIconWidget::UpdateWidget(int32 InSlotIndex)
{
	BP_UpdateWidget(InSlotIndex);
}

// 단일 슬롯 초기화
void UPX_KeyIconWidget::ClearWidget()
{
	BP_ClearWidget();
}

void UPX_KeyIconWidget::HighlightWidget(bool bHighlighted)
{
    SetRenderOpacity(bHighlighted ? 1.0f : 0.35f);
}


