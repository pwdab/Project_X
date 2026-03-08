// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/HUD/PX_WeaponListsWidget.h"
#include "UI/HUD/PX_WeaponListWidget.h"

void UPX_WeaponListsWidget::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    WeaponLists = { WeaponList_0, WeaponList_1, WeaponList_2, WeaponList_3};
}

void UPX_WeaponListsWidget::UpdateWidget(int32 SlotIndex, UTexture2D* InTexture)
{
    if ( !WeaponLists.IsValidIndex(SlotIndex) ) return;

    if ( WeaponLists[SlotIndex] )
    {
        WeaponLists[SlotIndex]->UpdateWidget(SlotIndex + 1, InTexture);
    }
}

void UPX_WeaponListsWidget::ClearWidget(int32 SlotIndex)
{
    if ( !WeaponLists.IsValidIndex(SlotIndex) ) return;

    if ( WeaponLists[SlotIndex] )
    {
        WeaponLists[SlotIndex]->ClearWidget();
    }
}

void UPX_WeaponListsWidget::ClearWidgets()
{
    /*
    for (int32 SlotIndex = 0; SlotIndex < WeaponLists.Num(); ++SlotIndex )
    {
        if ( WeaponLists[SlotIndex] )
        {
            WeaponLists[SlotIndex]->ClearWidget(SlotIndex + 1);
        }
    }
    */

    for ( UPX_WeaponListWidget* WeaponList : WeaponLists )
    {
        if ( WeaponList )
        {
            WeaponList->ClearWidget();
        }
    }
}

void UPX_WeaponListsWidget::HighlightWidgets(int32 SlotIndex)
{
    for ( int32 i = 0; i < WeaponLists.Num(); ++i )
    {
        if ( WeaponLists[i] )
        {
            WeaponLists[i]->HighlightWidget(i == SlotIndex);
        }
    }
}


