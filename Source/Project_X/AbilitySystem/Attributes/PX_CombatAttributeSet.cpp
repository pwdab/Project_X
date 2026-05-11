// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Attributes/PX_CombatAttributeSet.h"
#include "Net/UnrealNetwork.h"
#include "GameplayEffectExtension.h"

UPX_CombatAttributeSet::UPX_CombatAttributeSet()
{
	InitAttackPower(1.f);
	InitDefense(0.f);
	InitDamageReduction(0.f);
	InitCriticalChance(0.f);
	InitCriticalDamageMultiplier(1.5f);
	InitArmorPenetration(0.f);
	InitReloadSpeedMultiplier(1.0f);
	InitAttackRateMultiplier(1.0f);
}

void UPX_CombatAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UPX_CombatAttributeSet, AttackPower, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UPX_CombatAttributeSet, Defense, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UPX_CombatAttributeSet, DamageReduction, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UPX_CombatAttributeSet, CriticalChance, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UPX_CombatAttributeSet, CriticalDamageMultiplier, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UPX_CombatAttributeSet, ArmorPenetration, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UPX_CombatAttributeSet, ReloadSpeedMultiplier, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UPX_CombatAttributeSet, AttackRateMultiplier, COND_None, REPNOTIFY_Always);
}

void UPX_CombatAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	if ( Attribute == GetAttackPowerAttribute() )
	{
		NewValue = FMath::Max(NewValue, 0.f);
	}
	else if ( Attribute == GetDefenseAttribute() )
	{
		NewValue = FMath::Max(NewValue, 0.f);
	}
	else if ( Attribute == GetDamageReductionAttribute() )
	{
		NewValue = FMath::Clamp(NewValue, 0.f, 1.f);
	}
	else if ( Attribute == GetCriticalChanceAttribute() )
	{
		NewValue = FMath::Clamp(NewValue, 0.f, 1.f);
	}
	else if ( Attribute == GetCriticalDamageMultiplierAttribute() )
	{
		NewValue = FMath::Max(NewValue, 1.f);
	}
	else if ( Attribute == GetArmorPenetrationAttribute() )
	{
		NewValue = FMath::Max(NewValue, 0.f);
	}
	else if ( Attribute == GetReloadSpeedMultiplierAttribute() )
	{
		NewValue = FMath::Max(NewValue, 0.1f);
	}
	else if ( Attribute == GetAttackRateMultiplierAttribute() )
	{
		NewValue = FMath::Max(NewValue, 0.1f);
	}
}

void UPX_CombatAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	const FGameplayAttribute ModifiedAttribute = Data.EvaluatedData.Attribute;

	if ( ModifiedAttribute == GetAttackPowerAttribute() )
	{
		SetAttackPower(FMath::Max(GetAttackPower(), 0.f));
	}
	else if ( ModifiedAttribute == GetDefenseAttribute() )
	{
		SetDefense(FMath::Max(GetDefense(), 0.f));
	}
	else if ( ModifiedAttribute == GetDamageReductionAttribute() )
	{
		SetDamageReduction(FMath::Clamp(GetDamageReduction(), 0.f, 1.f));
	}
	else if ( ModifiedAttribute == GetCriticalChanceAttribute() )
	{
		SetCriticalChance(FMath::Clamp(GetCriticalChance(), 0.f, 1.f));
	}
	else if ( ModifiedAttribute == GetCriticalDamageMultiplierAttribute() )
	{
		SetCriticalDamageMultiplier(FMath::Max(GetCriticalDamageMultiplier(), 1.f));
	}
	else if ( ModifiedAttribute == GetArmorPenetrationAttribute() )
	{
		SetArmorPenetration(FMath::Clamp(GetArmorPenetration(), 0.f, 1.f));
	}
	else if ( ModifiedAttribute == GetReloadSpeedMultiplierAttribute() )
	{
		SetReloadSpeedMultiplier(FMath::Max(GetReloadSpeedMultiplier(), 0.1f));
	}
	else if ( ModifiedAttribute == GetAttackRateMultiplierAttribute() )
	{
		SetAttackRateMultiplier(FMath::Max(GetAttackRateMultiplier(), 0.1f));
	}
}

void UPX_CombatAttributeSet::OnRep_AttackPower(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UPX_CombatAttributeSet, AttackPower, OldValue);
}

void UPX_CombatAttributeSet::OnRep_Defense(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UPX_CombatAttributeSet, Defense, OldValue);
}

void UPX_CombatAttributeSet::OnRep_DamageReduction(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UPX_CombatAttributeSet, DamageReduction, OldValue);
}

void UPX_CombatAttributeSet::OnRep_CriticalChance(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UPX_CombatAttributeSet, CriticalChance, OldValue);
}

void UPX_CombatAttributeSet::OnRep_CriticalDamageMultiplier(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UPX_CombatAttributeSet, CriticalDamageMultiplier, OldValue);
}

void UPX_CombatAttributeSet::OnRep_ArmorPenetration(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UPX_CombatAttributeSet, ArmorPenetration, OldValue);
}

void UPX_CombatAttributeSet::OnRep_ReloadSpeedMultiplier(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UPX_CombatAttributeSet, ReloadSpeedMultiplier, OldValue);
}

void UPX_CombatAttributeSet::OnRep_AttackRateMultiplier(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UPX_CombatAttributeSet, AttackRateMultiplier, OldValue);
}