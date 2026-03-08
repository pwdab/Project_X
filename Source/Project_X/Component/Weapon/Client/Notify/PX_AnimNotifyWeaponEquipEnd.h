// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Project_X.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "PX_AnimNotifyWeaponEquipEnd.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(PX_AnimNotifyWeaponEquipEnd, Log, All);

/**
 * 
 */
UCLASS()
class PROJECT_X_API UPX_AnimNotifyWeaponEquipEnd : public UAnimNotify
{
	GENERATED_BODY()
	
public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;
	
	
};
