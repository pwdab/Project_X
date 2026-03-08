// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/HUD/PX_WeaponListWidget.h"
#include "UI/HUD/PX_WeaponIconWidget.h"
#include "UI/HUD/PX_KeyIconWidget.h"

void UPX_WeaponListWidget::UpdateWidget(int32 SlotIndex, UTexture2D* InTexture)
{
    if ( !WeaponIcon || !KeyIcon ) return;

    WeaponIcon->UpdateWidget(InTexture);
    KeyIcon->UpdateWidget(SlotIndex);
}

void UPX_WeaponListWidget::ClearWidget()
{
    if ( !WeaponIcon || !KeyIcon ) return;

    WeaponIcon->ClearWidget();
    KeyIcon->ClearWidget();
    //KeyIcon->UpdateWidget(SlotIndex);
}

void UPX_WeaponListWidget::HighlightWidget(bool bHighlighted)
{
    if ( !WeaponIcon || !KeyIcon ) return;

    WeaponIcon->HighlightWidget(bHighlighted);
    KeyIcon->HighlightWidget(bHighlighted);
}


