// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/PlayerState/PX_PlayerState.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/Component/PX_AbilitySystemComponent.h"
#include "AbilitySystem/Attributes/PX_ResourceAttributeSet.h"
#include "AbilitySystem/Attributes/PX_CombatAttributeSet.h"
#include "AbilitySystem/Attributes/PX_MovementAttributeSet.h"
#include "AbilitySystem/Abilities/PX_AbilitiesDataAsset.h"
#include "AbilitySystem/Abilities/PX_GameplayAbilityBase.h"
#include "AbilitySystem/Abilities/PX_GA_Locomotion.h"
#include "AbilitySystem/Abilities/PX_GA_WeaponStatusImbue.h"
#include "AbilitySystem/Effects/PX_GE_Regen.h"

APX_PlayerState::APX_PlayerState()
{
    NetUpdateFrequency = 100.0f;
    MinNetUpdateFrequency = 30.0f;

    // Ability System Component
    AbilitySystemComponent = CreateDefaultSubobject<UPX_AbilitySystemComponent>(TEXT("AbilitySystemComponent"));

    // Attribute Set
    ResourceAttributeSet = CreateDefaultSubobject<UPX_ResourceAttributeSet>(TEXT("ResourceSet"));
    CombatAttributeSet = CreateDefaultSubobject<UPX_CombatAttributeSet>(TEXT("CombatSet"));
    MovementAttributeSet = CreateDefaultSubobject<UPX_MovementAttributeSet>(TEXT("MovementSet"));

    AbilitySystemComponent->AddAttributeSetSubobject<UPX_ResourceAttributeSet>(ResourceAttributeSet);
    AbilitySystemComponent->AddAttributeSetSubobject<UPX_CombatAttributeSet>(CombatAttributeSet);
    AbilitySystemComponent->AddAttributeSetSubobject<UPX_MovementAttributeSet>(MovementAttributeSet);

    AbilitySystemComponent->SetIsReplicated(true);
    AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	static ConstructorHelpers::FObjectFinder<UPX_AbilitiesDataAsset> AbilitySetObj(TEXT("/Game/Project_X/Character/PX_Character/Data/DA_PX_DefaultAbilities.DA_PX_DefaultAbilities"));
	if ( AbilitySetObj.Succeeded() )
	{
		DefaultAbilitySet = AbilitySetObj.Object;
	}
}

void APX_PlayerState::GrantDefaultAbilities()
{
	if ( bDefaultAbilitiesGranted ) return;

	if ( !HasAuthority() ) return;

	if ( !AbilitySystemComponent ) return;

	FGameplayEffectContextHandle RegenEffectContext = AbilitySystemComponent->MakeEffectContext();
	RegenEffectContext.AddSourceObject(this);
	const FGameplayEffectSpecHandle RegenSpecHandle = AbilitySystemComponent->MakeOutgoingSpec(UPX_GE_ResourceRegen::StaticClass(), 1.0f, RegenEffectContext);
	if ( RegenSpecHandle.IsValid() )
	{
		AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*RegenSpecHandle.Data.Get());
	}

	auto GiveAbilityIfMissing = [this](TSubclassOf<UGameplayAbility> AbilityClass, int32 AbilityLevel)
	{
		if ( !AbilityClass )
		{
			return;
		}

		for ( const FGameplayAbilitySpec& ExistingSpec : AbilitySystemComponent->GetActivatableAbilities() )
		{
			if ( ExistingSpec.Ability && ExistingSpec.Ability->GetClass()->IsChildOf(AbilityClass) )
			{
				return;
			}
		}

		FGameplayAbilitySpec Spec(AbilityClass, AbilityLevel);

		if ( const UPX_GameplayAbilityBase* AbilityCDO = Cast<UPX_GameplayAbilityBase>(AbilityClass->GetDefaultObject()) )
		{
			if ( AbilityCDO->GetInputTag().IsValid() )
			{
				Spec.DynamicAbilityTags.AddTag(AbilityCDO->GetInputTag());
			}
		}

		AbilitySystemComponent->GiveAbility(Spec);
	};

	if ( DefaultAbilitySet )
	{
		for ( const FPX_AbilityEntry& Entry : DefaultAbilitySet->Abilities )
		{
			if ( !Entry.AbilityClass ) continue;
			if ( Entry.AbilityClass->IsChildOf(UPX_GA_WeaponStatusImbueBase::StaticClass()) ) continue;

			GiveAbilityIfMissing(Entry.AbilityClass, Entry.AbilityLevel);
		}
	}

	GiveAbilityIfMissing(UPX_GA_Jump::StaticClass(), 1);
	GiveAbilityIfMissing(UPX_GA_Walk::StaticClass(), 1);
	GiveAbilityIfMissing(UPX_GA_Sprint::StaticClass(), 1);
	GiveAbilityIfMissing(UPX_GA_Crouch::StaticClass(), 1);

	PX_LOG(Log, TEXT("Give Default Ability Set"));

	bDefaultAbilitiesGranted = true;
}

UAbilitySystemComponent* APX_PlayerState::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}
