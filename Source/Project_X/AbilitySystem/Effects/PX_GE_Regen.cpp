// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/Effects/PX_GE_Regen.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/Attributes/PX_ResourceAttributeSet.h"

namespace PXResourceRegen
{
	constexpr float Period = 0.1f;

	struct FStatics
	{
		DECLARE_ATTRIBUTE_CAPTUREDEF(Health);
		DECLARE_ATTRIBUTE_CAPTUREDEF(MaxHealth);
		DECLARE_ATTRIBUTE_CAPTUREDEF(HealthRegenRate);
		DECLARE_ATTRIBUTE_CAPTUREDEF(Shield);
		DECLARE_ATTRIBUTE_CAPTUREDEF(MaxShield);
		DECLARE_ATTRIBUTE_CAPTUREDEF(ShieldRegenRate);
		DECLARE_ATTRIBUTE_CAPTUREDEF(Stamina);
		DECLARE_ATTRIBUTE_CAPTUREDEF(MaxStamina);
		DECLARE_ATTRIBUTE_CAPTUREDEF(StaminaRegenRate);

		FStatics()
		{
			DEFINE_ATTRIBUTE_CAPTUREDEF(UPX_ResourceAttributeSet, Health, Target, false);
			DEFINE_ATTRIBUTE_CAPTUREDEF(UPX_ResourceAttributeSet, MaxHealth, Target, false);
			DEFINE_ATTRIBUTE_CAPTUREDEF(UPX_ResourceAttributeSet, HealthRegenRate, Target, false);
			DEFINE_ATTRIBUTE_CAPTUREDEF(UPX_ResourceAttributeSet, Shield, Target, false);
			DEFINE_ATTRIBUTE_CAPTUREDEF(UPX_ResourceAttributeSet, MaxShield, Target, false);
			DEFINE_ATTRIBUTE_CAPTUREDEF(UPX_ResourceAttributeSet, ShieldRegenRate, Target, false);
			DEFINE_ATTRIBUTE_CAPTUREDEF(UPX_ResourceAttributeSet, Stamina, Target, false);
			DEFINE_ATTRIBUTE_CAPTUREDEF(UPX_ResourceAttributeSet, MaxStamina, Target, false);
			DEFINE_ATTRIBUTE_CAPTUREDEF(UPX_ResourceAttributeSet, StaminaRegenRate, Target, false);
		}
	};

	const FStatics& Statics()
	{
		static FStatics Instance;
		return Instance;
	}
}

UPX_ExecCalc_ResourceRegen::UPX_ExecCalc_ResourceRegen()
{
	const PXResourceRegen::FStatics& Statics = PXResourceRegen::Statics();

	RelevantAttributesToCapture.Add(Statics.HealthDef);
	RelevantAttributesToCapture.Add(Statics.MaxHealthDef);
	RelevantAttributesToCapture.Add(Statics.HealthRegenRateDef);
	RelevantAttributesToCapture.Add(Statics.ShieldDef);
	RelevantAttributesToCapture.Add(Statics.MaxShieldDef);
	RelevantAttributesToCapture.Add(Statics.ShieldRegenRateDef);
	RelevantAttributesToCapture.Add(Statics.StaminaDef);
	RelevantAttributesToCapture.Add(Statics.MaxStaminaDef);
	RelevantAttributesToCapture.Add(Statics.StaminaRegenRateDef);
}

void UPX_ExecCalc_ResourceRegen::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	UAbilitySystemComponent* TargetASC = ExecutionParams.GetTargetAbilitySystemComponent();
	const UPX_ResourceAttributeSet* ResourceSet = TargetASC ? TargetASC->GetSet<UPX_ResourceAttributeSet>() : nullptr;
	if ( !TargetASC || !ResourceSet )
	{
		return;
	}

	const UWorld* World = TargetASC->GetWorld();
	if ( !World )
	{
		return;
	}

	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();
	FAggregatorEvaluateParameters EvaluateParameters;
	EvaluateParameters.SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	EvaluateParameters.TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	const PXResourceRegen::FStatics& Statics = PXResourceRegen::Statics();
	float Health = 0.0f;
	float MaxHealth = 0.0f;
	float HealthRegenRate = 0.0f;
	float Shield = 0.0f;
	float MaxShield = 0.0f;
	float ShieldRegenRate = 0.0f;
	float Stamina = 0.0f;
	float MaxStamina = 0.0f;
	float StaminaRegenRate = 0.0f;

	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(Statics.HealthDef, EvaluateParameters, Health);
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(Statics.MaxHealthDef, EvaluateParameters, MaxHealth);
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(Statics.HealthRegenRateDef, EvaluateParameters, HealthRegenRate);
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(Statics.ShieldDef, EvaluateParameters, Shield);
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(Statics.MaxShieldDef, EvaluateParameters, MaxShield);
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(Statics.ShieldRegenRateDef, EvaluateParameters, ShieldRegenRate);
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(Statics.StaminaDef, EvaluateParameters, Stamina);
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(Statics.MaxStaminaDef, EvaluateParameters, MaxStamina);
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(Statics.StaminaRegenRateDef, EvaluateParameters, StaminaRegenRate);

	const float CurrentTime = World->GetTimeSeconds();

	if ( Health < MaxHealth && HealthRegenRate > 0.0f && CurrentTime - ResourceSet->GetLastHealthDamagedTime() >= ResourceSet->GetHealthRegenDelay() )
	{
		const float RegenAmount = FMath::Min(HealthRegenRate * PXResourceRegen::Period, MaxHealth - Health);
		OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(UPX_ResourceAttributeSet::GetHealthAttribute(), EGameplayModOp::Additive, RegenAmount));
	}

	if ( Shield < MaxShield && ShieldRegenRate > 0.0f && CurrentTime - ResourceSet->GetLastShieldDamagedTime() >= ResourceSet->GetShieldRegenDelay() )
	{
		const float RegenAmount = FMath::Min(ShieldRegenRate * PXResourceRegen::Period, MaxShield - Shield);
		OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(UPX_ResourceAttributeSet::GetShieldAttribute(), EGameplayModOp::Additive, RegenAmount));
	}

	if ( Stamina < MaxStamina
		&& StaminaRegenRate > 0.0f
		&& !ResourceSet->IsStaminaBeingConsumed(CurrentTime)
		&& CurrentTime - ResourceSet->GetLastStaminaConsumedTime() >= ResourceSet->GetStaminaRegenDelay() )
	{
		const float RegenAmount = FMath::Min(StaminaRegenRate * PXResourceRegen::Period, MaxStamina - Stamina);
		OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(UPX_ResourceAttributeSet::GetStaminaAttribute(), EGameplayModOp::Additive, RegenAmount));
	}
}

UPX_GE_ResourceRegen::UPX_GE_ResourceRegen()
{
	DurationPolicy = EGameplayEffectDurationType::Infinite;
	Period = PXResourceRegen::Period;
	bExecutePeriodicEffectOnApplication = false;

	FGameplayEffectExecutionDefinition ExecutionDefinition;
	ExecutionDefinition.CalculationClass = UPX_ExecCalc_ResourceRegen::StaticClass();
	Executions.Add(ExecutionDefinition);
}
