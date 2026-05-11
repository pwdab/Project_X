// Fill out your copyright notice in the Description page of Project Settings.

#include "Settings/PX_GameplayPredictionSettings.h"

bool UPX_GameplayPredictionSettings::IsClientPredictionEnabled()
{
	return GetDefault<UPX_GameplayPredictionSettings>()->bUseClientPrediction;
}

bool UPX_GameplayPredictionSettings::IsClientPredictedStaminaUIEnabled()
{
	return GetDefault<UPX_GameplayPredictionSettings>()->bUseClientPredictedStaminaUI;
}

FName UPX_GameplayPredictionSettings::GetCategoryName() const
{
	return TEXT("Project X");
}
