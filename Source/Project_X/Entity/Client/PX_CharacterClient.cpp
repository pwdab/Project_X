// Fill out your copyright notice in the Description page of Project Settings.

#include "Entity/PX_Character.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/Attributes/PX_MovementAttributeSet.h"
#include "AbilitySystem/Tags/PX_GameplayTags.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Framework/PlayerState/PX_PlayerState.h"

#if !UE_SERVER
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Input/Config/PX_InputConfigDataAsset.h"
#include "Input/Component/PX_EnhancedInputComponent.h"
#include "AbilitySystem/Tags/PX_GameplayTags.h"
#include "AbilitySystem/Attributes/PX_MovementAttributeSet.h"
#include "AbilitySystem/Component/PX_AbilitySystemComponent.h"

#include "PX_CharacterAnimInstance.h"
#include "PX_CharacterLayerAnimInstance.h"
#include "Components/TimelineComponent.h"
#include "Entity/PX_Weapon.h"
//#include "Component/PX_WeaponComponent.h"
#include "Component/Weapon/PX_WeaponSystemComponent.h"
#include "Component/Inventory/PX_InventoryComponent.h"
#include "Component/Inventory/PX_WeaponItemInstance.h"
#include "Component//Weapon/PX_WeaponDataAsset.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/PawnMovementComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"

#include "Framework/Controller/PX_PlayerController.h"
#include "AbilitySystemBlueprintLibrary.h"
#endif

// Called when the game starts or when spawned
void APX_Character::BeginPlay()
{
	Super::BeginPlay();

#if !UE_SERVER
	// Cache PX_CharacterAnimInstance
	CachedAnimInstance = Cast<UPX_CharacterAnimInstance>(GetMesh()->GetAnimInstance());

	// Link Layered BP
	if ( CachedAnimInstance && AnimInstanceLayerClass )
	{
		CachedAnimInstance->LinkAnimClassLayers(AnimInstanceLayerClass);
		CachedLayerAnimInstance = Cast<UPX_CharacterLayerAnimInstance>(CachedAnimInstance->GetLinkedAnimLayerInstanceByClass(AnimInstanceLayerClass));

		/*
		// Copy PX_CharacterAnimInstance to PX_CharacterLayerAnimInstance
		if ( CachedAnimInstance && CachedLayerAnimInstance )
		{
			CachedLayerAnimInstance->ApplyAim(CachedAnimInstance->GetIsAiming(), CachedAnimInstance->GetAimProgress());
			CachedLayerAnimInstance->ApplyDraw(CachedAnimInstance->GetIsDrawing(), CachedAnimInstance->GetDrawProgress());
			CachedLayerAnimInstance->SetReloading(CachedAnimInstance->GetIsReloading());
		}
		*/
	}

	if (AimProgressCurve && AimProgressTimeline)
	{
		FOnTimelineFloat AimProgressUpdate;
		AimProgressUpdate.BindUFunction(this, FName("AimProgressUpdate"));
		AimProgressTimeline->AddInterpFloat(AimProgressCurve, AimProgressUpdate);
		AimProgressTimeline->SetLooping(false);
	}
	/*
	// Create User Widget
	if (IsLocallyControlled() && HudRootWidgetClass)
	{
		if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
		{
			HudRootWidget = CreateWidget<UUserWidget>(PlayerController, HudRootWidgetClass);
			if ( HudRootWidget )
			{
				HudRootWidget->AddToViewport();
			}
		}
	}
	*/
#endif
}

void APX_Character::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	APX_PlayerState* PX_PlayerState = GetPlayerState<APX_PlayerState>();
	if ( !PX_PlayerState ) return;

	UAbilitySystemComponent* ASC = PX_PlayerState->GetAbilitySystemComponent();
	if ( !ASC ) return;

	ASC->InitAbilityActorInfo(PX_PlayerState, this);

	PX_LOG(Log, TEXT("Init AbilitySystemComponent's AbilityActorInfo"));
}



// Called every frame
void APX_Character::Client_Tick(float DeltaTime)
{
	/*
	if ( HasAuthority() || !IsLocallyControlled() ) return;

	// ===== Draw Held (Pressed) =====
	if ( bWantsDraw )
	{
		// Aim이 아니면 Aim부터 진입 (원래 BeginDraw에서 하던 것)
		if ( !bIsAiming )
		{
			BeginAim(true);
			AimState.AddState(EAimState::Draw);
		}

		// Draw 시작(원래 BeginDraw에서 bIsDrawing 처음 true 될 때 하던 것)
		if ( !bIsDrawing )
		{
			// 로컬 예측 세팅 (서버 replication 오기 전까지도 상태가 유지되도록)
			bIsDrawing = true;
			DrawProgress = 0.f;

			if ( CachedAnimInstance )
			{
				CachedAnimInstance->SetIsDrawing(true);
				CachedAnimInstance->SetDrawProgress(0.f);
			}

			ServerBeginDraw(true);
		}

		// 드로우 진행도 (기존: Instance.GetElapsedTime() 사용)
		const float Now = GetWorld()->GetTimeSeconds();
		const float Elapsed = FMath::Clamp(Now - DrawInputStartTime, 0.f, 1.f);

		DrawProgress = Elapsed;

		if ( CachedAnimInstance )
		{
			CachedAnimInstance->SetDrawProgress(Elapsed);
		}

		ServerUpdateDrawProgress(Elapsed);
	}
	// ===== Draw Released (Not Pressed) =====
	else
	{
		// 드로우 상태였는데 지금은 뗐다 → EndDraw 처리(원래 EndDraw에서 하던 것)
		if ( bIsDrawing )
		{
			AimState.RemoveState(EAimState::Draw);

			bIsDrawing = false;   // 로컬 예측 세팅
			DrawProgress = 0.f;

			if ( CachedAnimInstance )
			{
				CachedAnimInstance->SetIsDrawing(false);
				CachedAnimInstance->SetDrawProgress(0.f);
			}

			ServerEndDraw(false);

			// 기존 EndDraw의 Attack 호출을 Tick로 이동
			if ( bFireOnDrawRelease && WeaponComponent )
			{
				WeaponComponent->Attack();
			}
		}

		bFireOnDrawRelease = false;
	}
	*/

}

