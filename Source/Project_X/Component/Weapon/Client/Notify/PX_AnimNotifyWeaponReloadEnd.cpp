// Fill out your copyright notice in the Description page of Project Settings.


#include "PX_AnimNotifyWeaponReloadEnd.h"
#include "GameFramework/Actor.h"
#include "Entity/PX_Character.h"
//#include "Component/Weapon/PX_WeaponSystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/Tags/PX_GamePlayTags.h"

DEFINE_LOG_CATEGORY(PX_AnimNotifyWeaponReloadEnd);

void UPX_AnimNotifyWeaponReloadEnd::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
    if ( !MeshComp ) return;
    AActor* Owner = MeshComp->GetOwner();
    if ( !Owner ) return;
    APX_Character* Character = Cast<APX_Character>(Owner);
    //if ( !Character || Character->HasAuthority() || !Character->IsLocallyControlled() || Character->GetLocalRole() != ROLE_AutonomousProxy ) return;
    if ( !Character ) return;

    /* WSC에서 Ability를 호출하도록 변경
    if ( UPX_WeaponSystemComponent* WeaponSystemComponent = Character->GetWeaponSystemComponent() )
    {
        //PX_LOG(Log, TEXT(""));
        UE_LOG(LogTemp, Log, TEXT("Client Reload End"));
        WeaponSystemComponent->ClientSubmitAction(FPXWeaponActionContext::Builder(EPXWeaponActionType::EndReload));
    }
    */
    UAbilitySystemComponent* ASC = Character->GetAbilitySystemComponent();
    if ( !ASC ) return;

    // End Equip Event를 전송
    FGameplayEventData EventData;
    EventData.EventTag = PX_GameplayTags::Event_Weapon_Reload_End;
    EventData.Instigator = Character;
    EventData.Target = Character;
    EventData.OptionalObject = nullptr;
    ASC->HandleGameplayEvent(EventData.EventTag, &EventData);

    PX_LOG(Log, TEXT("Client Reload End Notify -> Gameplay Event"));
}


