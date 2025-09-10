// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Project_X.h"
#include "Animation/AnimInstance.h"
#include "PX_BowAnimInstance.generated.h"

class APX_Character;

/**
 * 
 */
UCLASS()
class PROJECT_X_API UPX_BowAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

protected:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bow")
    bool bIsAiming = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bow")
    bool bIsReloading = false;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bow")
    FVector Target = FVector::ZeroVector;
	
private:
	// Mesh of Bow
	TWeakObjectPtr<USkeletalMeshComponent> Mesh;

	// Owner of Bow
	TWeakObjectPtr<APX_Character> Owner;
};
