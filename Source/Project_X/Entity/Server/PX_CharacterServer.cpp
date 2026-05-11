// Fill out your copyright notice in the Description page of Project Settings.

#include "Entity/PX_Character.h"
#include "GameFramework/PawnMovementComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Entity/Client/PX_CharacterAnimInstance.h"

// Called every frame
void APX_Character::Server_Tick(float DeltaTime)
{
	if ( !HasAuthority() ) return;

	if ( bIsAiming && bHasMoveInput )
	{
		const float CurrentYaw = GetActorRotation().Yaw;
		const float TargetYaw = GetAimRotation().Yaw;

		// 부드럽게 따라가게 (속도 조절 가능)
		const float TurnSpeedDegPerSec = 720.f; // 빠르게 붙게 하고 싶으면 720~1080
		const float NewYaw = FMath::FixedTurn(CurrentYaw, TargetYaw, TurnSpeedDegPerSec * DeltaTime);

		SetActorRotation(FRotator(0.f, NewYaw, 0.f));
	}
}

void APX_Character::ServerPlayTurnInPlace_Implementation(bool bTurn180, bool bTurnRight)
{
	if ( !HasAuthority() ) return;
	if ( !CachedAnimInstance ) return;
	if ( CachedAnimInstance->Montage_IsPlaying(Turn90Montage) || CachedAnimInstance->Montage_IsPlaying(Turn180Montage) )	return;

	//UE_LOG(LogTemp, Log, TEXT("ServerPlayTurnInPlace"));

	MulticastPlayTurnInPlace(bTurn180, bTurnRight);
}

void APX_Character::ServerTurnEndSnap_Implementation()
{
	const float TargetYaw = GetAimRotation().Yaw; // (서버에서 유효한 값이어야 함!)
	SetActorRotation(FRotator(0.f, TargetYaw, 0.f));
}

void APX_Character::ServerBeginMove_Implementation(const bool bMoved, const float Inspeed, const EMoveDirection InMoveDirection)
{
	if ( !HasAuthority() ) return;

	bHasMoveInput = bMoved;
	//UE_LOG(LogTemp, Log, TEXT("bShouldUseControllerRotationYaw : %s"), bIsAiming ? TEXT("true") : TEXT("false"));
	LastMoveSpeed = Inspeed;
	LastMoveDirection = InMoveDirection;
}

void APX_Character::ServerEndMove_Implementation(const bool bMoved)
{
	if ( !HasAuthority() ) return;

	bHasMoveInput = bMoved;
	if ( !bMoved )
	{
		LastMoveSpeed = 0.0f;
		LastMoveDirection = EMoveDirection::None;
	}
}

void APX_Character::SetDemoMoveInputState(bool bMoved, float InSpeed, EMoveDirection InMoveDirection)
{
	if ( !HasAuthority() ) return;

	bHasMoveInput = bMoved;
	LastMoveSpeed = bMoved ? InSpeed : 0.0f;
	LastMoveDirection = bMoved ? InMoveDirection : EMoveDirection::None;
}

void APX_Character::SetDemoAimYaw(float InYaw)
{
	if ( !HasAuthority() ) return;

	RemoteViewYaw = FRotator::CompressAxisToByte(InYaw);
	bForceDemoAimOffset = false;
	ForcedDemoAimYaw = 0.0f;
	ForcedDemoAimPitch = 0.0f;
}

void APX_Character::SetDemoAimRotation(const FRotator& InAimRotation)
{
	if ( !HasAuthority() ) return;

	RemoteViewYaw = FRotator::CompressAxisToByte(InAimRotation.Yaw);
	RemoteViewPitch = FRotator::CompressAxisToByte(InAimRotation.Pitch);

	const FRotator Delta = (InAimRotation - GetActorRotation()).GetNormalized();
	bForceDemoAimOffset = true;
	ForcedDemoAimYaw = FMath::Clamp(Delta.Yaw + 6.5f, -180.0f, 180.0f);
	ForcedDemoAimPitch = FMath::Clamp(Delta.Pitch, -90.0f, 90.0f);
}

void APX_Character::SetForceDemoAimOffset(bool bNewForce)
{
	if ( !HasAuthority() ) return;

	bForceDemoAimOffset = bNewForce;
	if ( !bForceDemoAimOffset )
	{
		ForcedDemoAimYaw = 0.0f;
		ForcedDemoAimPitch = 0.0f;
	}
}

void APX_Character::ServerClearGameplayInputStateForUI_Implementation()
{
	if ( !HasAuthority() ) return;

	bHasMoveInput = false;
	LastMoveSpeed = 0.0f;
	LastMoveDirection = EMoveDirection::None;
	ConsumeMovementInputVector();

	bUseControllerRotationYaw = false;
	if ( UCharacterMovementComponent* MovementComp = GetCharacterMovement() )
	{
		MovementComp->bOrientRotationToMovement = true;
		if ( !MovementComp->IsFalling() )
		{
			MovementComp->StopMovementImmediately();
		}
	}

	SetLocomotionCrouching(false);
	SetIsAiming(false);
	ApplyAimCameraMode(false, false);
	ApplyLocomotionSpeedMode();
}

void APX_Character::ServerBeginJump_Implementation()
{
	/*
	// Deprecated: locomotion is now handled by GAS abilities.
	if ( !HasAuthority() ) return;
	if ( bIsJumping || GetMovementComponent()->IsFalling() )	return;

	//UE_LOG(LogTemp, Log, TEXT("Server Begin Jump"));

	bIsJumping = true;
	*/
}