void APX_Character::SetLayerAnimInstanceByClass(TSubclassOf<UAnimInstance> InAnimInstanceClass)
{
	if ( InAnimInstanceClass && CachedAnimInstance )
	{
		//UE_LOG(LogTemp, Log, TEXT("PX_Character::Set Layer Anim Instace by Class"));
		AnimInstanceLayerClass = InAnimInstanceClass;
		CachedAnimInstance->LinkAnimClassLayers(AnimInstanceLayerClass);
		CachedLayerAnimInstance = Cast<UPX_CharacterLayerAnimInstance>(CachedAnimInstance->GetLinkedAnimLayerInstanceByClass(AnimInstanceLayerClass));
		if ( CachedLayerAnimInstance )
		{
			CachedLayerAnimInstance->SetHasEquippedWeapon(CachedAnimInstance->GetHasEquippedWeapon());
		}
	}
}

#if !UE_SERVER
// Called to bind functionality to input
void APX_Character::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(GetDefaultMappingContext(), 0);
		}
	}

	// Set up action bindings
	/*
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Moving (Jog)
		EnhancedInputComponent->BindAction(GetMoveAction(), ETriggerEvent::Triggered, this, &APX_Character::BeginMove);
		EnhancedInputComponent->BindAction(GetMoveAction(), ETriggerEvent::Completed, this, &APX_Character::EndMove);
		// Looking
		EnhancedInputComponent->BindAction(GetLookAction(), ETriggerEvent::Triggered, this, &APX_Character::Look);
		// Jump
		EnhancedInputComponent->BindAction(GetJumpAction(), ETriggerEvent::Started, this, &APX_Character::BeginJump);
		EnhancedInputComponent->BindAction(GetJumpAction(), ETriggerEvent::Completed, this, &APX_Character::EndJump);
		// Walk
		EnhancedInputComponent->BindAction(GetWalkAction(), ETriggerEvent::Started, this, &APX_Character::BeginWalk);
		EnhancedInputComponent->BindAction(GetWalkAction(), ETriggerEvent::Completed, this, &APX_Character::EndWalk);
		// Sprint
		EnhancedInputComponent->BindAction(GetSprintAction(), ETriggerEvent::Started, this, &APX_Character::BeginSprint);
		EnhancedInputComponent->BindAction(GetSprintAction(), ETriggerEvent::Completed, this, &APX_Character::EndSprint);
		// Crouch
		EnhancedInputComponent->BindAction(GetCrouchAction(), ETriggerEvent::Started, this, &APX_Character::BeginCrouch);
		EnhancedInputComponent->BindAction(GetCrouchAction(), ETriggerEvent::Completed, this, &APX_Character::EndCrouch);
		// Interact
		EnhancedInputComponent->BindAction(GetInteractAction(), ETriggerEvent::Started, this, &APX_Character::Interact);
		// Over the Shoulder Aiming
		EnhancedInputComponent->BindAction(GetOTSAimAction(), ETriggerEvent::Started, this, &APX_Character::BeginOTSAim);
		EnhancedInputComponent->BindAction(GetOTSAimAction(), ETriggerEvent::Completed, this, &APX_Character::EndOTSAim);
		// Aim Down Sights
		EnhancedInputComponent->BindAction(GetADSAction(), ETriggerEvent::Started, this, &APX_Character::BeginADS);
		EnhancedInputComponent->BindAction(GetADSAction(), ETriggerEvent::Completed, this, &APX_Character::EndADS);
		// Drawing
		//EnhancedInputComponent->BindAction(GetDrawAction(), ETriggerEvent::Ongoing, this, &APX_Character::BeginDraw);
		//EnhancedInputComponent->BindAction(GetDrawAction(), ETriggerEvent::Started, this, &APX_Character::BeginDraw);
		//EnhancedInputComponent->BindAction(GetDrawAction(), ETriggerEvent::Canceled, this, &APX_Character::EndDraw);
		//EnhancedInputComponent->BindAction(GetDrawAction(), ETriggerEvent::Completed, this, &APX_Character::EndDraw);
		// Fire
		EnhancedInputComponent->BindAction(GetFireAction(), ETriggerEvent::Started, this, &APX_Character::FirePressed);
		EnhancedInputComponent->BindAction(GetFireAction(), ETriggerEvent::Completed, this, &APX_Character::FireReleased);
		// Reload
		EnhancedInputComponent->BindAction(GetReloadAction(), ETriggerEvent::Started, this, &APX_Character::Reload);
		// EquipSlot
		EnhancedInputComponent->BindAction(GetEquipSlotAction(), ETriggerEvent::Started, this, &APX_Character::EquipSlot);
		// Switch Fire Mode
		EnhancedInputComponent->BindAction(GetSwitchFireModeAction(), ETriggerEvent::Started, this, &APX_Character::SwitchFireMode);
		// Toggle Inventory
		EnhancedInputComponent->BindAction(GetToggleInventoryAction(), ETriggerEvent::Started, this, &APX_Character::ToggleInventory);
	}
	*/
	UPX_EnhancedInputComponent* PX_EnhancedInputComponent = Cast<UPX_EnhancedInputComponent>(PlayerInputComponent);

	if ( !PX_EnhancedInputComponent || !InputConfigDataAsset ) return;

	PX_LOG(Log, TEXT("Setup Native Inputs"));

	// Native Input
	PX_EnhancedInputComponent->BindNativeInputAction(InputConfigDataAsset, PX_GameplayTags::Input_Native_Move, ETriggerEvent::Triggered, this, &APX_Character::BeginMove);
	PX_EnhancedInputComponent->BindNativeInputAction(InputConfigDataAsset, PX_GameplayTags::Input_Native_Move, ETriggerEvent::Completed, this, &APX_Character::EndMove);
	PX_EnhancedInputComponent->BindNativeInputAction(InputConfigDataAsset, PX_GameplayTags::Input_Native_Look, ETriggerEvent::Triggered, this, &APX_Character::Look);
	PX_EnhancedInputComponent->BindNativeInputAction(InputConfigDataAsset, PX_GameplayTags::Input_Native_ToggleInventory, ETriggerEvent::Started, this, &APX_Character::ToggleInventory);

	PX_LOG(Log, TEXT("Setup Ability Inputs"));

	// Ability Input
	PX_EnhancedInputComponent->BindAbilityInputActions(InputConfigDataAsset, this, &APX_Character::AbilityInputPressed, &APX_Character::AbilityInputReleased);

	PX_LOG(Log, TEXT("Setup Ability Special Inputs"));

	// Special Inputs
	PX_EnhancedInputComponent->BindAction(EquipSlotAction, ETriggerEvent::Started, this, &APX_Character::EquipSlot);
}

