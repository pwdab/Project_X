// Fill out your copyright notice in the Description page of Project Settings.

#include "Entity/Client/PX_CharacterAnimInstance.h"
#include "Entity/Client/PX_CharacterLayerAnimInstance.h"
#include "GameFramework/PawnMovementComponent.h"
//#include "Components/CapsuleComponent.h"    // Debug Draw
#include "Entity/PX_Character.h"

void UPX_CharacterAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    if (APawn* Pawn = TryGetPawnOwner())
    {

        Velocity = Pawn->GetVelocity();

        if ( ACharacter* Character = Cast<ACharacter>(Pawn) )
        {
            bIsFalling = Character->GetMovementComponent()->IsFalling();

            if ( APX_Character* PX_Character = Cast<APX_Character>(TryGetPawnOwner()) )
            {
                bHasInput = PX_Character->HasMoveInput();

                if ( bHasInput )
                {
                    // Get Character's Last Movement Speed and Direction
                    const float LastSpeed = PX_Character->GetLastMoveSpeed();
                    const FVector2D LastDirection = MoveDirectionToVector2D(PX_Character->GetLastMoveDirection());

                    LastForwardVelocity = LastDirection.X * LastSpeed;
                    LastRightVelocity = LastDirection.Y * LastSpeed;
                }
                
                if ( PX_Character->GetLayerAnimInstance() )
                {
                    PX_Character->GetLayerAnimInstance()->SetHasInput(bHasInput);
                    PX_Character->GetLayerAnimInstance()->SetVelocity(Velocity, LastForwardVelocity, LastRightVelocity);
                    PX_Character->GetLayerAnimInstance()->SetIsFalling(bIsFalling);
                }
            }
        }
        

        // Get Aim Rotation & Actor Rotation
        const FRotator AimRot = Pawn->IsLocallyControlled() ? Pawn->GetControlRotation() : Cast<APX_Character>(Pawn)->GetAimRotation();
        //const FRotator AimRot = PX_Character ? PX_Character->GetAimRotation() : Pawn->GetControlRotation();
        const FRotator RefRot = Pawn->GetActorRotation();

        // Calculate Delta of Aim Rotation & Actor Rotation
        FRotator Delta = (AimRot - RefRot).GetNormalized();

        // Set Aim Yaw & Pitch
        const float AimOffset = 6.5f;
        AimYaw = FMath::Clamp(Delta.Yaw + AimOffset, -180.f, 180.f);
        AimPitch = FMath::Clamp(Delta.Pitch, -90.f, 90.f);

        if ( APX_Character* PX_Character = Cast<APX_Character>(TryGetPawnOwner()) )
        {
            if ( PX_Character->GetLayerAnimInstance() )
            {
                PX_Character->GetLayerAnimInstance()->SetRotation(AimYaw, AimPitch);
            }
        }

        // Turn-in-place
        const float DeltaYaw = Delta.Yaw;
        const bool bCanTurnInPlace = bIsAiming && !bIsFalling && !bHasInput;
        if ( bCanTurnInPlace )
        {
            if ( APX_Character* PX_Character = Cast<APX_Character>(TryGetPawnOwner()) )
            {
                PX_Character->RequestTurnInPlace(DeltaYaw);
            }
        }
    }
}

void UPX_CharacterAnimInstance::SetIsJumping(bool Value)
{
    bIsJumping = Value;
    if ( APX_Character* PX_Character = Cast<APX_Character>(TryGetPawnOwner()) )
    {
        PX_Character->GetLayerAnimInstance()->SetIsJumping(bIsJumping);
    }
}

void UPX_CharacterAnimInstance::SetIsCrouching(bool Value)
{
    bIsCrouching = Value;
    if ( APX_Character* PX_Character = Cast<APX_Character>(TryGetPawnOwner()) )
    {
        PX_Character->GetLayerAnimInstance()->SetIsCrouching(bIsCrouching);
    }
}

