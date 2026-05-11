// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/Effects/PX_GE_Damage.h"

#include "AbilitySystem/Attributes/PX_ResourceAttributeSet.h"

const FName UPX_GE_Damage::DamageSetByCallerName(TEXT("Data.Damage"));

UPX_GE_Damage::UPX_GE_Damage()
{
	DurationPolicy = EGameplayEffectDurationType::Instant;

	FGameplayModifierInfo DamageModifier;
	DamageModifier.Attribute = UPX_ResourceAttributeSet::GetDamageAttribute();
	DamageModifier.ModifierOp = EGameplayModOp::Additive;

	FSetByCallerFloat DamageMagnitude;
	DamageMagnitude.DataName = DamageSetByCallerName;
	DamageModifier.ModifierMagnitude = FGameplayEffectModifierMagnitude(DamageMagnitude);

	Modifiers.Add(DamageModifier);
}