void APX_Character::AbilityInputPressed(FGameplayTag InputTag)
{
	//PX_LOG(Log, TEXT("Binded Ability Input Pressed Functions. Binded Action : %s"), *InputTag.ToString());
	APX_PlayerState* PX_PlayerState = GetPlayerState<APX_PlayerState>();
	if ( !PX_PlayerState )
	{
		PX_LOG(Warning, TEXT("Invalid PX_PlayerState"));
		return;
	}

	UPX_AbilitySystemComponent* PX_AbilitySystemComponent = Cast<UPX_AbilitySystemComponent>(PX_PlayerState->GetAbilitySystemComponent());
	if ( !PX_AbilitySystemComponent )
	{
		PX_LOG(Warning, TEXT("Invalid PX_AbilitySystemComponent"));
		return;
	}

	// Attack 입력일 때만 현재 무기에 따라 세부 Attack 태그로 변환
	if ( InputTag == PX_GameplayTags::Input_Combat_Attack )
	{
		CachedAttackInputTag = ResolveAttackInputTag();
		InputTag = CachedAttackInputTag;
	}
	else if ( InputTag.MatchesTag(PX_GameplayTags::Input_Combat_Attack) )
	{
		CachedAttackInputTag = InputTag;
	}

	PX_LOG(Log, TEXT("Binded Ability Input Pressed Functions. Binded Action : %s"), *InputTag.ToString());

	PX_AbilitySystemComponent->AbilityInputTagPressed(InputTag);

	// Attack 입력이면 Aim Tag도 함께 전달
	if ( InputTag == CachedAttackInputTag )
	{
		PX_AbilitySystemComponent->AbilityInputTagPressed(PX_GameplayTags::Input_Combat_Aim);
	}

	/*
	// ADS 혹은 OTS 입력이면 Aim Tag도 함께 전달
	if ( InputTag == PX_GameplayTags::Input_Combat_Aim_ADS || InputTag == PX_GameplayTags::Input_Combat_Aim_OTS )
	{
		PX_AbilitySystemComponent->AbilityInputTagPressed(PX_GameplayTags::Input_Combat_Aim);
	}
	*/
}

void APX_Character::AbilityInputReleased(FGameplayTag InputTag)
{
	//PX_LOG(Log, TEXT("Binded Ability Input Pressed Functions. Binded Action : %s"), *InputTag.ToString());
	APX_PlayerState* PXPlayerState = GetPlayerState<APX_PlayerState>();
	if ( !PXPlayerState ) return;

	UPX_AbilitySystemComponent* PX_AbilitySystemComponent = Cast<UPX_AbilitySystemComponent>(PXPlayerState->GetAbilitySystemComponent());
	if ( !PX_AbilitySystemComponent ) return;

	// Attack 입력이면 Cached Attack Input을 사용
	if ( InputTag == PX_GameplayTags::Input_Combat_Attack )
	{
		InputTag = CachedAttackInputTag;
	}

	PX_LOG(Log, TEXT("Binded Ability Input Pressed Functions. Binded Action : %s"), *InputTag.ToString());

	PX_AbilitySystemComponent->AbilityInputTagReleased(InputTag);

	// Attack 입력이면 Aim Tag도 함께 Release
	if ( InputTag == CachedAttackInputTag )
	{
		PX_AbilitySystemComponent->AbilityInputTagReleased(PX_GameplayTags::Input_Combat_Aim);
	}

	/*
	// ADS 혹은 OTS 입력이면 Aim Tag도 함께 Release
	if ( InputTag == PX_GameplayTags::Input_Combat_Aim_ADS || InputTag == PX_GameplayTags::Input_Combat_Aim_OTS )
	{
		PX_AbilitySystemComponent->AbilityInputTagReleased(PX_GameplayTags::Input_Combat_Aim);
	}
	*/
}

void APX_Character::BeginMove(const FInputActionValue& Value)
{
	if ( bGameplayInputBlockedForUI )
	{
		return;
	}

	// input is a Vector2D
	const FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw + CameraOffset, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);

		// Update Last Move Speed and Direction
		bHasMoveInput = true;
		bUseControllerRotationYaw = bIsAiming;
		GetCharacterMovement()->bOrientRotationToMovement = !bIsAiming;

		LastMoveSpeed = GetVelocity().Size2D();
		LastMoveDirection = ResolveMoveDirection(MovementVector);

		ServerBeginMove(bHasMoveInput, LastMoveSpeed, LastMoveDirection);
	}
}

void APX_Character::EndMove()
{
	bHasMoveInput = false;
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;

	ServerEndMove(bHasMoveInput);
}

EMoveDirection APX_Character::ResolveMoveDirection(const FVector2D& MovementVector, float DeadZone) const
{
	EMoveDirection Direction = EMoveDirection::None;

	if ( !bIsAiming )	return EMoveDirection::Front;

	// Front/Back (Y)
	if ( MovementVector.Y > DeadZone )
		Direction |= EMoveDirection::Front;
	else if ( MovementVector.Y < -DeadZone )
		Direction |= EMoveDirection::Back;

	// Right/Left (X)
	if ( MovementVector.X > DeadZone )
		Direction |= EMoveDirection::Right;
	else if ( MovementVector.X < -DeadZone )
		Direction |= EMoveDirection::Left;

	return Direction;
}

void APX_Character::Look(const FInputActionValue& Value)
{
	const FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);

		if (IsLocallyControlled())
		{
			const uint8 CompressedYaw = FRotator::CompressAxisToByte(Controller->GetControlRotation().Yaw);
			ServerAddControllerYawInput(CompressedYaw);
		}
	}
}

