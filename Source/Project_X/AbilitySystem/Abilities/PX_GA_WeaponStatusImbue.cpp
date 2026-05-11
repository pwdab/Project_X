// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/Abilities/PX_GA_WeaponStatusImbue.h"

#include "AbilitySystem/Effects/PX_GE_StatusEffects.h"
#include "AbilitySystem/Tags/PX_GameplayTags.h"
#include "Component/Inventory/PX_WeaponItemInstance.h"
#include "Component/Weapon/PX_WeaponDataAsset.h"
#include "Component/Weapon/PX_WeaponSystemComponent.h"
#include "AbilitySystemComponent.h"
#include "Engine/Texture2D.h"

UPX_GA_WeaponStatusImbueBase::UPX_GA_WeaponStatusImbueBase()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
	ImbueCooldownGameplayEffectClass = UPX_GE_AbilityCooldown::StaticClass();

	ActivationBlockedTags.AddTag(PX_GameplayTags::State_Combat_Equipping);
	ActivationBlockedTags.AddTag(PX_GameplayTags::State_Combat_Unequipping);
	ActivationBlockedTags.AddTag(PX_GameplayTags::State_Combat_Reloading);
	ActivationBlockedTags.AddTag(PX_GameplayTags::State_Condition_Dead);
	ActivationBlockedTags.AddTag(PX_GameplayTags::State_Condition_Down);
	ActivationBlockedTags.AddTag(PX_GameplayTags::State_Condition_Disabled);
	ActivationBlockedTags.AddTag(PX_GameplayTags::State_Condition_Stunned);
}

bool UPX_GA_WeaponStatusImbueBase::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if ( !Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags) )
	{
		PX_LOG(Log, TEXT("Status Imbue CanActivateAbility false: Super failed. Ability: %s"), *GetNameSafe(GetClass()));
		return false;
	}

	const UPX_WeaponSystemComponent* WSC = GetWeaponSystemComponent(ActorInfo);
	if ( !WSC || WSC->GetWeaponSlotIndex() == 4 || !StatusGameplayEffectClass )
	{
		PX_LOG(Log, TEXT("Status Imbue CanActivateAbility false: Invalid weapon/status. Ability: %s"), *GetNameSafe(GetClass()));
		return false;
	}

	const UPX_WeaponItemInstance* WeaponItemInstance = WSC->GetWeaponInstance();
	if ( !WeaponItemInstance || !WeaponItemInstance->GetWeaponDataAsset() )
	{
		PX_LOG(Log, TEXT("Status Imbue CanActivateAbility false: Invalid weapon data. Ability: %s"), *GetNameSafe(GetClass()));
		return false;
	}

	const bool bHasAmmo = WSC->GetCurrentAmmoInMag() > 0;
	if ( !bHasAmmo )
	{
		PX_LOG(Log, TEXT("Status Imbue CanActivateAbility false: No ammo. Ability: %s"), *GetNameSafe(GetClass()));
	}

	return bHasAmmo;
}

void UPX_GA_WeaponStatusImbueBase::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	PX_LOG(Log, TEXT("Activate Status Imbue Ability: %s"), *GetNameSafe(GetClass()));

	if ( !HasAuthority(&ActivationInfo) && ActorInfo && ActorInfo->IsLocallyControlled() && !ShouldUseClientPrediction(ActorInfo, &ActivationInfo) )
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
		return;
	}

	if ( !CommitAbility(Handle, ActorInfo, ActivationInfo) )
	{
		PX_LOG(Log, TEXT("Status Imbue CommitAbility failed. Ability: %s"), *GetNameSafe(GetClass()));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if ( ActorInfo && ActorInfo->AvatarActor.IsValid() && ActorInfo->AvatarActor->HasAuthority() )
	{
		if ( UPX_WeaponSystemComponent* WSC = GetWeaponSystemComponent(ActorInfo) )
		{
			WSC->QueueNextAttackStatusEffect(StatusGameplayEffectClass);
			PX_LOG(Log, TEXT("Queued status effect for next attack. Ability: %s, Effect: %s"), *GetNameSafe(GetClass()), *GetNameSafe(StatusGameplayEffectClass));
		}
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

const FGameplayTagContainer* UPX_GA_WeaponStatusImbueBase::GetCooldownTags() const
{
	TempCooldownTags.Reset();

	if ( const FGameplayTagContainer* ParentCooldownTags = Super::GetCooldownTags() )
	{
		TempCooldownTags.AppendTags(*ParentCooldownTags);
	}

	if ( CooldownTag.IsValid() )
	{
		TempCooldownTags.AddTag(CooldownTag);
	}

	return &TempCooldownTags;
}

void UPX_GA_WeaponStatusImbueBase::ApplyCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const
{
	if ( !ImbueCooldownGameplayEffectClass || !CooldownTag.IsValid() || Cooldown <= 0.0f )
	{
		Super::ApplyCooldown(Handle, ActorInfo, ActivationInfo);
		return;
	}

	const FGameplayEffectSpecHandle CooldownSpecHandle = MakeOutgoingGameplayEffectSpec(ImbueCooldownGameplayEffectClass, GetAbilityLevel(Handle, ActorInfo));
	if ( !CooldownSpecHandle.IsValid() || !CooldownSpecHandle.Data.IsValid() )
	{
		return;
	}

	CooldownSpecHandle.Data->DynamicGrantedTags.AddTag(CooldownTag);
	CooldownSpecHandle.Data->DynamicAssetTags.AddTag(CooldownTag);
	CooldownSpecHandle.Data->SetDuration(Cooldown, true);
	ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, CooldownSpecHandle);
}

UPX_GA_ImbueBurn::UPX_GA_ImbueBurn()
{
	InputTag = PX_GameplayTags::Input_Skill_E;
	AbilityTags.AddTag(PX_GameplayTags::Ability_Weapon_ImbueBurn);
	CooldownTag = PX_GameplayTags::Cooldown_Weapon_ImbueBurn;
	StatusGameplayEffectClass = UPX_GE_BurningDamage::StaticClass();
	SkillIcon = LoadObject<UTexture2D>(nullptr, TEXT("/Game/Project_X/Images/SkillIcon_Fire.SkillIcon_Fire"));
}

UPX_GA_ImbueSlow::UPX_GA_ImbueSlow()
{
	InputTag = PX_GameplayTags::Input_Skill_E;
	AbilityTags.AddTag(PX_GameplayTags::Ability_Weapon_ImbueSlow);
	Cooldown = 3.0f;
	CooldownTag = PX_GameplayTags::Cooldown_Weapon_ImbueSlow;
	StatusGameplayEffectClass = UPX_GE_SlowDebuff::StaticClass();
	SkillIcon = LoadObject<UTexture2D>(nullptr, TEXT("/Game/Project_X/Images/SkillIcon_Slow.SkillIcon_Slow"));
}

UPX_GA_ImbueStun::UPX_GA_ImbueStun()
{
	InputTag = PX_GameplayTags::Input_Skill_E;
	AbilityTags.AddTag(PX_GameplayTags::Ability_Weapon_ImbueStun);
	Cooldown = 10.0f;
	CooldownTag = PX_GameplayTags::Cooldown_Weapon_ImbueStun;
	StatusGameplayEffectClass = UPX_GE_StunDebuff::StaticClass();
	SkillIcon = LoadObject<UTexture2D>(nullptr, TEXT("/Game/Project_X/Images/SkillIcon_Stun.SkillIcon_Stun"));
}
