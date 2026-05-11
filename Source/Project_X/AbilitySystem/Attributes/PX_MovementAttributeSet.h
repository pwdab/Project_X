// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Project_X.h"
#include "AbilitySystem/Attributes/PX_AttributeSetBase.h"
#include "PX_MovementAttributeSet.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_X_API UPX_MovementAttributeSet : public UPX_AttributeSetBase
{
	GENERATED_BODY()
	
public:
	UPX_MovementAttributeSet();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;

public:
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MoveSpeed, Category = "Movement")
	FGameplayAttributeData MoveSpeed;
	ATTRIBUTE_ACCESSORS(UPX_MovementAttributeSet, MoveSpeed)

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_SprintSpeedMultiplier, Category = "Movement")
	FGameplayAttributeData SprintSpeedMultiplier;
	ATTRIBUTE_ACCESSORS(UPX_MovementAttributeSet, SprintSpeedMultiplier)

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_AccelerationMultiplier, Category = "Movement")
	FGameplayAttributeData AccelerationMultiplier;
	ATTRIBUTE_ACCESSORS(UPX_MovementAttributeSet, AccelerationMultiplier)

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_DecelerationMultiplier, Category = "Movement")
	FGameplayAttributeData DecelerationMultiplier;
	ATTRIBUTE_ACCESSORS(UPX_MovementAttributeSet, DecelerationMultiplier)

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_RotationRateMultiplier, Category = "Movement")
	FGameplayAttributeData RotationRateMultiplier;
	ATTRIBUTE_ACCESSORS(UPX_MovementAttributeSet, RotationRateMultiplier)

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_JumpZVelocity, Category = "Movement")
	FGameplayAttributeData JumpZVelocity;
	ATTRIBUTE_ACCESSORS(UPX_MovementAttributeSet, JumpZVelocity)

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_AirControl, Category = "Movement")
	FGameplayAttributeData AirControl;
	ATTRIBUTE_ACCESSORS(UPX_MovementAttributeSet, AirControl)

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_GravityScale, Category = "Movement")
	FGameplayAttributeData GravityScale;
	ATTRIBUTE_ACCESSORS(UPX_MovementAttributeSet, GravityScale)
	
protected:
	UFUNCTION()
	void OnRep_MoveSpeed(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_SprintSpeedMultiplier(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_AccelerationMultiplier(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_DecelerationMultiplier(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_RotationRateMultiplier(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_JumpZVelocity(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_AirControl(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_GravityScale(const FGameplayAttributeData& OldValue);
};
