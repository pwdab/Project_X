// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/PX_GameplayAbilityBase.h"
#include "Entity/PX_Character.h"
#include "Component/Weapon/PX_WeaponSystemComponent.h"
#include "Settings/PX_GameplayPredictionSettings.h"

void UPX_GameplayAbilityBase::CancelFromExternal()
{
	if ( IsActive() )
	{
		CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);

		if ( IsActive() )
		{
			EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		}
	}
}

void UPX_GameplayAbilityBase::CancelFromStun()
{
	CancelFromExternal();
}

bool UPX_GameplayAbilityBase::ShouldUseClientPrediction(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo* ActivationInfo) const
{
	if ( !UPX_GameplayPredictionSettings::IsClientPredictionEnabled() || !bUseClientPrediction || !ActorInfo || !ActorInfo->IsLocallyControlled() )
	{
		return false;
	}

	if ( ActivationInfo && HasAuthority(ActivationInfo) )
	{
		return false;
	}

	if ( ActorInfo->AvatarActor.IsValid() && ActorInfo->AvatarActor->HasAuthority() )
	{
		return false;
	}

	return true;
}

APX_Character* UPX_GameplayAbilityBase::GetPXCharacter(const FGameplayAbilityActorInfo* ActorInfo) const
{
    if ( !ActorInfo || !ActorInfo->AvatarActor.IsValid() ) return nullptr;

    return Cast<APX_Character>(ActorInfo->AvatarActor.Get());
}

UPX_WeaponSystemComponent* UPX_GameplayAbilityBase::GetWeaponSystemComponent(const FGameplayAbilityActorInfo* ActorInfo) const
{
    APX_Character* Character = GetPXCharacter(ActorInfo);
    if ( !Character ) return nullptr;

    return Character->GetWeaponSystemComponent();
}

UAbilitySystemComponent* UPX_GameplayAbilityBase::GetAbilitySystemComponent(const FGameplayAbilityActorInfo* ActorInfo) const
{
    if ( !ActorInfo ) return nullptr;

    return ActorInfo->AbilitySystemComponent.Get();
}