void UPX_CharacterAnimInstance::SetIsAiming(bool Value)
{
    
    //UE_LOG(LogTemp, Log, TEXT("PX_CharacterAnimInstance::SetIsAiming %s -> %s"), bIsAiming ? TEXT("true") : TEXT("false"), Value ? TEXT("true") : TEXT("false"));
    bIsAiming = Value;
    if ( APX_Character* PX_Character = Cast<APX_Character>(TryGetPawnOwner()) )
    {
        PX_Character->GetLayerAnimInstance()->SetIsAiming(bIsAiming);
    }
}

void UPX_CharacterAnimInstance::SetAimProgress(float Value)
{
    AimProgress = FMath::Clamp(Value, 0.f, 1.f);
    if ( APX_Character* PX_Character = Cast<APX_Character>(TryGetPawnOwner()) )
    {
        PX_Character->GetLayerAnimInstance()->SetAimProgress(AimProgress);
    }
}

void UPX_CharacterAnimInstance::SetIsDrawing(bool Value)
{
    bIsDrawing = Value;
    if ( APX_Character* PX_Character = Cast<APX_Character>(TryGetPawnOwner()) )
    {
        PX_Character->GetLayerAnimInstance()->SetIsDrawing(bIsDrawing);
    }
}

void UPX_CharacterAnimInstance::SetDrawProgress(float Value)
{
    DrawProgress = FMath::Clamp(Value, 0.f, 1.f);
    if ( APX_Character* PX_Character = Cast<APX_Character>(TryGetPawnOwner()) )
    {
        PX_Character->GetLayerAnimInstance()->SetDrawProgress(DrawProgress);
    }
}

void UPX_CharacterAnimInstance::SetIsReloading(bool Value)
{
    bIsReloading = Value;
    if ( APX_Character* PX_Character = Cast<APX_Character>(TryGetPawnOwner()) )
    {
        PX_Character->GetLayerAnimInstance()->SetIsReloading(bIsReloading);
    }
}

void UPX_CharacterAnimInstance::AnimNotify_ReloadStart()
{
    UE_LOG(LogTemp, Log, TEXT("AnimNotify_ReloadStart"));
    bIsReloading = true;
    if ( APX_Character* PX_Character = Cast<APX_Character>(TryGetPawnOwner()) )
    {
        PX_Character->GetLayerAnimInstance()->SetIsReloading(bIsReloading);
    }
}

void UPX_CharacterAnimInstance::AnimNotify_ReloadEnd()
{
    UE_LOG(LogTemp, Log, TEXT("AnimNotify_ReloadEnd"));
    bIsReloading = false;
    if ( APX_Character* PX_Character = Cast<APX_Character>(TryGetPawnOwner()) )
    {
        PX_Character->HandleEndAim();
        PX_Character->GetLayerAnimInstance()->SetIsReloading(bIsReloading);
    }
}

void UPX_CharacterAnimInstance::AnimNotify_TurnEnd()
{
    if ( APX_Character* PX_Character = Cast<APX_Character>(TryGetPawnOwner()) )
    {
        PX_Character->RequestTurnEndSnap();
    }
}

FVector2D UPX_CharacterAnimInstance::MoveDirectionToVector2D(EMoveDirection MoveDirection)
{
    float foward = 0.f;
    float right = 0.f;

    const bool bFront = EnumHasAnyFlags(MoveDirection, EMoveDirection::Front);
    const bool bBack = EnumHasAnyFlags(MoveDirection, EMoveDirection::Back);
    const bool bRight = EnumHasAnyFlags(MoveDirection, EMoveDirection::Right);
    const bool bLeft = EnumHasAnyFlags(MoveDirection, EMoveDirection::Left);
    const bool bDiagonal = ((bFront || bBack) && (bLeft || bRight));

    if ( bFront ) foward += 1.f;
    if ( bBack )  foward -= 1.f;
    if ( bRight ) right += 1.f;
    if ( bLeft )  right -= 1.f;

    if ( bDiagonal )
    {
        foward *= 0.5f;
        right *= 0.5f;
    }

    return FVector2D(foward, right);
}