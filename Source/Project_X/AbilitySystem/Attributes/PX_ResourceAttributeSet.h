// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Project_X.h"
#include "AbilitySystem/Attributes/PX_AttributeSetBase.h"
#include "PX_ResourceAttributeSet.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_X_API UPX_ResourceAttributeSet : public UPX_AttributeSetBase
{
	GENERATED_BODY()
	
public:
	UPX_ResourceAttributeSet();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;

	float GetLastHealthDamagedTime() const { return LastHealthDamagedTime; }
	float GetLastShieldDamagedTime() const { return LastShieldDamagedTime; }
	float GetLastStaminaConsumedTime() const { return LastStaminaConsumedTime; }
	float GetHealthRegenDelay() const { return HealthRegenDelay; }
	float GetShieldRegenDelay() const { return ShieldRegenDelay; }
	float GetStaminaRegenDelay() const { return StaminaRegenDelay; }
	bool IsStaminaBeingConsumed(float CurrentTime) const { return CurrentTime < StaminaConsumeLockUntilTime; }
	void NotifyStaminaConsumed() const;

public:
	// ===== Vital =====
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Health, Category = "Resource|Health")
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS(UPX_ResourceAttributeSet, Health)

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxHealth, Category = "Resource|Health")
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS(UPX_ResourceAttributeSet, MaxHealth)

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Shield, Category = "Resource|Shield")
	FGameplayAttributeData Shield;
	ATTRIBUTE_ACCESSORS(UPX_ResourceAttributeSet, Shield)

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxShield, Category = "Resource|Shield")
	FGameplayAttributeData MaxShield;
	ATTRIBUTE_ACCESSORS(UPX_ResourceAttributeSet, MaxShield)

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Stamina, Category = "Resource|Stamina")
	FGameplayAttributeData Stamina;
	ATTRIBUTE_ACCESSORS(UPX_ResourceAttributeSet, Stamina)

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxStamina, Category = "Resource|Stamina")
	FGameplayAttributeData MaxStamina;
	ATTRIBUTE_ACCESSORS(UPX_ResourceAttributeSet, MaxStamina)

	// ===== Regen =====
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_HealthRegenRate, Category = "Resource|Regen")
	FGameplayAttributeData HealthRegenRate;
	ATTRIBUTE_ACCESSORS(UPX_ResourceAttributeSet, HealthRegenRate)

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_ShieldRegenRate, Category = "Resource|Regen")
	FGameplayAttributeData ShieldRegenRate;
	ATTRIBUTE_ACCESSORS(UPX_ResourceAttributeSet, ShieldRegenRate)

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_StaminaRegenRate, Category = "Resource|Regen")
	FGameplayAttributeData StaminaRegenRate;
	ATTRIBUTE_ACCESSORS(UPX_ResourceAttributeSet, StaminaRegenRate)

	// ===== Meta =====
	UPROPERTY(BlueprintReadOnly, Category = "Resource|Meta")
	FGameplayAttributeData Damage;
	ATTRIBUTE_ACCESSORS(UPX_ResourceAttributeSet, Damage)

	UPROPERTY(BlueprintReadOnly, Category = "Resource|Meta")
	FGameplayAttributeData Healing;
	ATTRIBUTE_ACCESSORS(UPX_ResourceAttributeSet, Healing)

	UPROPERTY(BlueprintReadOnly, Category = "Resource|Meta")
	FGameplayAttributeData StaminaCost;
	ATTRIBUTE_ACCESSORS(UPX_ResourceAttributeSet, StaminaCost)

protected:
	UFUNCTION()
	void OnRep_Health(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_MaxHealth(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_Shield(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_MaxShield(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_Stamina(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_MaxStamina(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_HealthRegenRate(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_ShieldRegenRate(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_StaminaRegenRate(const FGameplayAttributeData& OldValue);

private:
	float ResolveWorldTime() const;
	void MarkHealthDamaged() const;
	void MarkShieldDamaged() const;
	void MarkStaminaConsumed() const;

	mutable float LastHealthDamagedTime = -1000.0f;
	mutable float LastShieldDamagedTime = -1000.0f;
	mutable float LastStaminaConsumedTime = -1000.0f;
	mutable float StaminaConsumeLockUntilTime = -1000.0f;
	float HealthRegenDelay = 2.0f;
	float ShieldRegenDelay = 5.0f;
	float StaminaRegenDelay = 2.0f;
	float StaminaConsumeLockDuration = 0.15f;
};
