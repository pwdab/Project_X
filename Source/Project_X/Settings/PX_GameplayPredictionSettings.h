// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "PX_GameplayPredictionSettings.generated.h"

UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Project X Gameplay Prediction"))
class PROJECT_X_API UPX_GameplayPredictionSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	static bool IsClientPredictionEnabled();
	static bool IsClientPredictedStaminaUIEnabled();

	virtual FName GetCategoryName() const override;

	UPROPERTY(Config, EditAnywhere, Category = "Prediction")
	bool bUseClientPrediction = true;

	UPROPERTY(Config, EditAnywhere, Category = "Prediction")
	bool bUseClientPredictedStaminaUI = true;
};
