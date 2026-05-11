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
#include "Component/Weapon/PX_WeaponSystemComponent.h"
#include "Component/Inventory/PX_InventoryComponent.h"
#include "Component/Demo/PX_DemoBotComponent.h"
#include "Components/CapsuleComponent.h"
#include "Entity/Client/PX_CharacterAnimInstance.h"
#include "Framework/PlayerState/PX_PlayerState.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/Component/PX_AbilitySystemComponent.h"
#include "AbilitySystem/Attributes/PX_CombatAttributeSet.h"
#include "AbilitySystem/Attributes/PX_MovementAttributeSet.h"
#include "AbilitySystem/Attributes/PX_ResourceAttributeSet.h"
#include "Input/Config/PX_InputConfigDataAsset.h"
#include "Framework/PlayerState/PX_PlayerState.h"
#include "AbilitySystem/Tags/PX_GameplayTags.h"
#include "Component/Inventory/PX_WeaponItemInstance.h"
#include "Component/Inventory/PX_EquippableItemDataAsset.h"
#include "Component/UI/PX_TargetStatusComponent.h"

// 크로스헤어
#include "Blueprint/UserWidget.h"

// Sets default values
APX_Character::APX_Character()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Setup Skeletal Mesh
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> SkeletalMeshAsset(TEXT("/Game/Project_X/Character/PX_Character/Meshes/SK_Mannequin.SK_Mannequin"));
	TObjectPtr<USkeletalMeshComponent> SkeletalMesh = GetMesh();
	if ( SkeletalMeshAsset.Succeeded() )
	{
		SkeletalMesh->SetSkeletalMesh(SkeletalMeshAsset.Object);
	}
	SkeletalMesh->SetRelativeLocation(FVector(0.0f, 0.0f, -90.0f));
	SkeletalMesh->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
	SkeletalMesh->SetCollisionProfileName("PX_SkelMesh");
	//SkeletalMesh->SetNotifyRigidBodyCollision(true);		// OnHit()

	// Setup SpringArm Component
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 250.0f;
	CameraBoom->bUsePawnControlRotation = true;

	// Setup Camera Component
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom);
	FollowCamera->SetRelativeLocation(FVector(0.0f, 45.0f, 100.0f));
	FollowCamera->SetRelativeRotation(FRotator(-5.0f, CameraOffset, 0.0f));

	FPSCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FPSCamera"));
	FPSCamera->SetupAttachment(GetMesh(), TEXT("head"));
	FPSCamera->SetRelativeLocation(FVector(0.f, 15.f, 0.f));
	FPSCamera->bUsePawnControlRotation = true;
	FPSCamera->SetActive(false);

	// Setup Weapon Component
	//WeaponComponent = CreateDefaultSubobject<UPX_WeaponComponent>(TEXT("Weapon"));
	//WeaponComponent->SetupAttachment(RootComponent);

	// Setup Weapon System Component (new)
	WeaponSystemComponent = CreateDefaultSubobject<UPX_WeaponSystemComponent>(TEXT("WeaponSystem"));

	// Setup Inventory Component
	InventoryComponent = CreateDefaultSubobject<UPX_InventoryComponent>(TEXT("Inventory"));

	// Setup target status display data.
	TargetStatusComponent = CreateDefaultSubobject<UPX_TargetStatusComponent>(TEXT("TargetStatus"));

	CharacterAbilitySystemComponent = CreateDefaultSubobject<UPX_AbilitySystemComponent>(TEXT("CharacterAbilitySystemComponent"));
	CharacterResourceAttributeSet = CreateDefaultSubobject<UPX_ResourceAttributeSet>(TEXT("CharacterResourceSet"));
	CharacterCombatAttributeSet = CreateDefaultSubobject<UPX_CombatAttributeSet>(TEXT("CharacterCombatSet"));
	CharacterMovementAttributeSet = CreateDefaultSubobject<UPX_MovementAttributeSet>(TEXT("CharacterMovementSet"));

	CharacterAbilitySystemComponent->AddAttributeSetSubobject<UPX_ResourceAttributeSet>(CharacterResourceAttributeSet);
	CharacterAbilitySystemComponent->AddAttributeSetSubobject<UPX_CombatAttributeSet>(CharacterCombatAttributeSet);
	CharacterAbilitySystemComponent->AddAttributeSetSubobject<UPX_MovementAttributeSet>(CharacterMovementAttributeSet);
	CharacterAbilitySystemComponent->SetIsReplicated(true);
	CharacterAbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Full);

	// Setup Input Mapping Context
	static ConstructorHelpers::FObjectFinder<UInputMappingContext> IMC_Default(TEXT("/Game/Project_X/Input/PX_IMC_Default.PX_IMC_Default"));
	if (IMC_Default.Succeeded())
	{
		DefaultMappingContext = IMC_Default.Object;
	}

	/* InputConfigDataAsset로 대체
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

	// Setup Jump Input Action
	static ConstructorHelpers::FObjectFinder<UInputAction> IA_Jump(TEXT("/Game/Project_X/Input/Action/PX_IA_Jump.PX_IA_Jump"));
	if ( IA_Jump.Succeeded() )
	{
		JumpAction = IA_Jump.Object;
	}

	// Setup Walk Input Action
	static ConstructorHelpers::FObjectFinder<UInputAction> IA_Walk(TEXT("/Game/Project_X/Input/Action/PX_IA_Walk.PX_IA_Walk"));
	if ( IA_Walk.Succeeded() )
	{
		WalkAction = IA_Walk.Object;
	}

	// Setup Sprint Input Action
	static ConstructorHelpers::FObjectFinder<UInputAction> IA_Sprint(TEXT("/Game/Project_X/Input/Action/PX_IA_Sprint.PX_IA_Sprint"));
	if ( IA_Sprint.Succeeded() )
	{
		SprintAction = IA_Sprint.Object;
	}

	// Setup Crouch Input Action
	static ConstructorHelpers::FObjectFinder<UInputAction> IA_Crouch(TEXT("/Game/Project_X/Input/Action/PX_IA_Crouch.PX_IA_Crouch"));
	if ( IA_Crouch.Succeeded() )
	{
		CrouchAction = IA_Crouch.Object;
	}

	// Setup Interact Input Action
	static ConstructorHelpers::FObjectFinder<UInputAction> IA_Interact(TEXT("/Game/Project_X/Input/Action/PX_IA_Interact.PX_IA_Interact"));
	if ( IA_Interact.Succeeded() )
	{
		InteractAction = IA_Interact.Object;
	}

	// Setup Over the Shoulder Aim Input Action
	static ConstructorHelpers::FObjectFinder<UInputAction> IA_OTSAim(TEXT("/Game/Project_X/Input/Action/PX_IA_OTSAim.PX_IA_OTSAim"));
	if ( IA_OTSAim.Succeeded())
	{
		OTSAimAction = IA_OTSAim.Object;
	}

	// Setup ADS Input Action
	static ConstructorHelpers::FObjectFinder<UInputAction> IA_ADS(TEXT("/Game/Project_X/Input/Action/PX_IA_ADS.PX_IA_ADS"));
	if ( IA_ADS.Succeeded() )
	{
		ADSAction = IA_ADS.Object;
	}

	// Setup Draw Input Action
	static ConstructorHelpers::FObjectFinder<UInputAction> IA_Draw(TEXT("/Game/Project_X/Input/Action/PX_IA_Draw.PX_IA_Draw"));
	if ( IA_Draw.Succeeded() )
	{
		DrawAction = IA_Draw.Object;
	}

	// Setup Fire Input Action
	static ConstructorHelpers::FObjectFinder<UInputAction> IA_Fire(TEXT("/Game/Project_X/Input/Action/PX_IA_Fire.PX_IA_Fire"));
	if ( IA_Fire.Succeeded() )
	{
		FireAction = IA_Fire.Object;
	}

	// Setup Reload Input Action
	static ConstructorHelpers::FObjectFinder<UInputAction> IA_Reload(TEXT("/Game/Project_X/Input/Action/PX_IA_Reload.PX_IA_Reload"));
	if ( IA_Reload.Succeeded() )
	{
		ReloadAction = IA_Reload.Object;
	}

	// Setup Equip Slot Input Action
	static ConstructorHelpers::FObjectFinder<UInputAction> IA_EquipSlot(TEXT("/Game/Project_X/Input/Action/PX_IA_EquipSlot.PX_IA_EquipSlot"));
	if ( IA_EquipSlot.Succeeded() )
	{
		EquipSlotAction = IA_EquipSlot.Object;
	}

	// Setup Switch Fire Mode Input Action
	static ConstructorHelpers::FObjectFinder<UInputAction> IA_SwitchFireMode(TEXT("/Game/Project_X/Input/Action/PX_IA_SwitchFireMode.PX_IA_SwitchFireMode"));
	if ( IA_SwitchFireMode.Succeeded() )
	{
		SwitchFireModeAction = IA_SwitchFireMode.Object;
	}

	// Setup Toggle Inventory Input Action
	static ConstructorHelpers::FObjectFinder<UInputAction> IA_ToggleInventory(TEXT("/Game/Project_X/Input/Action/PX_IA_ToggleInventory.PX_IA_ToggleInventory"));
	if ( IA_ToggleInventory.Succeeded() )
	{
		ToggleInventoryAction = IA_ToggleInventory.Object;
	}
	*/

	// Setup Input Config Data Asset
	static ConstructorHelpers::FObjectFinder<UPX_InputConfigDataAsset> InputConfig(TEXT("/Game/Project_X/Input/DA_PX_InputConfig.DA_PX_InputConfig"));
	if ( InputConfig.Succeeded() )
	{
		InputConfigDataAsset = InputConfig.Object;
	}
	
	// Setup Equip Slot Input Action
	static ConstructorHelpers::FObjectFinder<UInputAction> IA_EquipSlot(TEXT("/Game/Project_X/Input/Action/PX_IA_EquipSlot.PX_IA_EquipSlot"));
	if ( IA_EquipSlot.Succeeded() )
	{
		EquipSlotAction = IA_EquipSlot.Object;
	}

	// Setup AnimInstance Class
	//static ConstructorHelpers::FClassFinder<UAnimInstance> AnimClass(TEXT("/Game/Project_X/Character/PX_Character/Animations/BP_PXCharacter.BP_PXCharacter_C"));
	static ConstructorHelpers::FClassFinder<UAnimInstance> AnimClass(TEXT("/Game/Project_X/Character/PX_Character/Animations/ABP_PXCharacter_Main.ABP_PXCharacter_Main_C"));
	if (AnimClass.Class)
	{
		AnimInstanceClass = AnimClass.Class;
		GetMesh()->SetAnimInstanceClass(AnimInstanceClass);
	}

	static ConstructorHelpers::FClassFinder<UAnimInstance> IdleAnimLayerClass(TEXT("/Game/Project_X/Character/PX_Character/Animations/ABP_PXCharacter_Idle.ABP_PXCharacter_Idle_C"));
	static ConstructorHelpers::FClassFinder<UAnimInstance> BowAnimLayerClass(TEXT("/Game/Project_X/Character/PX_Character/Animations/ABP_PXCharacter_Bow.ABP_PXCharacter_Bow_C"));
	static ConstructorHelpers::FClassFinder<UAnimInstance> PistolAnimLayerClass(TEXT("/Game/Project_X/Character/PX_Character/Animations/ABP_PXCharacter_Pistol.ABP_PXCharacter_Pistol_C"));
	static ConstructorHelpers::FClassFinder<UAnimInstance> RifleAnimLayerClass(TEXT("/Game/Project_X/Character/PX_Character/Animations/ABP_PXCharacter_Pistol.ABP_PXCharacter_Pistol_C"));
	static ConstructorHelpers::FClassFinder<UAnimInstance> ShotgunAnimLayerClass(TEXT("/Game/Project_X/Character/PX_Character/Animations/ABP_PXCharacter_Pistol.ABP_PXCharacter_Pistol_C"));
	if ( IdleAnimLayerClass.Class )
	{
		AnimInstanceLayerClass = IdleAnimLayerClass.Class;
	}

	// Setup Character Properties
	bUseControllerRotationYaw = false;

	// Setup Character Movement Component Properties
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;

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
	/*
	static ConstructorHelpers::FClassFinder<UUserWidget> CrosshairBPClass(TEXT("/Game/Project_X/CrossHair.CrossHair_C"));
	if (CrosshairBPClass.Succeeded())
	{
		CrosshairWidgetClass = CrosshairBPClass.Class;
	}
	*/
	/*
	static ConstructorHelpers::FClassFinder<UUserWidget> HudRootBPClass(TEXT("/Game/Project_X/UMG/HUD/WBP_HUD_Root.WBP_HUD_Root_C"));
	if ( HudRootBPClass.Succeeded() )
	{
		HudRootWidgetClass = HudRootBPClass.Class;
	}
	*/