void APX_Character::RequestTurnInPlace(float DeltaYaw)
{
	if ( !IsLocallyControlled() ) return;

	const float AbsYaw = FMath::Abs(DeltaYaw);

	bool bTurn180 = false;
	if ( AbsYaw >= 140.0f )      bTurn180 = true;
	else if ( AbsYaw >= 90.0f )  bTurn180 = false;
	else return;

	const bool bTurnRight = (DeltaYaw > 0.f);

	const float PlayRate = FMath::GetMappedRangeValueClamped(FVector2D(60.f, 180.f), FVector2D(1.0f, 3.0f), AbsYaw);

	ServerPlayTurnInPlace(bTurn180, bTurnRight);
}

void APX_Character::RequestTurnEndSnap()
{
	if ( !IsLocallyControlled() ) return;
	ServerTurnEndSnap();
}

void APX_Character::BeginJump()
{
	/*
	// Deprecated: locomotion is now handled by GAS abilities.
	if ( HasAuthority() || !IsLocallyControlled() ) return;

	if ( !bIsJumping && !GetMovementComponent()->IsFalling() )
	{
		bPressedJump = true;
		if ( CachedAnimInstance )
		{
			CachedAnimInstance->SetIsCrouching(true);
		}

		ServerBeginJump();
	}
	*/
}

void APX_Character::EndJump()
{
	/*
	// Deprecated: locomotion is now handled by GAS abilities.
	if ( HasAuthority() || !IsLocallyControlled() ) return;

	if ( bIsJumping || GetMovementComponent()->IsFalling() )
	{
		bPressedJump = false;
		if ( CachedAnimInstance )
		{
			CachedAnimInstance->SetIsCrouching(false);
		}

		ServerEndJump();
	}
	*/
}

void APX_Character::BeginWalk()
{
	/*
	// Deprecated: locomotion is now handled by GAS abilities.
	//UE_LOG(LogTemp, Log, TEXT("Client Begin Walk"));

	if ( HasAuthority() || !IsLocallyControlled() ) return;

	GetCharacterMovement()->MaxWalkSpeed = 300.0f;
	ServerBeginWalk();
	*/
}

void APX_Character::EndWalk()
{
	/*
	// Deprecated: locomotion is now handled by GAS abilities.
	//UE_LOG(LogTemp, Log, TEXT("Client End Walk"));

	if ( HasAuthority() || !IsLocallyControlled() ) return;

	GetCharacterMovement()->MaxWalkSpeed = 600.0f;
	ServerEndWalk();
	*/
}

void APX_Character::BeginSprint()
{
	/*
	// Deprecated: locomotion is now handled by GAS abilities.
	//UE_LOG(LogTemp, Log, TEXT("Client Begin Sprint"));

	if ( HasAuthority() || !IsLocallyControlled() ) return;

	GetCharacterMovement()->MaxWalkSpeed = 900.0f;
	ServerBeginSprint();
	*/
}

void APX_Character::EndSprint()
{
	/*
	// Deprecated: locomotion is now handled by GAS abilities.
	//UE_LOG(LogTemp, Log, TEXT("Client End Sprint"));

	if ( HasAuthority() || !IsLocallyControlled() ) return;

	GetCharacterMovement()->MaxWalkSpeed = 600.0f;
	ServerEndSprint();
	*/
}

void APX_Character::BeginCrouch()
{
	/*
	// Deprecated: locomotion is now handled by GAS abilities.
	if ( HasAuthority() || !IsLocallyControlled() ) return;

	//UE_LOG(LogTemp, Log, TEXT("Client Begin Crouch: bIsCrouching is %s"), bIsCrouching ? TEXT("true") : TEXT("false"));

	if ( !bIsCrouching )
	{
		if ( CachedAnimInstance )
		{
			CachedAnimInstance->SetIsCrouching(true);
		}

		GetCharacterMovement()->MaxWalkSpeed = 300.0f;
		ServerBeginCrouch();
	}
	*/
}

void APX_Character::EndCrouch()
{
	/*
	// Deprecated: locomotion is now handled by GAS abilities.
	if ( HasAuthority() || !IsLocallyControlled() ) return;

	//UE_LOG(LogTemp, Log, TEXT("Client End Crouch: bIsCrouching is %s"), bIsCrouching ? TEXT("true") : TEXT("false"));

	if ( bIsCrouching )
	{
		if ( CachedAnimInstance )
		{
			CachedAnimInstance->SetIsCrouching(false);
		}

		GetCharacterMovement()->MaxWalkSpeed = 600.0f;
		ServerEndCrouch();
	}
	*/
}

void APX_Character::Interact()
{
	UE_LOG(LogTemp, Log, TEXT("Client Interact"));
}

/*
void APX_Character::BeginAim(const EAimState NewAimState)
{
	//UE_LOG(LogTemp, Log, TEXT("Client Begin Aim"));

	if ( HasAuthority() || !IsLocallyControlled() ) return;

	//
	//if ( !bIsAiming )
	//{
		//if ( bHasMoveInput )
		//{
			//bUseControllerRotationYaw = true;
//			GetCharacterMovement()->bOrientRotationToMovement = false;
		//}

		//if ( CachedAnimInstance )
		//{
//			CachedAnimInstance->SetIsAiming(Value.Get<bool>());
		//}

		//if ( AimProgressTimeline )
		//{
		//	AimProgressTimeline->PlayFromStart();
	//	}
//
	//	ServerBeginAim(Value.Get<bool>());
	//}
	//

	if ( !WeaponSystemComponent ) return;

	//UE_LOG(LogTemp, Log, TEXT("PX_Character Client Begin Aim"));

	AimState.AddState(NewAimState);

	if ( AimState.ShouldBeginAim() )
	{
		//WeaponSystemComponent->ClientSubmitAction(FPXWeaponActionContext::Builder(EPXWeaponActionType::Aim_On));
	}



	bUseControllerRotationYaw = true;
	GetCharacterMovement()->bOrientRotationToMovement = false;

	ServerBeginAim(true);
}
*/