void APX_Character::ServerEndJump_Implementation()
{
	/*
	// Deprecated: locomotion is now handled by GAS abilities.
	if ( !HasAuthority() ) return;
	if ( !bIsJumping )	return;

	bIsJumping = false;
	*/
}

void APX_Character::ServerBeginWalk_Implementation()
{
	/*
	// Deprecated: locomotion is now handled by GAS abilities.
	if ( !HasAuthority() ) return;

	//UE_LOG(LogTemp, Log, TEXT("Server Begin Walk"));

	GetCharacterMovement()->MaxWalkSpeed = 300.0f;
	*/
}

void APX_Character::ServerEndWalk_Implementation()
{
	/*
	// Deprecated: locomotion is now handled by GAS abilities.
	if ( !HasAuthority() ) return;

	//UE_LOG(LogTemp, Log, TEXT("Server End Walk"));

	GetCharacterMovement()->MaxWalkSpeed = 600.0f;
	*/
}

void APX_Character::ServerBeginSprint_Implementation()
{
	/*
	// Deprecated: locomotion is now handled by GAS abilities.
	if ( !HasAuthority() ) return;

	//UE_LOG(LogTemp, Log, TEXT("Server Begin Sprint"));

	GetCharacterMovement()->MaxWalkSpeed = 1200.0f;
	*/
}

void APX_Character::ServerEndSprint_Implementation()
{
	/*
	// Deprecated: locomotion is now handled by GAS abilities.
	if ( !HasAuthority() ) return;

	//UE_LOG(LogTemp, Log, TEXT("Server End Sprint"));

	GetCharacterMovement()->MaxWalkSpeed = 600.0f;
	*/
}

void APX_Character::ServerBeginCrouch_Implementation()
{
	/*
	// Deprecated: locomotion is now handled by GAS abilities.
	if ( !HasAuthority() ) return;
	if ( bIsCrouching )	return;

	//UE_LOG(LogTemp, Log, TEXT("Server Begin Crouch"));

	bIsCrouching = true;
	*/
}

void APX_Character::ServerEndCrouch_Implementation()
{
	/*
	// Deprecated: locomotion is now handled by GAS abilities.
	if ( !HasAuthority() ) return;
	if ( !bIsCrouching )	return;
	//UE_LOG(LogTemp, Log, TEXT("Server End Crouch"));
	bIsCrouching = false;
	*/
}

bool APX_Character::ServerBeginAim_Validate(const bool bPressed)
{
	return true;
}

void APX_Character::ServerBeginAim_Implementation(const bool bPressed)
{
	if ( !HasAuthority() ) return;
	if ( !bPressed )	return;			// Invlaid Function call
	if ( bIsAiming ) return;			// Duplicated Function call

	//UE_LOG(LogTemp, Log, TEXT("Server Begin Aim.. Value : %s"), bPressed ? TEXT("true") : TEXT("false"));
	bIsAiming = bPressed;

	bUseControllerRotationYaw = true;
	GetCharacterMovement()->bOrientRotationToMovement = false;
}

bool APX_Character::ServerEndAim_Validate(const bool bPressed)
{
	return true;
}

void APX_Character::ServerEndAim_Implementation(const bool bPressed)
{
	if ( !HasAuthority() ) return;
	if ( bPressed )	return;			// Invlaid Function call
	if ( !bIsAiming ) return;			// Duplicated Function call

	//UE_LOG(LogTemp, Log, TEXT("Server End Aim.. Value : %s"), bPressed ? TEXT("true") : TEXT("false"));
	bIsAiming = bPressed;

	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;

	if ( bIsDrawing )
	{
		bIsDrawing = bPressed;
		DrawProgress = 0.f;
	}
}

bool APX_Character::ServerBeginDraw_Validate(const bool bPressed)
{
    return true;
}

void APX_Character::ServerBeginDraw_Implementation(const bool bPressed)
{
	if (!HasAuthority()) return;
	if (!bPressed)	return;			// Invlaid Function call
	if (bIsDrawing ) return;			// Duplicated Function call

	//UE_LOG(LogTemp, Log, TEXT("Server Begin Draw.. Value : %s"), bPressed ? TEXT("true") : TEXT("false"));
	bIsDrawing = bPressed;
}

bool APX_Character::ServerEndDraw_Validate(const bool bPressed)
{
	return true;
}

void APX_Character::ServerEndDraw_Implementation(const bool bPressed)
{
	if (!HasAuthority()) return;
	if (bPressed)	return;			// Invlaid Function call
	if (!bIsDrawing ) return;			// Duplicated Function call

	//UE_LOG(LogTemp, Log, TEXT("Server End Draw.. Value : %s"), bPressed ? TEXT("true") : TEXT("false"));
	bIsDrawing = bPressed;
	DrawProgress = 0.f;
}

bool APX_Character::ServerUpdateDrawProgress_Validate(float Progress)
{
	return true;
}
void APX_Character::ServerUpdateDrawProgress_Implementation(float Progress)
{
	if (!HasAuthority()) return;
	if (!bIsDrawing) return;

	DrawProgress = FMath::Clamp(Progress, 0.f, 1.f);
}

void APX_Character::ServerAddControllerYawInput_Implementation(uint8 CompressedYaw)
{
	if (!HasAuthority()) return;

	RemoteViewYaw = CompressedYaw;
}

float APX_Character::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (!HasAuthority()) return 0.f;

	const float Damage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	UE_LOG(LogTemp, Log, TEXT("%s takes %f damage from %s"), *this->GetName(), Damage, *DamageCauser->GetName());

	return Damage;
}
