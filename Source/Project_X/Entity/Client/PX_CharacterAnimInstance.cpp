// Fill out your copyright notice in the Description page of Project Settings.


#include "Entity/Client/PX_CharacterAnimInstance.h"

void UPX_CharacterAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    if (APawn* Pawn = TryGetPawnOwner())
    {

        Velocity = Pawn->GetVelocity();

        if (Pawn->IsLocallyControlled())
        {
            //UE_LOG(LogTemp, Log, TEXT("Locally Controlled Pawn: %s, Velocity: (%f, %f, %f)"), *Pawn->GetName(), Velocity.X, Velocity.Y, Velocity.Z);
        }
    }
}