#endif

	static ConstructorHelpers::FObjectFinder<UAnimMontage> Turn90(TEXT("/Game/Project_X/Character/PX_Character/Animations/Weapon/Bow/Montage/MTG_Turn_90.MTG_Turn_90"));
	if ( Turn90.Succeeded() )
	{
		Turn90Montage = Turn90.Object;
	}

	static ConstructorHelpers::FObjectFinder<UAnimMontage> Turn180(TEXT("/Game/Project_X/Character/PX_Character/Animations/Weapon/Bow/Montage/MTG_Turn_180.MTG_Turn_180"));
	if ( Turn180.Succeeded() )
	{
		Turn180Montage = Turn180.Object;
	}
}

// Called every frame
void APX_Character::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if ( HasAuthority() )
	{
		Server_Tick(DeltaTime);
	}

	if ( IsLocallyControlled() )
	{
		Client_Tick(DeltaTime);
	}

	ApplyLocomotionSpeedMode();
}

void APX_Character::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APX_Character, bIsJumping);
	DOREPLIFETIME(APX_Character, bIsCrouching);
	DOREPLIFETIME(APX_Character, bIsAiming);
	DOREPLIFETIME(APX_Character, bIsDrawing);
	DOREPLIFETIME(APX_Character, DrawProgress);
	DOREPLIFETIME_CONDITION(APX_Character, RemoteViewYaw, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(APX_Character, bHasMoveInput, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(APX_Character, LastMoveSpeed, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(APX_Character, LastMoveDirection, COND_SkipOwner);
	DOREPLIFETIME(APX_Character, bForceDemoAimOffset);
	DOREPLIFETIME(APX_Character, ForcedDemoAimYaw);
	DOREPLIFETIME(APX_Character, ForcedDemoAimPitch);
}

void APX_Character::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	APX_PlayerState* PX_PlayerState = GetPlayerState<APX_PlayerState>();
	if ( !PX_PlayerState ) return;

	UAbilitySystemComponent* ASC = PX_PlayerState->GetAbilitySystemComponent();
	if ( !ASC ) return;

	ASC->InitAbilityActorInfo(PX_PlayerState, this);
	PX_PlayerState->GrantDefaultAbilities();

	PX_LOG(Log, TEXT("Init AbilitySystemComponent's AbilityActorInfo"));
}

void APX_Character::MulticastPlayTurnInPlace_Implementation(bool bTurn180, bool bTurnRight)
{
	//UE_LOG(LogTemp, Log, TEXT("MulticastPlayTurnInPlace"));

	UAnimMontage* MontageToPlay = bTurn180 ? Turn180Montage : Turn90Montage;
	if ( !MontageToPlay ) return;
	if ( !CachedAnimInstance ) return;
	if ( CachedAnimInstance->Montage_IsPlaying(Turn90Montage) || CachedAnimInstance->Montage_IsPlaying(Turn180Montage) ) return;

	CachedAnimInstance->Montage_Play(MontageToPlay);
	CachedAnimInstance->Montage_JumpToSection(bTurnRight ? FName("Right") : FName("Left"), MontageToPlay);
}

UAbilitySystemComponent* APX_Character::GetAbilitySystemComponent() const
{
	if ( const APX_PlayerState* PX_PlayerState = GetPlayerState<APX_PlayerState>() )
	{
		return PX_PlayerState->GetAbilitySystemComponent();
	}

	return FindComponentByClass<UPX_DemoBotComponent>() ? CharacterAbilitySystemComponent : nullptr;
}

FGameplayTag APX_Character::ResolveAttackInputTag() const
{
	if ( !WeaponSystemComponent )
	{
		PX_LOG(Warning, TEXT("WeaponSystemComponent is null"));
		return PX_GameplayTags::Input_Combat_Attack;
	}

	UPX_WeaponItemInstance* WeaponItemInstance = WeaponSystemComponent->GetWeaponInstance();
	if ( !WeaponItemInstance )
	{
		PX_LOG(Warning, TEXT("WeaponItemInstance is null"));
		return PX_GameplayTags::Input_Combat_Attack;
	}

	UPX_EquippableItemDataAsset* EquippableItemDataAsset = WeaponItemInstance->GetEquippableItemDataAsset();
	if ( !EquippableItemDataAsset )
	{
		PX_LOG(Warning, TEXT("EquippableItemDataAsset is null"));
		return PX_GameplayTags::Input_Combat_Attack;
	}

	const FGameplayTagContainer& GrantedTags = EquippableItemDataAsset->GrantedTags;

	if ( GrantedTags.HasTag(PX_GameplayTags::Weapon_Type_Gun) )
	{
		return PX_GameplayTags::Input_Combat_Attack_Gun;
	}

	if ( GrantedTags.HasTag(PX_GameplayTags::Weapon_Type_Bow) )
	{
		return PX_GameplayTags::Input_Combat_Attack_Bow;
	}

	// 필요하면 추가
	// if ( GrantedTags.HasTag(PX_GameplayTags::Weapon_Type_Melee) )
	// {
	//     return PX_GameplayTags::Input_Combat_Attack_Melee;
	// }

	return PX_GameplayTags::Input_Combat_Attack;
}