/*
void APX_Character::EndAim(const EAimState NewAimState)
{
	//UE_LOG(LogTemp, Log, TEXT("Client OTS Aim"));

	if ( HasAuthority() || !IsLocallyControlled() ) return;

	//
	//if ( bIsAiming )
	//{
	//	bUseControllerRotationYaw = false;
	//	GetCharacterMovement()->bOrientRotationToMovement = true;

//		if ( CachedAnimInstance )
	//	{
	//		CachedAnimInstance->SetIsAiming(Value.Get<bool>());
	//	}

	//	if ( AimProgressTimeline )
	//	{
	//		AimProgressTimeline->Reverse();
	//	}

	//	if ( bIsDrawing )
	//	{
	//		CachedAnimInstance->SetIsDrawing(Value.Get<bool>());
	//		CachedAnimInstance->SetDrawProgress(0.f);
	//	}

	//	ServerEndAim(Value.Get<bool>());
	//}
	
	if ( !WeaponSystemComponent ) return;

	//UE_LOG(LogTemp, Log, TEXT("PX_Character Client End Aim"));

	AimState.RemoveState(NewAimState);

	if ( AimState.ShouldEndAim() )
	{
		//WeaponSystemComponent->ClientSubmitAction(FPXWeaponActionContext::Builder(EPXWeaponActionType::Aim_Off));

		bUseControllerRotationYaw = false;
		GetCharacterMovement()->bOrientRotationToMovement = true;

		ServerEndAim(false);
	}

	
}
*/

void APX_Character::HandleEndAim()
{
	//UE_LOG(LogTemp, Log, TEXT("Client OTS Aim"));
	/*
	if ( HasAuthority() || !IsLocallyControlled() ) return;

	if ( AimState.GetState() < EAimState::Aim )
	{
		EndAim(false);
	}
	*/
}

void APX_Character::BeginOTSAim(const FInputActionValue& Value)
{
	//UE_LOG(LogTemp, Log, TEXT("Client Begin OTS Aim"));

	if ( HasAuthority() || !IsLocallyControlled() ) return;

	//BeginAim(EAimState::OTS);

	// Move Camera to OTS Aim Position
	CameraBoom->TargetArmLength = 90.0f;
	FollowCamera->SetRelativeLocation(FVector(0.0f, 45.0f, 80.0f));

	ServerBeginAim(true);

	if ( !bIsAiming )
	{
		if ( CachedAnimInstance )
		{
			CachedAnimInstance->SetIsAiming(Value.Get<bool>());
		}

		if ( AimProgressTimeline )
		{
			AimProgressTimeline->PlayFromStart();
		}

		// Move Camera to OTS Aim Position
		CameraBoom->TargetArmLength = 90.0f;
		FollowCamera->SetRelativeLocation(FVector(0.0f, 45.0f, 80.0f));

		//UE_LOG(LogTemp, Log, TEXT("Client Begin Aim.. Value : %s"), *Value.ToString());
		//ServerBeginOTSAim(Value.Get<bool>());
		ServerBeginAim(Value.Get<bool>());
	}
}

void APX_Character::EndOTSAim(const FInputActionValue& Value)
{
	UE_LOG(LogTemp, Log, TEXT("Client End OTS Aim"));

	if ( HasAuthority() || !IsLocallyControlled() ) return;

	//EndAim(EAimState::OTS);

	// Move Camera to Idle Position
	CameraBoom->TargetArmLength = 250.0f;
	FollowCamera->SetRelativeLocation(FVector(0.0f, 45.0f, 100.0f));

	if ( bIsAiming )
	{
		if ( CachedAnimInstance )
		{
			CachedAnimInstance->SetIsAiming(Value.Get<bool>());
		}

		if ( AimProgressTimeline )
		{
			AimProgressTimeline->Reverse();
		}

		if ( bIsDrawing )
		{
			CachedAnimInstance->SetIsDrawing(Value.Get<bool>());
			CachedAnimInstance->SetDrawProgress(0.f);
		}

		// Move Camera to Idle Position
		CameraBoom->TargetArmLength = 250.0f;
		FollowCamera->SetRelativeLocation(FVector(0.0f, 45.0f, 100.0f));

		//UE_LOG(LogTemp, Log, TEXT("Client End Aim.. Value : %s"), *Value.ToString());
		//ServerEndOTSAim(Value.Get<bool>());
		ServerEndAim(Value.Get<bool>());
	}
}

void APX_Character::BeginADS(const FInputActionValue& Value)
{
	//UE_LOG(LogTemp, Log, TEXT("Client Begin ADS"));

	if ( HasAuthority() || !IsLocallyControlled() ) return;

	//BeginAim(EAimState::ADS);

	// Move Camera to ADS Position
	CameraBoom->bDoCollisionTest = false;
	FollowCamera->SetActive(false);
	FPSCamera->SetActive(true);

	ServerBeginAim(true);

	/*
	if ( !bIsAiming )
	{
		if ( CachedAnimInstance )
		{
			CachedAnimInstance->SetIsAiming(Value.Get<bool>());
		}

		if ( AimProgressTimeline )
		{
			AimProgressTimeline->PlayFromStart();
		}

		// Move Camera to OTS Aim Position
		//CameraBoom->TargetArmLength = 0.0f;
		//FollowCamera->SetRelativeLocationAndRotation(FVector(-10.0f, 10.0f, 80.0f), FRotator(0.0f, 0.0f, 0.0f));

		CameraBoom->bDoCollisionTest = false;
		FollowCamera->SetActive(false);
		FPSCamera->SetActive(true);

		//UE_LOG(LogTemp, Log, TEXT("Client Begin Aim.. Value : %s"), *Value.ToString());
		//ServerBeginOTSAim(Value.Get<bool>());
		ServerBeginAim(Value.Get<bool>());
	}
	*/
}

