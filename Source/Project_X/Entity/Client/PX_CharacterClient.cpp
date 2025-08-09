// Fill out your copyright notice in the Description page of Project Settings.


#include "Entity/Client/PX_CharacterClient.h"

// Sets default values
APX_CharacterClient::APX_CharacterClient()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Setup Timeline Component
	AimProgressTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("AimProgressTimeline"));

	// Setup Curve
	static ConstructorHelpers::FObjectFinder<UCurveFloat> Curve(TEXT("/Game/Project_X/Character/PX_Character/Animations/AimProgressCurve.AimProgressCurve"));
	if (Curve.Succeeded())
	{
		AimProgressCurve = Curve.Object;
	}
}
// Called when the game starts or when spawned
void APX_CharacterClient::BeginPlay()
{
	Super::BeginPlay();

	// Cache PX_CharacterAnimInstance
	PX_CharacterAnimInstance = Cast<UPX_CharacterAnimInstance>(GetMesh()->GetAnimInstance());

	if (AimProgressCurve && AimProgressTimeline)
	{
		FOnTimelineFloat AimProgressUpdate;
		AimProgressUpdate.BindUFunction(this, FName("AimProgressUpdate"));
		AimProgressTimeline->AddInterpFloat(AimProgressCurve, AimProgressUpdate);
		AimProgressTimeline->SetLooping(false);
	}
}

// Called to bind functionality to input
void APX_CharacterClient::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
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
		EnhancedInputComponent->BindAction(GetMoveAction(), ETriggerEvent::Triggered, this, &APX_CharacterClient::Move);
		// Looking
		EnhancedInputComponent->BindAction(GetLookAction(), ETriggerEvent::Triggered, this, &APX_CharacterClient::Look);
		// Aiming
		EnhancedInputComponent->BindAction(GetAimAction(), ETriggerEvent::Started, this, &APX_CharacterClient::BeginAim);
		EnhancedInputComponent->BindAction(GetAimAction(), ETriggerEvent::Completed, this, &APX_CharacterClient::EndAim);
		// Drawing
		EnhancedInputComponent->BindAction(GetDrawAction(), ETriggerEvent::Ongoing, this, &APX_CharacterClient::BeginDraw);
		EnhancedInputComponent->BindAction(GetDrawAction(), ETriggerEvent::Canceled, this, &APX_CharacterClient::EndDraw);
		EnhancedInputComponent->BindAction(GetDrawAction(), ETriggerEvent::Completed, this, &APX_CharacterClient::EndDraw);
	}
}

void APX_CharacterClient::Move(const FInputActionValue& Value)
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

void APX_CharacterClient::Look(const FInputActionValue& Value)
{
	const FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void APX_CharacterClient::BeginAim(const FInputActionValue& Value)
{
	if (PX_CharacterAnimInstance && !PX_CharacterAnimInstance->GetIsAiming())
	{
		UE_LOG(LogTemp, Log, TEXT("Begin Aim.. Value : %s"), *Value.ToString());
		PX_CharacterAnimInstance->SetIsAiming(Value.Get<bool>());
		AimProgressTimeline->PlayFromStart();
	}
}

void APX_CharacterClient::EndAim(const FInputActionValue& Value)
{
	if (PX_CharacterAnimInstance && PX_CharacterAnimInstance->GetIsAiming())
	{
		UE_LOG(LogTemp, Log, TEXT("End Aim.. Value : %s"), *Value.ToString());
		PX_CharacterAnimInstance->SetIsAiming(Value.Get<bool>());
		AimProgressTimeline->Reverse();

		if (PX_CharacterAnimInstance->GetIsDrawing())
		{
			PX_CharacterAnimInstance->SetIsDrawing(Value.Get<bool>());
			PX_CharacterAnimInstance->SetDrawProgress(0.f);
		}
	}
}

void APX_CharacterClient::BeginDraw(const FInputActionInstance& Instance)
{
	if (PX_CharacterAnimInstance && PX_CharacterAnimInstance->GetIsAiming())
	{
		UE_LOG(LogTemp, Log, TEXT("Begin Draw.. Elapsed Time : %f"), FMath::Clamp(Instance.GetElapsedTime(), 0.f, 1.f));
		if (!PX_CharacterAnimInstance->GetIsDrawing())
		{
			PX_CharacterAnimInstance->SetIsDrawing(true);
		}

		PX_CharacterAnimInstance->SetDrawProgress(FMath::Clamp(Instance.GetElapsedTime(), 0.f, 1.f));
		UE_LOG(LogTemp, Log, TEXT("Draw Progress : %f"), PX_CharacterAnimInstance->GetDrawProgress());
	}
}

void APX_CharacterClient::EndDraw(const FInputActionInstance& Instance)
{
	if (PX_CharacterAnimInstance && PX_CharacterAnimInstance->GetIsAiming())
	{
		UE_LOG(LogTemp, Log, TEXT("End Draw.. Elapsed Time : %f"), FMath::Clamp(Instance.GetElapsedTime(), 0.f, 1.f));
		if (PX_CharacterAnimInstance && PX_CharacterAnimInstance->GetIsDrawing())
		{
			PX_CharacterAnimInstance->SetIsDrawing(false);
			PX_CharacterAnimInstance->SetDrawProgress(0.f);
		}
	}
}

void APX_CharacterClient::AimProgressUpdate(float Value)
{
	UE_LOG(LogTemp, Log, TEXT("AimProgressUpdate.. AimProgress : %f"), Value);
	PX_CharacterAnimInstance->SetAimProgress(Value);
}
