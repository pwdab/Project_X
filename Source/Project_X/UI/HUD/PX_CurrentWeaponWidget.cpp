// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/HUD/PX_CurrentWeaponWidget.h"
#include "UI/HUD/PX_KeyIconWidget.h"
#include "UI/HUD/PX_WeaponIconWidget.h"

void UPX_CurrentWeaponWidget::UpdateWidget(const FPXCurrentWeaponHUDData& Data)
{
	if ( !KeyIcon || !WeaponIcon ) return;

	KeyIcon->UpdateWidget(Data.SlotIndex + 1);
	WeaponIcon->UpdateWidget(Data.WeaponIcon);

	//PX_LOG(Log, TEXT(""));

	BP_UpdateWidget(Data);
}

void UPX_CurrentWeaponWidget::UpdateAttackMode(const FText& InAttackMode)
{
	//PX_LOG(Log, TEXT(""));
	BP_UpdateAttackMode(InAttackMode);
}

void UPX_CurrentWeaponWidget::UpdateAmmo(int32 InAmmoInMag, int32 InReserved)
{
	BP_UpdateAmmo(InAmmoInMag, InReserved);
}

void UPX_CurrentWeaponWidget::ClearWidget()
{
	if ( !KeyIcon || !WeaponIcon ) return;

	KeyIcon->ClearWidget();
	WeaponIcon->ClearWidget();

	BP_ClearWidget();
}