void APX_Character::EndADS(const FInputActionValue& Value)
{
	//UE_LOG(LogTemp, Log, TEXT("Client End ADS"));

	if ( HasAuthority() || !IsLocallyControlled() ) return;

	//EndAim(EAimState::ADS);

	// Move Camera to Idle Position
	FPSCamera->SetActive(false);
	FollowCamera->SetActive(true);
	CameraBoom->bDoCollisionTest = true;

	/*
	if ( bIsAiming )
	{
		if ( CachedAnimInstance )
		{
			CachedAnimInstance->SetIsAiming(Value.Get<bool>());
		}

		if ( AimProgressTimeline )
		{
			AimProgressTimeline->Reverse();
		}

		if ( bIsDrawing )
		{
			CachedAnimInstance->SetIsDrawing(Value.Get<bool>());
			CachedAnimInstance->SetDrawProgress(0.f);
		}

		// Move Camera to Idle Position
		//CameraBoom->TargetArmLength = 250.0f;
		//FollowCamera->SetRelativeLocationAndRotation(FVector(-10.0f, 45.0f, 80.0f), FRotator(-5.0f, CameraOffset, 0.0f));

		FPSCamera->SetActive(false);
		FollowCamera->SetActive(true);
		CameraBoom->bDoCollisionTest = true;

		//UE_LOG(LogTemp, Log, TEXT("Client End Aim.. Value : %s"), *Value.ToString());
		//ServerEndOTSAim(Value.Get<bool>());
		ServerEndAim(Value.Get<bool>());
	}
	*/
}

void APX_Character::BeginDraw(const FInputActionInstance& Instance)
{
	if ( HasAuthority() || !IsLocallyControlled() ) return;

	/*
	// 처음 눌리는 순간만 타임스탬프 갱신
	if ( !bWantsDraw )
	{
		DrawInputStartTime = GetWorld()->GetTimeSeconds();

		// (선택) 눌릴 때마다 스로틀 기준 초기화
		LastDrawProgressSentTime = 0.f;
		LastDrawProgressSentValue = 0.f;
	}

	bWantsDraw = true;
	bFireOnDrawRelease = true; // 드로우 해제 시 Attack 실행
	*/

	/*
	const float Elapsed = FMath::Clamp(Instance.GetElapsedTime(), 0.f, 1.f);
	//UE_LOG(LogTemp, Log, TEXT("Client Begin Draw.. Elapsed Time : %f"), Elapsed);

	if ( !bIsAiming )
	{
		BeginAim(true);
		AimState.AddState(EAimState::Aim);
	}

	if ( !bIsDrawing )
	{
		if ( CachedAnimInstance )
		{
			CachedAnimInstance->SetIsDrawing(true);
			CachedAnimInstance->SetDrawProgress(Elapsed);
		}

		ServerBeginDraw(true);
		ServerUpdateDrawProgress(Elapsed);
	}

	if ( CachedAnimInstance )
	{
		CachedAnimInstance->SetDrawProgress(Elapsed);
	}

	ServerUpdateDrawProgress(Elapsed);
	*/

	//UE_LOG(LogTemp, Log, TEXT("Draw Progress : %f"), CachedAnimInstance->GetDrawProgress());

	/*
	// 주기·임계값으로 스로틀링
	const float Now = GetWorld()->GetTimeSeconds();
	const bool bTimeOk = (Now - LastDrawProgressSentTime) >= 0.05f;					// 20Hz 정도
	const bool bDeltaOk = FMath::Abs(Elapsed - LastDrawProgressSentValue) >= 0.02f; // 2% 이상 변화

	if (bTimeOk || bDeltaOk)
	{
		LastDrawProgressSentTime = Now;
		LastDrawProgressSentValue = Elapsed;
		ServerUpdateDrawProgress(Elapsed);
	}
	*/
}

void APX_Character::EndDraw(const FInputActionInstance& Instance)
{
	//UE_LOG(LogTemp, Log, TEXT("Client End Draw"));

	if ( HasAuthority() || !IsLocallyControlled() ) return;

	//bWantsDraw = false;

	/*
	if ( bIsDrawing )
	{
		AimState.RemoveState(EAimState::Aim);

		if ( CachedAnimInstance )
		{
			CachedAnimInstance->SetIsDrawing(false);
			CachedAnimInstance->SetDrawProgress(0.f);
		}

		ServerEndDraw(false);
	}

	// New unified action pipeline
	if (WeaponSystemComponent)
	{
		//WeaponSystemComponent->TryStartAction(EPXWeaponActionType::Fire);
	}
	*/
}
#endif

void APX_Character::SetLocomotionJumping(bool bNewIsJumping)
{
	bIsJumping = bNewIsJumping;

#if !UE_SERVER
	if ( CachedAnimInstance )
	{
		CachedAnimInstance->SetIsJumping(bNewIsJumping);
		if ( bNewIsJumping )
		{
			CachedAnimInstance->SetIsFalling(true);
		}
	}
#endif

	if ( bNewIsJumping )
	{
		Jump();
	}
	else
	{
		StopJumping();
	}
}

void APX_Character::SetLocomotionCrouching(bool bNewIsCrouching)
{
	bIsCrouching = bNewIsCrouching;

#if !UE_SERVER
	if ( CachedAnimInstance )
	{
		CachedAnimInstance->SetIsCrouching(bNewIsCrouching);
	}
#endif

	if ( bNewIsCrouching )
	{
		Crouch();
	}
	else
	{
		UnCrouch();
	}

	ApplyLocomotionSpeedMode();
}

void APX_Character::ApplyLocomotionSpeedMode()
{
	UCharacterMovementComponent* MovementComp = GetCharacterMovement();
	if ( !MovementComp )
	{
		return;
	}

	float BaseMoveSpeed = 600.0f;
	float SprintSpeedMultiplier = 1.5f;

	if ( UAbilitySystemComponent* ASC = GetAbilitySystemComponent() )
	{
		BaseMoveSpeed = ASC->GetNumericAttribute(UPX_MovementAttributeSet::GetMoveSpeedAttribute());
		SprintSpeedMultiplier = ASC->GetNumericAttribute(UPX_MovementAttributeSet::GetSprintSpeedMultiplierAttribute());

		if ( ASC->HasMatchingGameplayTag(PX_GameplayTags::State_Locomotion_Crouching) )
		{
			MovementComp->MaxWalkSpeed = BaseMoveSpeed * 0.5f;
			MovementComp->MaxWalkSpeedCrouched = BaseMoveSpeed * 0.5f;
			return;
		}

		if ( ASC->HasMatchingGameplayTag(PX_GameplayTags::State_Locomotion_Sprinting) )
		{
			MovementComp->MaxWalkSpeed = BaseMoveSpeed * SprintSpeedMultiplier;
			return;
		}

		if ( ASC->HasMatchingGameplayTag(PX_GameplayTags::State_Locomotion_Walking) )
		{
			MovementComp->MaxWalkSpeed = BaseMoveSpeed * 0.5f;
			return;
		}
	}

	MovementComp->MaxWalkSpeed = BaseMoveSpeed;
	MovementComp->MaxWalkSpeedCrouched = BaseMoveSpeed * 0.5f;
}

