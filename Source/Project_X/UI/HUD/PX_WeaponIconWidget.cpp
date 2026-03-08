// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/HUD/PX_WeaponIconWidget.h"

void UPX_WeaponIconWidget::UpdateWidget(TObjectPtr<UTexture2D> InTexture)
{
	BP_UpdateWidget(InTexture.Get());
}

// 단일 슬롯 초기화
void UPX_WeaponIconWidget::ClearWidget()
{
	BP_ClearWidget();
}

void UPX_WeaponIconWidget::HighlightWidget(bool bHighlighted)
{
	SetRenderOpacity(bHighlighted ? 1.0f : 0.35f);
	SetRenderScale(bHighlighted ? FVector2D(1.05f, 1.05f) : FVector2D(1.0f, 1.0f));
}
