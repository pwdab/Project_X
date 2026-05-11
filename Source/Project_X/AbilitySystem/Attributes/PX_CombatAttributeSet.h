// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Project_X.h"
#include "AbilitySystem/Attributes/PX_AttributeSetBase.h"
#include "PX_CombatAttributeSet.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_X_API UPX_CombatAttributeSet : public UPX_AttributeSetBase
{
	GENERATED_BODY()
	
public:
	UPX_CombatAttributeSet();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;

public:
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_AttackPower, Category = "Combat")
	FGameplayAttributeData AttackPower;
	ATTRIBUTE_ACCESSORS(UPX_CombatAttributeSet, AttackPower)

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Defense, Category = "Combat")
	FGameplayAttributeData Defense;
	ATTRIBUTE_ACCESSORS(UPX_CombatAttributeSet, Defense)

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_DamageReduction, Category = "Combat")
	FGameplayAttributeData DamageReduction;
	ATTRIBUTE_ACCESSORS(UPX_CombatAttributeSet, DamageReduction)

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_CriticalChance, Category = "Combat")
	FGameplayAttributeData CriticalChance;
	ATTRIBUTE_ACCESSORS(UPX_CombatAttributeSet, CriticalChance)

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_CriticalDamageMultiplier, Category = "Combat")
	FGameplayAttributeData CriticalDamageMultiplier;
	ATTRIBUTE_ACCESSORS(UPX_CombatAttributeSet, CriticalDamageMultiplier)

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_ArmorPenetration, Category = "Combat")
	FGameplayAttributeData ArmorPenetration;
	ATTRIBUTE_ACCESSORS(UPX_CombatAttributeSet, ArmorPenetration)

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_ReloadSpeedMultiplier, Category = "Combat")
	FGameplayAttributeData ReloadSpeedMultiplier;
	ATTRIBUTE_ACCESSORS(UPX_CombatAttributeSet, ReloadSpeedMultiplier)

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_AttackRateMultiplier, Category = "Combat")
	FGameplayAttributeData AttackRateMultiplier;
	ATTRIBUTE_ACCESSORS(UPX_CombatAttributeSet, AttackRateMultiplier)

protected:
	UFUNCTION()
	void OnRep_AttackPower(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_Defense(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_DamageReduction(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_CriticalChance(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_CriticalDamageMultiplier(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_ArmorPenetration(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_ReloadSpeedMultiplier(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_AttackRateMultiplier(const FGameplayAttributeData& OldValue);
	
	
};
