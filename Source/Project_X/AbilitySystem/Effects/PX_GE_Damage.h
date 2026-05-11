// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "PX_GE_Damage.generated.h"

UCLASS()
class PROJECT_X_API UPX_GE_Damage : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UPX_GE_Damage();

	static const FName DamageSetByCallerName;
};