void APX_Character::ClearGameplayInputStateForUI()
{
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

#if !UE_SERVER
	if ( CachedAnimInstance )
	{
		CachedAnimInstance->SetIsAiming(false);
		CachedAnimInstance->SetIsCrouching(false);
	}
#endif

	if ( !HasAuthority() && IsLocallyControlled() )
	{
		ServerClearGameplayInputStateForUI();
	}
}

void APX_Character::SetGameplayInputBlockedForUI(bool bNewBlocked)
{
	bGameplayInputBlockedForUI = bNewBlocked;
}

void APX_Character::SetHasEquippedWeapon(bool bNewHasEquippedWeapon)
{
#if !UE_SERVER
	if ( CachedAnimInstance )
	{
		CachedAnimInstance->SetHasEquippedWeapon(bNewHasEquippedWeapon);
	}
#endif
}

void APX_Character::FirePressed()
{
	UE_LOG(LogTemp, Log, TEXT("Client Fire Pressed"));
	/*
	BeginAim(EAimState::Aim);

	if (WeaponSystemComponent)
	{
		WeaponSystemComponent->ClientSubmitAction(FPXWeaponActionContext::Builder(EPXWeaponActionType::AttackPressed));
	}
	*/
}

void APX_Character::FireReleased()
{
	UE_LOG(LogTemp, Log, TEXT("Client Fire Released"));
	/*
	if ( WeaponSystemComponent )
	{
		WeaponSystemComponent->ClientSubmitAction(FPXWeaponActionContext::Builder(EPXWeaponActionType::AttackReleased));
	}

	EndAim(EAimState::Aim);
	*/
}

void APX_Character::Reload()
{
	UE_LOG(LogTemp, Log, TEXT("Client Begin Reload"));
	/*
	if (WeaponSystemComponent)
	{
		WeaponSystemComponent->ClientSubmitAction(FPXWeaponActionContext::Builder(EPXWeaponActionType::BeginReload));
	}
	*/
}

void APX_Character::EquipSlot(const FInputActionValue& Value)
{
    const int32 SlotIndex = FMath::RoundToInt(Value.Get<float>() - 1.0f);
    UE_LOG(LogTemp, Log, TEXT("Equip Index %d Weapon"), SlotIndex);
	EquipSlotByIndex(SlotIndex);
}

void APX_Character::EquipSlotByIndex(int32 SlotIndex)
{
	if ( SlotIndex == INDEX_NONE || !WeaponSystemComponent )
	{
		PX_LOG(Warning, TEXT("Invalid slot index or WeaponSystemComponent is null"));
		return;
	}

	if ( !InventoryComponent || !InventoryComponent->IsInventoryReady() )
	{
		PX_LOG(Warning, TEXT("Inventory is not ready"));
		return;
	}

	UPX_WeaponItemInstance* NewWeaponItemInstance = InventoryComponent->GetWeaponItemInstanceBySlot(SlotIndex);
	if ( !NewWeaponItemInstance )
	{
		PX_LOG(Warning, TEXT("No weapon item instance found in slot %d"), SlotIndex);
		return;
	}

	FGuid NewWeaponInstanceId = NewWeaponItemInstance->GetInstanceId();

	// 같은 무기 재장착 방지
	if ( NewWeaponInstanceId == WeaponSystemComponent->GetWeaponItemInstanceId() )
	{
		PX_LOG(Warning, TEXT("Weapon in slot %d is already equipped"), SlotIndex);
		return;
	}

	UPX_AbilitySystemComponent* PX_ASC = Cast<UPX_AbilitySystemComponent>(GetAbilitySystemComponent());
	if ( !PX_ASC )
	{
		PX_LOG(Warning, TEXT("PX_ASC is Invalid."));
	}

	if ( !WeaponSystemComponent )
	{
		PX_LOG(Warning, TEXT("WeaponSystemComponent is Invalid."));
	}

	// 무기 변경 시 기존 무기의 액션이 남아있을 수 있으므로 관련 액션 태그를 가진 능력들을 취소
	PX_LOG(Log, TEXT("Canceling abilities with tags: Ability_Weapon_Equip, Ability_Weapon_Reload, Ability_Weapon_Attack"));
	FGameplayTagContainer CancelTags;
	//CancelTags.AddTag(PX_GameplayTags::Ability_Weapon_Equip);	// 무기 변경은 Cancel이 아니라 Equip_Chance로 분기하도록 변경
	CancelTags.AddTag(PX_GameplayTags::Ability_Weapon_Reload);
	CancelTags.AddTag(PX_GameplayTags::Ability_Weapon_Attack);
	PX_ASC->CancelAbilities(&CancelTags, nullptr, nullptr);

	// Event 생성
	FGameplayEventData EventData;
	
	EventData.Instigator = this;
	EventData.Target = this;
	EventData.EventMagnitude = static_cast<float>(SlotIndex);

	if ( PX_ASC->HasMatchingGameplayTag(PX_GameplayTags::State_Combat_Equipping) )
	{
		PX_LOG(Log, TEXT("Equip Action is still Active.."));
		if ( WeaponSystemComponent && WeaponSystemComponent->Local_IsEquipCancelable() )
		{
			PX_LOG(Log, TEXT("Change Equip Weapon"));
			WeaponSystemComponent->Local_ChangeEquipBySlot(SlotIndex);
			return;
		}
		PX_LOG(Log, TEXT("Abort input"));
		return;
	}

	// Equip 중이 아니면 Equip 시작
	EventData.EventTag = PX_GameplayTags::Event_Weapon_Equip_Begin;
	PX_LOG(Log, TEXT("Call Action with Event Tag: %s"), *EventData.EventTag.ToString());
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, PX_GameplayTags::Event_Weapon_Equip_Begin, EventData);
}

