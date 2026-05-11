// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Attributes/PX_AttributeSetBase.h"
#include "Net/UnrealNetwork.h"

void UPX_AttributeSetBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

void UPX_AttributeSetBase::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);
}

void UPX_AttributeSetBase::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);
}

float UPX_AttributeSetBase::ClampToRange(float Value, float MinValue, float MaxValue)
{
	return FMath::Clamp(Value, MinValue, MaxValue);
}


