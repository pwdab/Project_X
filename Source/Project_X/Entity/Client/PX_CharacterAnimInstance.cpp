// Fill out your copyright notice in the Description page of Project Settings.


#include "Entity/Client/PX_CharacterAnimInstance.h"

void UPX_CharacterAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    if (APawn* Pawn = TryGetPawnOwner())
    {

        Velocity = Pawn->GetVelocity();

        // Get Aim Rotation & Actor Rotation
        const FRotator AimRot = Pawn->GetBaseAimRotation();
        const FRotator RefRot = Pawn->GetActorRotation();

        // Calculate Delta of Aim Rotation & Actor Rotation
        FRotator Delta = FRotator::ZeroRotator;
        Delta.Pitch = FMath::FindDeltaAngleDegrees(RefRot.Pitch, AimRot.Pitch);
        Delta.Yaw = FMath::FindDeltaAngleDegrees(RefRot.Yaw, AimRot.Yaw);
        Delta.Roll = FMath::FindDeltaAngleDegrees(RefRot.Roll, AimRot.Roll);

        // Set Aim Yaw & Pitch
        AimYaw = FMath::Clamp(Delta.Yaw, -180.f, 180.f);
        AimPitch = FMath::Clamp(Delta.Pitch, -90.f, 90.f);

        if (Pawn->IsLocallyControlled())
        {
            //UE_LOG(LogTemp, Log, TEXT("Locally Controlled Pawn: %s, Velocity: (%f, %f, %f)"), *Pawn->GetName(), Velocity.X, Velocity.Y, Velocity.Z);
        }
    }
}

void UPX_CharacterAnimInstance::SetIsAiming(bool Value)
{
    bIsAiming = Value;
}

void UPX_CharacterAnimInstance::SetAimProgress(float Value)
{
    AimProgress = FMath::Clamp(Value, 0.f, 1.f);
}

void UPX_CharacterAnimInstance::SetIsDrawing(bool Value)
{
    bIsDrawing = Value;
}

void UPX_CharacterAnimInstance::SetDrawProgress(float Value)
{
    DrawProgress = FMath::Clamp(Value, 0.f, 1.f);
}

void UPX_CharacterAnimInstance::AnimNotify_ReloadStart()
{
    //UE_LOG(LogTemp, Log, TEXT("AnimNotify_ReloadStart"));
    bIsReloading = true;
}

void UPX_CharacterAnimInstance::AnimNotify_ReloadEnd()
{
    //UE_LOG(LogTemp, Log, TEXT("AnimNotify_ReloadEnd"));
    bIsReloading = false;
}
