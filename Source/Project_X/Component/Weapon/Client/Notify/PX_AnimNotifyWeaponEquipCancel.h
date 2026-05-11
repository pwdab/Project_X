// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Project_X.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "PX_AnimNotifyWeaponEquipCancel.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(PX_AnimNotifyWeaponEquipCancel, Log, All);

/**
 * 
 */
UCLASS()
class PROJECT_X_API UPX_AnimNotifyWeaponEquipCancel : public UAnimNotify
{
	GENERATED_BODY()
	
public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;
	
	
};
