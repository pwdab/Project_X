// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PX_WeaponAnimDriver.h"
#include "PX_BowAnimDriver.generated.h"

/**
 * Bow driver: computes bow aim direction/target point from weapon sockets and camera.
 * This moves the old UPX_WeaponComponent::TickComponent bow-aim logic into an extensible driver.
 */
UCLASS(Blueprintable, EditInlineNew, DefaultToInstanced)
class PROJECT_X_API UPX_BowAnimDriver : public UPX_WeaponAnimDriver
{
    GENERATED_BODY()

public:
    /** Weapon socket names used by the legacy bow asset. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Bow")
    FName BowStringSocket = TEXT("bow_string");

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Bow")
    FName BowMuzzleSocket = TEXT("bow_string_origin");

    /** Trace range for aim calculations. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Bow")
    float TraceRange = 15000.0f;

    /** Last computed target point (world). Useful for debug / AnimBP variables. */
    UPROPERTY(BlueprintReadOnly, Category="Bow")
    FVector LastTargetPoint = FVector::ZeroVector;

    /** Last computed aim rotation from bow string towards target. */
    UPROPERTY(BlueprintReadOnly, Category="Bow")
    FRotator LastAimRotation = FRotator::ZeroRotator;

    /** If true, draws debug lines/spheres. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Bow")
    bool bDrawDebug = false;

    //virtual void OnTick_Implementation(float DeltaSeconds, const FPXWeaponDriverContext& Context) override;
};
