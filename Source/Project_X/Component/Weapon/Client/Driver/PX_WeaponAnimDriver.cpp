// Fill out your copyright notice in the Description page of Project Settings.

#include "PX_WeaponAnimDriver.h"
#include "Component/Weapon/PX_WeaponDataAsset.h"

DEFINE_LOG_CATEGORY(PX_WeaponAnimDriver);

void UPX_WeaponAnimDriver::Initialize(UAnimInstance* InCharacterAnim, UAnimInstance* InWeaponAnim)
{
    if ( !InCharacterAnim || !InWeaponAnim ) return;

    CharacterAnim = InCharacterAnim;
    WeaponAnim = InWeaponAnim;

    //PX_LOG(Log, TEXT("Weapon Anim Driver Init Complete."));
}

void UPX_WeaponAnimDriver::PlayCommittedAction(const FPXWeaponActionAnimContext& AnimContext, bool bIsLocal)
{
    //PX_LOG(Log, TEXT(""));
    if ( CharacterAnim && AnimContext.MontageToPlay.CharacterMontage )
    {
        const float Rate = (AnimContext.MontageToPlay.PlayRate > 0.f) ? AnimContext.MontageToPlay.PlayRate : 1.f;
        //PX_LOG(Log, TEXT("Character Montage : %s"), *AnimContext.MontageToPlay.CharacterMontage.GetName());
        CharacterAnim->Montage_Play(AnimContext.MontageToPlay.CharacterMontage, Rate);
        if ( AnimContext.MontageToPlay.CharacterSection != NAME_None )
        {
            CharacterAnim->Montage_JumpToSection(AnimContext.MontageToPlay.CharacterSection, AnimContext.MontageToPlay.CharacterMontage);
        }
    }

    if ( WeaponAnim && AnimContext.MontageToPlay.WeaponMontage )
    {
        const float Rate = (AnimContext.MontageToPlay.PlayRate > 0.f) ? AnimContext.MontageToPlay.PlayRate : 1.f;
        //PX_LOG(Log, TEXT("Play Weapon Montage : %s"), *AnimContext.MontageToPlay.WeaponMontage.GetName());
        WeaponAnim->Montage_Play(AnimContext.MontageToPlay.WeaponMontage, Rate);
        if ( AnimContext.MontageToPlay.WeaponSection != NAME_None )
        {
            WeaponAnim->Montage_JumpToSection(AnimContext.MontageToPlay.WeaponSection, AnimContext.MontageToPlay.WeaponMontage);
        }
    }
}

