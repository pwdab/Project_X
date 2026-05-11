// Fill out your copyright notice in the Description page of Project Settings.

#include "PX_WeaponSystemComponent.h"
#include "Net/UnrealNetwork.h"
#include "Entity/PX_Character.h"
#include "Component/Inventory/PX_InventoryComponent.h"
#include "AbilitySystem/Tags/PX_GamePlayTags.h"
#include "AbilitySystem/Abilities/PX_GameplayAbility_Equip.h"

UPX_WeaponSystemComponent::UPX_WeaponSystemComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    SetIsReplicatedByDefault(true);
}

void UPX_WeaponSystemComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(UPX_WeaponSystemComponent, CurrentWeaponSlotIndex);
    DOREPLIFETIME(UPX_WeaponSystemComponent, CurrentWeaponItemInstanceId);
    DOREPLIFETIME(UPX_WeaponSystemComponent, CurrentWeapon);
}

void UPX_WeaponSystemComponent::BeginPlay()
{
    Super::BeginPlay();

    Character = Cast<ACharacter>(GetOwner());
    if ( !Character ) return;

    APX_Character* PX_Character = Cast<APX_Character>(Character);
    if ( !PX_Character ) return;

    Inventory = PX_Character->GetInventoryComponent();

    if ( !Character->HasAuthority() && Character->IsLocallyControlled() )
    {
        if ( !Inventory ) return;

        if ( Inventory->IsInventoryReady() )
        {
            PX_LOG(Log, TEXT("Inventory is Already Ready. Not using delegate."));
            ServerEquipBySlot(4);
        }
        else
        {
            PX_LOG(Log, TEXT("Inventory is not Ready. Using delegate."));
            Inventory->OnInventoryReady.AddUObject(this, &UPX_WeaponSystemComponent::HandleInventoryReady);
        }
    }
}

void UPX_WeaponSystemComponent::TickComponent(float DeltaSeconds, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaSeconds, TickType, ThisTickFunction);

    if ( Character->HasAuthority() )
    {
        //Server_TickComponent(DeltaSeconds, TickType, ThisTickFunction);
    }

    if ( Character->IsLocallyControlled() )
    {
        //Client_TickComponent(DeltaSeconds, TickType, ThisTickFunction);
    }
}

bool UPX_WeaponSystemComponent::Local_IsEquipCancelable() const
{
    if ( !Character ) return false;

    APX_Character* PX_Character = Cast<APX_Character>(Character);
    if ( !PX_Character )
    {
        PX_LOG(Warning, TEXT("Invalid PX_Character"));
        return false;
    }

    UAbilitySystemComponent* ASC = PX_Character->GetAbilitySystemComponent();
    if ( !ASC )
    {
        PX_LOG(Warning, TEXT("Invalid ASC"));
        return false;
    }

    FGameplayTagContainer TagContainer;
    TagContainer.AddTag(PX_GameplayTags::Ability_Weapon_Equip);

    TArray<FGameplayAbilitySpec*> MatchingSpecs;
    ASC->GetActivatableGameplayAbilitySpecsByAllMatchingTags(TagContainer, MatchingSpecs, false);

    for ( FGameplayAbilitySpec* Spec : MatchingSpecs )
    {
        if ( !Spec || !Spec->IsActive() ) continue;

        for ( UGameplayAbility* Instance : Spec->GetAbilityInstances() )
        {
            if ( UPX_GameplayAbility_Equip* EquipGA = Cast<UPX_GameplayAbility_Equip>(Instance) )
            {
                return EquipGA->IsEquipCancelable();
            }
        }
    }

    return false;
}

void UPX_WeaponSystemComponent::QueueNextAttackStatusEffect(TSubclassOf<UGameplayEffect> StatusEffectClass)
{
    if ( !StatusEffectClass )
    {
        return;
    }

    PendingNextAttackStatusEffects.Reset();
    PendingNextAttackStatusEffects.Add(StatusEffectClass);
}

TArray<TSubclassOf<UGameplayEffect>> UPX_WeaponSystemComponent::ConsumeNextAttackStatusEffects()
{
    TArray<TSubclassOf<UGameplayEffect>> Result = PendingNextAttackStatusEffects;
    PendingNextAttackStatusEffects.Reset();
    return Result;
}

void UPX_WeaponSystemComponent::Local_SetIsEquipCancelable(bool InValue)
{
    PX_LOG(Log, TEXT(""));

    APX_Character* PXCharacter = Cast<APX_Character>(Character);
    if ( !PXCharacter )
    {
        PX_LOG(Warning, TEXT("Invalid PXCharacter"));
        return;
    }

    UAbilitySystemComponent* ASC = PXCharacter->GetAbilitySystemComponent();
    if ( !ASC )
    {
        PX_LOG(Warning, TEXT("ASC is null"));
        return;
    }

    FGameplayTagContainer TagContainer;
    TagContainer.AddTag(PX_GameplayTags::Ability_Weapon_Equip);

    TArray<FGameplayAbilitySpec*> MatchingSpecs;
    ASC->GetActivatableGameplayAbilitySpecsByAllMatchingTags(TagContainer, MatchingSpecs, false);

    for ( FGameplayAbilitySpec* Spec : MatchingSpecs )
    {
        if ( !Spec || !Spec->IsActive() ) continue;

        TArray<UGameplayAbility*> AbilityInstances = Spec->GetAbilityInstances();
        for ( UGameplayAbility* AbilityInstance : AbilityInstances )
        {
            UPX_GameplayAbility_Equip* EquipAbility = Cast<UPX_GameplayAbility_Equip>(AbilityInstance);
            if ( !EquipAbility ) continue;

            EquipAbility->SetEquipCancelable(InValue);
            return;
        }
    }

    PX_LOG(Warning, TEXT("No active equip ability instance found"));
}
