// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Attributes/PX_ResourceAttributeSet.h"
#include "Net/UnrealNetwork.h"
#include "GameplayEffectExtension.h"

UPX_ResourceAttributeSet::UPX_ResourceAttributeSet()
{
	InitHealth(200.f);
	InitMaxHealth(200.f);

	InitShield(200.f);
	InitMaxShield(200.f);

	InitStamina(100.f);
	InitMaxStamina(100.f);

	InitHealthRegenRate(10.f);
	InitShieldRegenRate(20.f);
	InitStaminaRegenRate(0.f);

	InitDamage(0.f);
	InitHealing(0.f);
	InitStaminaCost(0.f);
}

void UPX_ResourceAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UPX_ResourceAttributeSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UPX_ResourceAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UPX_ResourceAttributeSet, Shield, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UPX_ResourceAttributeSet, MaxShield, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UPX_ResourceAttributeSet, Stamina, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UPX_ResourceAttributeSet, MaxStamina, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UPX_ResourceAttributeSet, HealthRegenRate, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UPX_ResourceAttributeSet, ShieldRegenRate, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UPX_ResourceAttributeSet, StaminaRegenRate, COND_None, REPNOTIFY_Always);
}

void UPX_ResourceAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	if ( Attribute == GetHealthAttribute() )
	{
		NewValue = ClampToRange(NewValue, 0.f, GetMaxHealth());
	}
	else if ( Attribute == GetShieldAttribute() )
	{
		NewValue = ClampToRange(NewValue, 0.f, GetMaxShield());
	}
	else if ( Attribute == GetStaminaAttribute() )
	{
		NewValue = ClampToRange(NewValue, 0.f, GetMaxStamina());
	}
	else if ( Attribute == GetMaxHealthAttribute() )
	{
		NewValue = FMath::Max(NewValue, 1.f);
	}
	else if ( Attribute == GetMaxShieldAttribute() )
	{
		NewValue = FMath::Max(NewValue, 0.f);
	}
	else if ( Attribute == GetMaxStaminaAttribute() )
	{
		NewValue = FMath::Max(NewValue, 0.f);
	}
}

void UPX_ResourceAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	const FGameplayAttribute& ModifiedAttribute = Data.EvaluatedData.Attribute;
	const float EvaluatedMagnitude = Data.EvaluatedData.Magnitude;

	if ( ModifiedAttribute == GetDamageAttribute() )
	{
		const float LocalDamage = GetDamage();
		SetDamage(0.f);

		if ( LocalDamage > 0.f )
		{
			MarkHealthDamaged();
			MarkShieldDamaged();

			float RemainingDamage = LocalDamage;

			const float CurrentShield = GetShield();
			if ( CurrentShield > 0.f )
			{
				const float NewShield = FMath::Max(CurrentShield - RemainingDamage, 0.f);
				RemainingDamage = FMath::Max(RemainingDamage - CurrentShield, 0.f);
				SetShield(NewShield);
			}

			if ( RemainingDamage > 0.f )
			{
				const float NewHealth = FMath::Clamp(GetHealth() - RemainingDamage, 0.f, GetMaxHealth());
				SetHealth(NewHealth);
			}
		}
	}
	else if ( ModifiedAttribute == GetHealingAttribute() )
	{
		const float LocalHealing = GetHealing();
		SetHealing(0.f);

		if ( LocalHealing > 0.f )
		{
			SetHealth(FMath::Clamp(GetHealth() + LocalHealing, 0.f, GetMaxHealth()));
		}
	}
	else if ( ModifiedAttribute == GetStaminaCostAttribute() )
	{
		const float LocalCost = GetStaminaCost();
		SetStaminaCost(0.f);

		if ( LocalCost > 0.f )
		{
			MarkStaminaConsumed();
			SetStamina(FMath::Clamp(GetStamina() - LocalCost, 0.f, GetMaxStamina()));
		}
	}
	else if ( ModifiedAttribute == GetHealthAttribute() )
	{
		if ( EvaluatedMagnitude < -KINDA_SMALL_NUMBER )
		{
			MarkHealthDamaged();
			MarkShieldDamaged();
		}

		SetHealth(FMath::Clamp(GetHealth(), 0.f, GetMaxHealth()));
	}
	else if ( ModifiedAttribute == GetShieldAttribute() )
	{
		if ( EvaluatedMagnitude < -KINDA_SMALL_NUMBER )
		{
			MarkHealthDamaged();
			MarkShieldDamaged();
		}

		SetShield(FMath::Clamp(GetShield(), 0.f, GetMaxShield()));
	}
	else if ( ModifiedAttribute == GetStaminaAttribute() )
	{
		if ( EvaluatedMagnitude < -KINDA_SMALL_NUMBER )
		{
			MarkStaminaConsumed();
		}

		SetStamina(FMath::Clamp(GetStamina(), 0.f, GetMaxStamina()));
	}
	else if ( ModifiedAttribute == GetMaxHealthAttribute() )
	{
		SetHealth(FMath::Clamp(GetHealth(), 0.f, GetMaxHealth()));
	}
	else if ( ModifiedAttribute == GetMaxShieldAttribute() )
	{
		SetShield(FMath::Clamp(GetShield(), 0.f, GetMaxShield()));
	}
	else if ( ModifiedAttribute == GetMaxStaminaAttribute() )
	{
		SetStamina(FMath::Clamp(GetStamina(), 0.f, GetMaxStamina()));
	}
}

void UPX_ResourceAttributeSet::NotifyStaminaConsumed() const
{
	MarkStaminaConsumed();
}

float UPX_ResourceAttributeSet::ResolveWorldTime() const
{
	if ( const UWorld* World = GetWorld() )
	{
		return World->GetTimeSeconds();
	}

	return 0.0f;
}

void UPX_ResourceAttributeSet::MarkHealthDamaged() const
{
	LastHealthDamagedTime = ResolveWorldTime();
}

void UPX_ResourceAttributeSet::MarkShieldDamaged() const
{
	LastShieldDamagedTime = ResolveWorldTime();
}

void UPX_ResourceAttributeSet::MarkStaminaConsumed() const
{
	const float CurrentTime = ResolveWorldTime();
	LastStaminaConsumedTime = CurrentTime;
	StaminaConsumeLockUntilTime = CurrentTime + StaminaConsumeLockDuration;
}

void UPX_ResourceAttributeSet::OnRep_Health(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UPX_ResourceAttributeSet, Health, OldValue);
}

void UPX_ResourceAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UPX_ResourceAttributeSet, MaxHealth, OldValue);
}

void UPX_ResourceAttributeSet::OnRep_Shield(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UPX_ResourceAttributeSet, Shield, OldValue);
}

void UPX_ResourceAttributeSet::OnRep_MaxShield(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UPX_ResourceAttributeSet, MaxShield, OldValue);
}

void UPX_ResourceAttributeSet::OnRep_Stamina(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UPX_ResourceAttributeSet, Stamina, OldValue);
}

void UPX_ResourceAttributeSet::OnRep_MaxStamina(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UPX_ResourceAttributeSet, MaxStamina, OldValue);
}

void UPX_ResourceAttributeSet::OnRep_HealthRegenRate(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UPX_ResourceAttributeSet, HealthRegenRate, OldValue);
}

void UPX_ResourceAttributeSet::OnRep_ShieldRegenRate(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UPX_ResourceAttributeSet, ShieldRegenRate, OldValue);
}

void UPX_ResourceAttributeSet::OnRep_StaminaRegenRate(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UPX_ResourceAttributeSet, StaminaRegenRate, OldValue);
}



