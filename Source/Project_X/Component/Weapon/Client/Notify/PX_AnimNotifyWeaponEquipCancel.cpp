// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/Weapon/Client/Notify/PX_AnimNotifyWeaponEquipCancel.h"
#include "GameFramework/Actor.h"
#include "Entity/PX_Character.h"
#include "Component/Weapon/PX_WeaponSystemComponent.h"

DEFINE_LOG_CATEGORY(PX_AnimNotifyWeaponEquipCancel);

void UPX_AnimNotifyWeaponEquipCancel::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
    if ( !MeshComp ) return;
    AActor* Owner = MeshComp->GetOwner();
    if ( !Owner ) return;
    APX_Character* Character = Cast<APX_Character>(Owner);
    if ( !Character || Character->HasAuthority() || !Character->IsLocallyControlled() || Character->GetLocalRole() != ROLE_AutonomousProxy ) return;

    /*
    UAbilitySystemComponent* ASC = Character->GetAbilitySystemComponent();
    if ( !ASC ) return;

    const FGameplayTag EquipAbilityTag = PX_GameplayTags::Ability_Weapon_Equip;

    TArray<FGameplayAbilitySpec*> MatchingSpecs;
    ASC->GetActivatableGameplayAbilitySpecsByAllMatchingTags(FGameplayTagContainer(EquipAbilityTag), MatchingSpecs, false);

    for ( FGameplayAbilitySpec* Spec : MatchingSpecs )
    {
        if ( !Spec || !Spec->IsActive() ) continue;

        TArray<UGameplayAbility*> AbilityInstances = Spec->GetAbilityInstances();
        for ( UGameplayAbility* AbilityInstance : AbilityInstances )
        {
            UPX_GameplayAbility_Equip* EquipAbility = Cast<UPX_GameplayAbility_Equip>(AbilityInstance);
            if ( !EquipAbility ) continue;

            EquipAbility->SetEquipCancelable(true);
            return;
        }
    }
    */

    UPX_WeaponSystemComponent* WSC = Character->GetWeaponSystemComponent();
    if ( !WSC ) return;

    WSC->SetIsEquipCancelable(true);
}


