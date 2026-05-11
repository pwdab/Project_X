// Fill out your copyright notice in the Description page of Project Settings.

#include "Component/Inventory/PX_EquippableItemInstance.h"
#include "Component/Inventory/PX_EquippableItemDataAsset.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/Abilities/PX_GameplayAbilityBase.h"

void UPX_EquippableItemInstance::GiveAbilities(UAbilitySystemComponent* ASC)
{
	if ( !ASC || !ItemDataAsset ) return;
	if ( GivenAbilityHandles.Num() > 0 ) return;

	UPX_EquippableItemDataAsset* EquippableDataAsset = GetEquippableItemDataAsset();
	if ( !EquippableDataAsset ) return;

	// Ability 부여
	for ( const FPX_GrantedAbilityEntry& GrantedAbility : EquippableDataAsset->GrantedAbilities )
	{
		if ( !GrantedAbility.AbilityClass ) continue;

		FGameplayAbilitySpec Spec(GrantedAbility.AbilityClass, 1, INDEX_NONE, this);

		if ( GrantedAbility.InputTag.IsValid() )
		{
			Spec.DynamicAbilityTags.AddTag(GrantedAbility.InputTag);
		}

		const FGameplayAbilitySpecHandle Handle = ASC->GiveAbility(Spec);
		if ( Handle.IsValid() )
		{
			GivenAbilityHandles.Add(Handle);
		}
	}

	PX_LOG(Log, TEXT("Give Ability"));
}

void UPX_EquippableItemInstance::RemoveAbilities(UAbilitySystemComponent* ASC)
{
	if ( !ASC ) return;

	UPX_EquippableItemDataAsset* EquippableDataAsset = GetEquippableItemDataAsset();
	if ( !EquippableDataAsset ) return;

	// Ability 회수
	for ( const FGameplayAbilitySpecHandle& Handle : GivenAbilityHandles )
	{
		if ( Handle.IsValid() )
		{
			ASC->ClearAbility(Handle);
		}
	}
	GivenAbilityHandles.Empty();
	PX_LOG(Log, TEXT("Remove Ability"));
}

void UPX_EquippableItemInstance::AddTags(UAbilitySystemComponent* ASC)
{
	if ( !ASC || !ItemDataAsset ) return;

	UPX_EquippableItemDataAsset* EquippableDataAsset = GetEquippableItemDataAsset();
	if ( !EquippableDataAsset ) return;

	const FGameplayTagContainer& Tags = EquippableDataAsset->GrantedTags;

	if ( Tags.IsEmpty() ) return;

	// Tag 부여
	for ( const FGameplayTag& Tag : Tags )
	{
		if ( Tag.IsValid() )
		{
			ASC->AddLooseGameplayTag(Tag);

			PX_LOG(Log, TEXT("Add Tag: %s"), *Tag.ToString());
		}
	}
}

void UPX_EquippableItemInstance::RemoveTags(UAbilitySystemComponent* ASC)
{
	if ( !ASC || !ItemDataAsset ) return;

	UPX_EquippableItemDataAsset* EquippableDataAsset = GetEquippableItemDataAsset();
	if ( !EquippableDataAsset ) return;

	const FGameplayTagContainer& Tags = EquippableDataAsset->GrantedTags;

	if ( Tags.IsEmpty() ) return;

	// Tag 회수
	for ( const FGameplayTag& Tag : Tags )
	{
		if ( Tag.IsValid() )
		{
			ASC->RemoveLooseGameplayTag(Tag);

			PX_LOG(Log, TEXT("Remove Tag: %s"), *Tag.ToString());
		}
	}
}
