// Fill out your copyright notice in the Description page of Project Settings.


#include "PX_AnimNotifyWeaponCommit.h"
#include "GameFramework/Actor.h"
#include "Entity/PX_Character.h"
#include "Component/Weapon/PX_WeaponSystemComponent.h"

void UPX_AnimNotifyWeaponCommit::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
    if ( !MeshComp ) return;
    AActor* Owner = MeshComp->GetOwner();
    if ( !Owner ) return;
    APX_Character* Character = Cast<APX_Character>(Owner->GetOwner());
    if ( !Character ) return;
    if ( !Character || Character->HasAuthority() || !Character->IsLocallyControlled() || Character->GetLocalRole() != ROLE_AutonomousProxy ) return;

    if ( UPX_WeaponSystemComponent* WeaponSystemComponent = Character->GetWeaponSystemComponent() )
    {
        //WeaponSystemComponent->NotifyActionCommit();
    }
}


