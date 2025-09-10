// Fill out your copyright notice in the Description page of Project Settings.

#include "Entity/PX_Character.h"

#if !UE_SERVER
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "PX_CharacterAnimInstance.h"
#include "Components/TimelineComponent.h"
#include "Entity/PX_Weapon.h"
#include "Component/PX_WeaponComponent.h"
#include "Blueprint/UserWidget.h"
#endif

// Called when the game starts or when spawned
void APX_Character::BeginPlay()
{
	Super::BeginPlay();

#if !UE_SERVER
	// Cache PX_CharacterAnimInstance
	CachedAnimInstance = Cast<UPX_CharacterAnimInstance>(GetMesh()->GetAnimInstance());

	if (AimProgressCurve && AimProgressTimeline)
	{
		FOnTimelineFloat AimProgressUpdate;
		AimProgressUpdate.BindUFunction(this, FName("AimProgressUpdate"));
		AimProgressTimeline->AddInterpFloat(AimProgressCurve, AimProgressUpdate);
		AimProgressTimeline->SetLooping(false);
	}

	// Spawn and Attach Bow to PX_Character
	FName BowSocket(TEXT("Bow_L"));
	APX_Weapon* Bow = GetWorld()->SpawnActor<APX_Weapon>(FVector::ZeroVector, FRotator::ZeroRotator);
	if (Bow) {
		Bow->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, BowSocket);
	}

	// Create User Widget
	if (IsLocallyControlled() && CrosshairWidgetClass)
	{
		if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
		{
			CrosshairWidget = CreateWidget<UUserWidget>(PlayerController, CrosshairWidgetClass);
			if (CrosshairWidget)
			{
				CrosshairWidget->AddToViewport();
			}
		}
	}

#endif
} 

// Called every frame
void APX_Character::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

#if !UE_SERVER
#endif
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
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Moving
		EnhancedInputComponent->BindAction(GetMoveAction(), ETriggerEvent::Triggered, this, &APX_Character::Move);
		// Looking
		EnhancedInputComponent->BindAction(GetLookAction(), ETriggerEvent::Triggered, this, &APX_Character::Look);
		// Aiming
		EnhancedInputComponent->BindAction(GetAimAction(), ETriggerEvent::Started, this, &APX_Character::BeginAim);
		EnhancedInputComponent->BindAction(GetAimAction(), ETriggerEvent::Completed, this, &APX_Character::EndAim);
		// Drawing
		EnhancedInputComponent->BindAction(GetDrawAction(), ETriggerEvent::Ongoing, this, &APX_Character::BeginDraw);
		EnhancedInputComponent->BindAction(GetDrawAction(), ETriggerEvent::Canceled, this, &APX_Character::EndDraw);
		EnhancedInputComponent->BindAction(GetDrawAction(), ETriggerEvent::Completed, this, &APX_Character::EndDraw);
	}
}

void APX_Character::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	const FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void APX_Character::Look(const FInputActionValue& Value)
{
	const FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void APX_Character::BeginAim(const FInputActionValue& Value)
{
	if (!(IsLocallyControlled() && !HasAuthority())) return;

	if (CachedAnimInstance && !CachedAnimInstance->GetIsAiming())
	{
		//UE_LOG(LogTemp, Log, TEXT("Client Begin Aim.. Value : %s"), *Value.ToString());
		CachedAnimInstance->SetIsAiming(Value.Get<bool>());

		if (AimProgressTimeline) {
			AimProgressTimeline->PlayFromStart();
		}

		ServerBeginAim(Value.Get<bool>());
	}
}

void APX_Character::EndAim(const FInputActionValue& Value)
{
	if (!(IsLocallyControlled() && !HasAuthority())) return;

	if (CachedAnimInstance && CachedAnimInstance->GetIsAiming())
	{
		//UE_LOG(LogTemp, Log, TEXT("Client End Aim.. Value : %s"), *Value.ToString());
		CachedAnimInstance->SetIsAiming(Value.Get<bool>());

		if (AimProgressTimeline) {
			AimProgressTimeline->Reverse();
		}

		if (CachedAnimInstance->GetIsDrawing())
		{
			CachedAnimInstance->SetIsDrawing(Value.Get<bool>());
			CachedAnimInstance->SetDrawProgress(0.f);
		}

		ServerEndAim(Value.Get<bool>());
	}
}

void APX_Character::BeginDraw(const FInputActionInstance& Instance)
{
	if (!(IsLocallyControlled() && !HasAuthority())) return;

	if (CachedAnimInstance && CachedAnimInstance->GetIsAiming())
	{
		const float Elapsed = FMath::Clamp(Instance.GetElapsedTime(), 0.f, 1.f);
		//UE_LOG(LogTemp, Log, TEXT("Client Begin Draw.. Elapsed Time : %f"), Elapsed);

		if (!CachedAnimInstance->GetIsDrawing())
		{
			CachedAnimInstance->SetIsDrawing(true);

			ServerBeginDraw(true);
		}

		CachedAnimInstance->SetDrawProgress(Elapsed);
		//UE_LOG(LogTemp, Log, TEXT("Draw Progress : %f"), CachedAnimInstance->GetDrawProgress());

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

		// 스로틀링 없이 RPC
		// ServerUpdateDrawProgress(Elapsed);
	}	
}

void APX_Character::EndDraw(const FInputActionInstance& Instance)
{
	if (!(IsLocallyControlled() && !HasAuthority())) return;
	
	if (CachedAnimInstance && CachedAnimInstance->GetIsAiming())
	{
		//UE_LOG(LogTemp, Log, TEXT("Client End Draw.. Elapsed Time : %f"), FMath::Clamp(Instance.GetElapsedTime(), 0.f, 1.f));
		if (CachedAnimInstance && CachedAnimInstance->GetIsDrawing())
		{
			CachedAnimInstance->SetIsDrawing(false);
			CachedAnimInstance->SetDrawProgress(0.f);

			ServerEndDraw(false);

			if (Weapon)
			{
				Weapon->Attack();
			}
		}
	}
	
}
#endif

void APX_Character::OnRep_AimState()
{
#if !UE_SERVER
	//UE_LOG(LogTemp, Log, TEXT("OnRep Aim.."));

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
#endif
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
	CachedAnimInstance->SetAimProgress(Value);
#endif
}