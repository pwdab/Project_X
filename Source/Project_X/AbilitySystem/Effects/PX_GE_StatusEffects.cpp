// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/Effects/PX_GE_StatusEffects.h"

#include "AbilitySystem/Attributes/PX_MovementAttributeSet.h"
#include "AbilitySystem/Attributes/PX_ResourceAttributeSet.h"
#include "AbilitySystem/Tags/PX_GameplayTags.h"

UPX_GE_BurningDamage::UPX_GE_BurningDamage()
{
	DurationPolicy = EGameplayEffectDurationType::HasDuration;
	DurationMagnitude = FScalableFloat(5.0f);
	Period = 1.0f;
	bExecutePeriodicEffectOnApplication = false;

	InheritableOwnedTagsContainer.AddTag(PX_GameplayTags::State_Condition_Burning);

	FGameplayModifierInfo DamageModifier;
	DamageModifier.Attribute = UPX_ResourceAttributeSet::GetDamageAttribute();
	DamageModifier.ModifierOp = EGameplayModOp::Additive;
	DamageModifier.ModifierMagnitude = FScalableFloat(5.0f);
	Modifiers.Add(DamageModifier);
}

UPX_GE_SlowDebuff::UPX_GE_SlowDebuff()
{
	constexpr float SlowMoveSpeedMultiplier = 0.5f;

	DurationPolicy = EGameplayEffectDurationType::HasDuration;
	DurationMagnitude = FScalableFloat(5.0f);

	InheritableOwnedTagsContainer.AddTag(PX_GameplayTags::State_Condition_Slowed);

	FGameplayModifierInfo MoveSpeedModifier;
	MoveSpeedModifier.Attribute = UPX_MovementAttributeSet::GetMoveSpeedAttribute();
	MoveSpeedModifier.ModifierOp = EGameplayModOp::Multiplicitive;
	MoveSpeedModifier.ModifierMagnitude = FScalableFloat(SlowMoveSpeedMultiplier);
	Modifiers.Add(MoveSpeedModifier);
}

UPX_GE_StunDebuff::UPX_GE_StunDebuff()
{
	DurationPolicy = EGameplayEffectDurationType::HasDuration;
	DurationMagnitude = FScalableFloat(3.0f);

	InheritableOwnedTagsContainer.AddTag(PX_GameplayTags::State_Condition_Stunned);

	FGameplayModifierInfo MoveSpeedModifier;
	MoveSpeedModifier.Attribute = UPX_MovementAttributeSet::GetMoveSpeedAttribute();
	MoveSpeedModifier.ModifierOp = EGameplayModOp::Override;
	MoveSpeedModifier.ModifierMagnitude = FScalableFloat(0.0f);
	Modifiers.Add(MoveSpeedModifier);
}

UPX_GE_AbilityCooldown::UPX_GE_AbilityCooldown()
{
	DurationPolicy = EGameplayEffectDurationType::HasDuration;
	DurationMagnitude = FScalableFloat(0.0f);
}