void APX_Character::SwitchFireMode()
{
	UE_LOG(LogTemp, Log, TEXT("Switch Fire Mode"));
	/*
	if ( APX_Weapon* Weapon = WeaponSystemComponent->GetWeapon() )
	{
		Weapon->SwitchFireMode();
	}
	*/
	/*
	if ( WeaponSystemComponent )
	{
		//WeaponSystemComponent->ClientSubmitAction(FPXWeaponActionContext::Builder(EPXWeaponActionType::SetFireMode));
	}
	*/
}

void APX_Character::ToggleInventory()
{
	UE_LOG(LogTemp, Log, TEXT("Toggle Inventory"));

	if ( APX_PlayerController* PX_PlayerController = Cast<APX_PlayerController>(GetController()) )
	{
		PX_PlayerController->ToggleInventory();
	}
}

void APX_Character::OnRep_Jump()
{
	if ( CachedAnimInstance )
	{
		CachedAnimInstance->SetIsJumping(bIsJumping);
	}
}

void APX_Character::OnRep_Crouch()
{
	if ( CachedAnimInstance )
	{
		//UE_LOG(LogTemp, Log, TEXT("OnRep_Crouch(): bIsCrouching changed from %s to %s"), bIsCrouching ? TEXT("false") : TEXT("true"), bIsCrouching ? TEXT("true") : TEXT("false"));
		CachedAnimInstance->SetIsCrouching(bIsCrouching);
	}
}

void APX_Character::SetIsAiming(bool bNewIsAiming)
{
	PX_LOG(Log, TEXT(""));

	bIsAiming = bNewIsAiming;

	if ( AimProgressTimeline )
	{
		if ( bIsAiming )
		{
			AimProgressTimeline->PlayFromStart();
		}
		else
		{
			AimProgressTimeline->Reverse();
		}
	}
}

void APX_Character::ApplyAimCameraMode(bool bWantsADS, bool bWantsOTS)
{
	if ( !IsLocallyControlled() )
	{
		return;
	}

	if ( bWantsADS )
	{
		if ( CameraBoom )
		{
			CameraBoom->bDoCollisionTest = false;
		}
		if ( FollowCamera )
		{
			FollowCamera->SetActive(false);
		}
		if ( FPSCamera )
		{
			FPSCamera->SetActive(true);
		}
		return;
	}

	if ( FPSCamera )
	{
		FPSCamera->SetActive(false);
	}
	if ( FollowCamera )
	{
		FollowCamera->SetActive(true);
	}
	if ( CameraBoom )
	{
		CameraBoom->bDoCollisionTest = true;
	}

	if ( bWantsOTS )
	{
		if ( CameraBoom )
		{
			CameraBoom->TargetArmLength = 90.0f;
		}
		if ( FollowCamera )
		{
			FollowCamera->SetRelativeLocation(FVector(0.0f, 45.0f, 80.0f));
		}
		return;
	}

	if ( CameraBoom )
	{
		CameraBoom->TargetArmLength = 250.0f;
	}
	if ( FollowCamera )
	{
		FollowCamera->SetRelativeLocation(FVector(0.0f, 45.0f, 100.0f));
	}
}

void APX_Character::OnRep_AimState()
{
	PX_LOG(Log, TEXT(""));

	if (CachedAnimInstance)
	{
		CachedAnimInstance->SetIsAiming(bIsAiming);
	}

	if (AimProgressTimeline)
	{
		if (bIsAiming)
		{
			AimProgressTimeline->PlayFromStart();
		}
		else
		{
			AimProgressTimeline->Reverse();
		}
	}
}

void APX_Character::OnRep_DrawState()
{
#if !UE_SERVER
	//UE_LOG(LogTemp, Log, TEXT("OnRep Draw.."));
	if (CachedAnimInstance)
	{
		CachedAnimInstance->SetIsDrawing(bIsDrawing);
		if (!bIsDrawing)
		{
			CachedAnimInstance->SetDrawProgress(0.f);
		}
	}
#endif
}

void APX_Character::OnRep_DrawProgress()
{
#if !UE_SERVER
	//UE_LOG(LogTemp, Log, TEXT("OnRep DrawProgress.. %f"), DrawProgress);
	if (CachedAnimInstance)
	{
		CachedAnimInstance->SetDrawProgress(DrawProgress);
	}
#endif
}

void APX_Character::AimProgressUpdate(float Value)
{
#if !UE_SERVER
	//UE_LOG(LogTemp, Log, TEXT("AimProgressUpdate.. AimProgress : %f"), Value);
	if ( CachedAnimInstance )
	{
		CachedAnimInstance->SetAimProgress(Value);
	}
#endif
}

void APX_Character::ComparisonBeginMoveInput(const FVector2D& MovementVector)
{
#if !UE_SERVER
	if ( bGameplayInputBlockedForUI || !Controller )
	{
		return;
	}

	const FRotator Rotation = Controller->GetControlRotation();
	const FRotator YawRotation(0, Rotation.Yaw + CameraOffset, 0);
	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	AddMovementInput(ForwardDirection, MovementVector.Y);
	AddMovementInput(RightDirection, MovementVector.X);

	bHasMoveInput = !MovementVector.IsNearlyZero();
	bUseControllerRotationYaw = bIsAiming;
	if ( UCharacterMovementComponent* MovementComp = GetCharacterMovement() )
	{
		MovementComp->bOrientRotationToMovement = !bIsAiming;
	}

	LastMoveSpeed = GetVelocity().Size2D();
	LastMoveDirection = ResolveMoveDirection(MovementVector);
	ServerBeginMove(bHasMoveInput, LastMoveSpeed, LastMoveDirection);
#endif
}

void APX_Character::ComparisonEndMoveInput()
{
#if !UE_SERVER
	EndMove();
#endif
}

void APX_Character::ComparisonPressAbilityInput(FGameplayTag InputTag)
{
#if !UE_SERVER
	AbilityInputPressed(InputTag);
#endif
}

void APX_Character::ComparisonReleaseAbilityInput(FGameplayTag InputTag)
{
#if !UE_SERVER
	AbilityInputReleased(InputTag);
#endif
}

void APX_Character::ComparisonEquipSlot(int32 SlotIndex)
{
#if !UE_SERVER
	EquipSlotByIndex(SlotIndex);
#endif
}
