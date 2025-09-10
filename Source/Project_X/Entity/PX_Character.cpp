// Fill out your copyright notice in the Description page of Project Settings.


#include "PX_Character.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "InputMappingContext.h"
#include "InputAction.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/TimelineComponent.h"
#include "Net/UnrealNetwork.h"
#include "Component/PX_WeaponComponent.h"
// 크로스헤어
#include "Blueprint/UserWidget.h"

// Sets default values
APX_Character::APX_Character()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Setup SpringArm Component
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 250.0f;
	CameraBoom->bUsePawnControlRotation = true;

	// Setup Camera Component
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom);
	FollowCamera->SetRelativeLocation(FVector(0.0f, 75.0f, 75.0f));
	FollowCamera->SetRelativeRotation(FRotator(-5.0f, 0.0f, 0.0f));

	// Setup Skeletal Mesh
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> SkeletalMeshAsset(TEXT("/Game/Project_X/Character/PX_Character/Meshes/SK_Mannequin.SK_Mannequin"));
	if (SkeletalMeshAsset.Succeeded())
	{
		GetMesh()->SetSkeletalMesh(SkeletalMeshAsset.Object);
	}
	GetMesh()->SetRelativeLocation(FVector(0.0f, 0.0f, -90.0f));
	GetMesh()->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));

	// Setup Weapon Component
	Weapon = CreateDefaultSubobject<UPX_WeaponComponent>(TEXT("Weapon"));
	Weapon->SetupAttachment(RootComponent);

	// Setup Input Mapping Context
	static ConstructorHelpers::FObjectFinder<UInputMappingContext> IMC_Default(TEXT("/Game/Project_X/Input/PX_IMC_Default.PX_IMC_Default"));
	if (IMC_Default.Succeeded())
	{
		DefaultMappingContext = IMC_Default.Object;
	}

	// Setup Move Input Action
	static ConstructorHelpers::FObjectFinder<UInputAction> IA_Move(TEXT("/Game/Project_X/Input/Action/PX_IA_Move.PX_IA_Move"));
	if (IA_Move.Succeeded())
	{
		MoveAction = IA_Move.Object;
	}

	// Setup Look Input Action
	static ConstructorHelpers::FObjectFinder<UInputAction> IA_Look(TEXT("/Game/Project_X/Input/Action/PX_IA_Look.PX_IA_Look"));
	if (IA_Look.Succeeded())
	{
		LookAction = IA_Look.Object;
	}

	// Setup Aim Input Action
	static ConstructorHelpers::FObjectFinder<UInputAction> IA_Aim(TEXT("/Game/Project_X/Input/Action/PX_IA_Aim.PX_IA_Aim"));
	if (IA_Aim.Succeeded())
	{
		AimAction = IA_Aim.Object;
	}

	// Setup Draw Input Action
	static ConstructorHelpers::FObjectFinder<UInputAction> IA_Draw(TEXT("/Game/Project_X/Input/Action/PX_IA_Draw.PX_IA_Draw"));
	if (IA_Draw.Succeeded())
	{
		DrawAction = IA_Draw.Object;
	}

	// Setup AnimInstance Class
	static ConstructorHelpers::FClassFinder<UAnimInstance> AnimClass(TEXT("/Game/Project_X/Character/PX_Character/Animations/BP_PXCharacter.BP_PXCharacter_C"));
	if (AnimClass.Class)
	{
		AnimInstanceClass = AnimClass.Class;
		GetMesh()->SetAnimInstanceClass(AnimInstanceClass);
	}

	// Setup Character Properties
	bUseControllerRotationYaw = false;

	// Setup Character Movement Component Properties
	GetCharacterMovement()->bOrientRotationToMovement = true;

#if !UE_SERVER
	// Setup Timeline Component
	AimProgressTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("AimProgressTimeline"));

	// Setup Curve
	static ConstructorHelpers::FObjectFinder<UCurveFloat> Curve(TEXT("/Game/Project_X/Character/PX_Character/Animations/AimProgressCurve.AimProgressCurve"));
	if (Curve.Succeeded())
	{
		AimProgressCurve = Curve.Object;
	}

	// Setup User Widget
	static ConstructorHelpers::FClassFinder<UUserWidget> CrosshairBPClass(TEXT("/Game/Project_X/CrossHair.CrossHair_C"));
	if (CrosshairBPClass.Succeeded())
	{
		CrosshairWidgetClass = CrosshairBPClass.Class;
	}
#endif
}

void APX_Character::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APX_Character, bIsAiming);
	DOREPLIFETIME(APX_Character, bIsDrawing);
	DOREPLIFETIME(APX_Character, DrawProgress);
}
