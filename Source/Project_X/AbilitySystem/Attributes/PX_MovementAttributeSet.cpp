// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Attributes/PX_MovementAttributeSet.h"
#include "Net/UnrealNetwork.h"
#include "GameplayEffectExtension.h"

UPX_MovementAttributeSet::UPX_MovementAttributeSet()
{
	InitMoveSpeed(600.f);
	InitSprintSpeedMultiplier(1.5f);
	InitAccelerationMultiplier(1.0f);
	InitDecelerationMultiplier(1.0f);
	InitRotationRateMultiplier(1.0f);
	InitJumpZVelocity(420.f);
	InitAirControl(0.35f);
	InitGravityScale(1.0f);
}

void UPX_MovementAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UPX_MovementAttributeSet, MoveSpeed, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UPX_MovementAttributeSet, SprintSpeedMultiplier, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UPX_MovementAttributeSet, AccelerationMultiplier, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UPX_MovementAttributeSet, DecelerationMultiplier, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UPX_MovementAttributeSet, RotationRateMultiplier, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UPX_MovementAttributeSet, JumpZVelocity, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UPX_MovementAttributeSet, AirControl, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UPX_MovementAttributeSet, GravityScale, COND_None, REPNOTIFY_Always);
}

void UPX_MovementAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	if ( Attribute == GetMoveSpeedAttribute() )
	{
		NewValue = FMath::Max(NewValue, 0.f);
	}
	else if ( Attribute == GetSprintSpeedMultiplierAttribute() )
	{
		NewValue = FMath::Max(NewValue, 0.f);
	}
	else if ( Attribute == GetAccelerationMultiplierAttribute() )
	{
		NewValue = FMath::Max(NewValue, 0.f);
	}
	else if ( Attribute == GetDecelerationMultiplierAttribute() )
	{
		NewValue = FMath::Max(NewValue, 0.f);
	}
	else if ( Attribute == GetRotationRateMultiplierAttribute() )
	{
		NewValue = FMath::Max(NewValue, 0.f);
	}
	else if ( Attribute == GetJumpZVelocityAttribute() )
	{
		NewValue = FMath::Max(NewValue, 0.f);
	}
	else if ( Attribute == GetAirControlAttribute() )
	{
		NewValue = FMath::Clamp(NewValue, 0.f, 1.f);
	}
	else if ( Attribute == GetGravityScaleAttribute() )
	{
		NewValue = FMath::Max(NewValue, 0.f);
	}
}

void UPX_MovementAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	const FGameplayAttribute ModifiedAttribute = Data.EvaluatedData.Attribute;

	if ( ModifiedAttribute == GetMoveSpeedAttribute() )
	{
		SetMoveSpeed(FMath::Max(GetMoveSpeed(), 0.f));
	}
	else if ( ModifiedAttribute == GetSprintSpeedMultiplierAttribute() )
	{
		SetSprintSpeedMultiplier(FMath::Max(GetSprintSpeedMultiplier(), 0.f));
	}
	else if ( ModifiedAttribute == GetAccelerationMultiplierAttribute() )
	{
		SetAccelerationMultiplier(FMath::Max(GetAccelerationMultiplier(), 0.f));
	}
	else if ( ModifiedAttribute == GetDecelerationMultiplierAttribute() )
	{
		SetDecelerationMultiplier(FMath::Max(GetDecelerationMultiplier(), 0.f));
	}
	else if ( ModifiedAttribute == GetRotationRateMultiplierAttribute() )
	{
		SetRotationRateMultiplier(FMath::Max(GetRotationRateMultiplier(), 0.f));
	}
	else if ( ModifiedAttribute == GetJumpZVelocityAttribute() )
	{
		SetJumpZVelocity(FMath::Max(GetJumpZVelocity(), 0.f));
	}
	else if ( ModifiedAttribute == GetAirControlAttribute() )
	{
		SetAirControl(FMath::Clamp(GetAirControl(), 0.f, 1.f));
	}
	else if ( ModifiedAttribute == GetGravityScaleAttribute() )
	{
		SetGravityScale(FMath::Max(GetGravityScale(), 0.f));
	}
}

void UPX_MovementAttributeSet::OnRep_MoveSpeed(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UPX_MovementAttributeSet, MoveSpeed, OldValue);
}

void UPX_MovementAttributeSet::OnRep_SprintSpeedMultiplier(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UPX_MovementAttributeSet, SprintSpeedMultiplier, OldValue);
}

void UPX_MovementAttributeSet::OnRep_AccelerationMultiplier(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UPX_MovementAttributeSet, AccelerationMultiplier, OldValue);
}

void UPX_MovementAttributeSet::OnRep_DecelerationMultiplier(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UPX_MovementAttributeSet, DecelerationMultiplier, OldValue);
}

void UPX_MovementAttributeSet::OnRep_RotationRateMultiplier(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UPX_MovementAttributeSet, RotationRateMultiplier, OldValue);
}

void UPX_MovementAttributeSet::OnRep_JumpZVelocity(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UPX_MovementAttributeSet, JumpZVelocity, OldValue);
}

void UPX_MovementAttributeSet::OnRep_AirControl(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UPX_MovementAttributeSet, AirControl, OldValue);
}

void UPX_MovementAttributeSet::OnRep_GravityScale(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UPX_MovementAttributeSet, GravityScale, OldValue);
}


