// Fill out your copyright notice in the Description page of Project Settings.


#include "PX_AnimNotifyWeaponReloadEnd.h"
#include "GameFramework/Actor.h"
#include "Entity/PX_Character.h"
#include "Component/Weapon/PX_WeaponSystemComponent.h"

DEFINE_LOG_CATEGORY(PX_AnimNotifyWeaponReloadEnd);

void UPX_AnimNotifyWeaponReloadEnd::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
    if ( !MeshComp ) return;
    AActor* Owner = MeshComp->GetOwner();
    if ( !Owner ) return;
    APX_Character* Character = Cast<APX_Character>(Owner->GetOwner());
    if ( !Character || Character->HasAuthority() || !Character->IsLocallyControlled() || Character->GetLocalRole() != ROLE_AutonomousProxy ) return;

    if ( UPX_WeaponSystemComponent* WeaponSystemComponent = Character->GetWeaponSystemComponent() )
    {
        //PX_LOG(Log, TEXT(""));
        UE_LOG(LogTemp, Log, TEXT("Client Reload End"));
        WeaponSystemComponent->ClientSubmitAction(FPXWeaponActionContext::Builder(EPXWeaponActionType::EndReload));
    }
}


