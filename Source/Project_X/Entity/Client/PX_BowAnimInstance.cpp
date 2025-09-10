// Fill out your copyright notice in the Description page of Project Settings.


#include "Entity/Client/PX_BowAnimInstance.h"
#include "Entity/PX_Character.h"
#include "Entity/Client/PX_CharacterAnimInstance.h"

void UPX_BowAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();

    Mesh = GetSkelMeshComponent();
    
    if (USkeletalMeshComponent* SkeletalMeshComponent = Mesh.Get())
    {
        if (USceneComponent* Parent = SkeletalMeshComponent->GetAttachParent())
        {
            if (AActor* ParentOwner = Parent->GetOwner())
            {
                if (APX_Character* PX_Character = Cast<APX_Character>(ParentOwner))
                {
                    Owner = PX_Character;
                }
            }
        }
    }
}

void UPX_BowAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    if (!Mesh.IsValid())
    {
        Mesh = GetSkelMeshComponent();
        if (!Mesh.IsValid()) return;
    }

    if (!Owner.IsValid())
    {
        if (USkeletalMeshComponent* SkeletalMeshComponent = Mesh.Get())
        {
            if (USceneComponent* Parent = SkeletalMeshComponent->GetAttachParent())
            {
                if (AActor* ParentOwner = Parent->GetOwner())
                {
                    if (APX_Character* PX_Character = Cast<APX_Character>(ParentOwner))
                    {
                        Owner = PX_Character;
                    }
                }
            }
        }
        if (!Owner.IsValid()) return;
    }

    if (USkeletalMeshComponent* SkeletalMeshComponent = Owner->GetMesh())
    {
        if (UAnimInstance* AnimInstance = SkeletalMeshComponent->GetAnimInstance())
        {
            if (UPX_CharacterAnimInstance* PX_CharacterAnimInstance = Cast<UPX_CharacterAnimInstance>(AnimInstance))
            {
                bIsAiming = PX_CharacterAnimInstance->GetIsAiming();
                bIsReloading = PX_CharacterAnimInstance->GetIsReloading();
            }
        }
    }

    USkeletalMeshComponent* BowSkeletal = Mesh.Get();
    USkeletalMeshComponent* ChrSkeletal = Owner->GetMesh();
    const FTransform& BowTransform = BowSkeletal->GetComponentTransform();

    if (bIsAiming && !bIsReloading)
    {
        Target = BowTransform.InverseTransformPosition(ChrSkeletal->GetSocketLocation(TEXT("Bow_R")));
    }
    else
    {
        Target = BowTransform.InverseTransformPosition(BowSkeletal->GetSocketLocation(TEXT("bow_string_origin")));
    }
}