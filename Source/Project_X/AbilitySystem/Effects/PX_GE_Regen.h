// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Project_X.h"
#include "GameplayEffect.h"
#include "GameplayEffectExecutionCalculation.h"
#include "PX_GE_Regen.generated.h"

UCLASS()
class PROJECT_X_API UPX_ExecCalc_ResourceRegen : public UGameplayEffectExecutionCalculation
{
	GENERATED_BODY()

public:
	UPX_ExecCalc_ResourceRegen();

	virtual void Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const override;
};

UCLASS()
class PROJECT_X_API UPX_GE_ResourceRegen : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UPX_GE_ResourceRegen();
};
