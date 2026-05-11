// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "PX_GE_StatusEffects.generated.h"

UCLASS()
class PROJECT_X_API UPX_GE_BurningDamage : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UPX_GE_BurningDamage();
};

UCLASS()
class PROJECT_X_API UPX_GE_SlowDebuff : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UPX_GE_SlowDebuff();
};

UCLASS()
class PROJECT_X_API UPX_GE_StunDebuff : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UPX_GE_StunDebuff();
};

UCLASS()
class PROJECT_X_API UPX_GE_AbilityCooldown : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UPX_GE_AbilityCooldown();
};
