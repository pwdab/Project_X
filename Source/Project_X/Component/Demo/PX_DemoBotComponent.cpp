#include "Component/Demo/PX_DemoBotComponent.h"

#include "Component/Inventory/PX_InventoryComponent.h"
#include "Component/Inventory/PX_ItemDataAsset.h"
#include "Component/Inventory/PX_WeaponItemInstance.h"
#include "Entity/PX_Weapon.h"
#include "Component/Weapon/PX_WeaponDataAsset.h"
#include "Component/Weapon/PX_WeaponSystemComponent.h"
#include "Entity/PX_Character.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/Tags/PX_GameplayTags.h"
#include "Abilities/GameplayAbilityTargetTypes.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "EngineUtils.h"
#include "UObject/ConstructorHelpers.h"

UPX_DemoBotComponent::UPX_DemoBotComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;

	static ConstructorHelpers::FObjectFinder<UPX_ItemDataAsset> DefaultRifleData(TEXT("/Game/Project_X/Character/Weapon/Lyra/Rifle/Data/DA_PX_WeaponData_Rifle.DA_PX_WeaponData_Rifle"));
	if (DefaultRifleData.Succeeded())
	{
		EquipItemDataAsset = DefaultRifleData.Object;
		CombatItemDataAsset = DefaultRifleData.Object;
	}

	static ConstructorHelpers::FClassFinder<UAnimInstance> DefaultStrafeAnimClass(TEXT("/Game/Project_X/Character/PX_Character/Animations/ABP_PXCharacter_Idle.ABP_PXCharacter_Idle_C"));
	if (DefaultStrafeAnimClass.Class)
	{
		StrafeAnimInstanceClass = DefaultStrafeAnimClass.Class;
	}

	static ConstructorHelpers::FClassFinder<UAnimInstance> DefaultStrafeBaseAnimClass(TEXT("/Game/Project_X/Character/PX_Character/Animations/ABP_PXCharacter_Main.ABP_PXCharacter_Main_C"));
	if (DefaultStrafeBaseAnimClass.Class)
	{
		StrafeBaseAnimInstanceClass = DefaultStrafeBaseAnimClass.Class;
	}

	static ConstructorHelpers::FObjectFinder<UAnimMontage> DefaultBotWeaponFireMontage(TEXT("/Game/Project_X/Character/Weapon/Lyra/Rifle/Animations/AM_Weap_Rifle_Bot.AM_Weap_Rifle_Bot"));
	if (DefaultBotWeaponFireMontage.Succeeded())
	{
		BotWeaponFireMontage = DefaultBotWeaponFireMontage.Object;
	}

	static ConstructorHelpers::FObjectFinder<UAnimMontage> DefaultBotCharacterReloadMontage(TEXT("/Game/Project_X/Character/PX_Character/Animations/Lyra/Rifle/Montage/MM_Rifle_Reload_Bot.MM_Rifle_Reload_Bot"));
	if (DefaultBotCharacterReloadMontage.Succeeded())
	{
		BotCharacterReloadMontage = DefaultBotCharacterReloadMontage.Object;
	}

	static ConstructorHelpers::FObjectFinder<UAnimMontage> DefaultBotWeaponReloadMontage(TEXT("/Game/Project_X/Character/Weapon/Lyra/Rifle/Animations/AM_Weap_Rifle_Reload_Bot.AM_Weap_Rifle_Reload_Bot"));
	if (DefaultBotWeaponReloadMontage.Succeeded())
	{
		BotWeaponReloadMontage = DefaultBotWeaponReloadMontage.Object;
	}
}

void UPX_DemoBotComponent::BeginPlay()
{
	Super::BeginPlay();

	CacheOwner();

	if (OwnerCharacter)
	{
		InitialActorRotation = OwnerCharacter->GetActorRotation();
		if (UAbilitySystemComponent* ASC = OwnerCharacter->GetAbilitySystemComponent())
		{
			AbilitySystem = ASC;
			ASC->InitAbilityActorInfo(OwnerCharacter, OwnerCharacter);
		}
	}

	if (Behavior == EPXDemoBotBehavior::StrafeLoop)
	{
		ApplyStrafeAnimLayer();
	}

	BindStunTagEvent();

	if (bAutoStart && CanRunDemo())
	{
		if (StartDelay > 0.0f)
		{
			GetWorld()->GetTimerManager().SetTimer(StartTimerHandle, this, &UPX_DemoBotComponent::StartDemo, StartDelay, false);
		}
		else
		{
			StartDemo();
		}
	}
}

void UPX_DemoBotComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UnbindStunTagEvent();
	StopDemo();
	Super::EndPlay(EndPlayReason);
}

void UPX_DemoBotComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bRunning || !CanRunDemo())
	{
		return;
	}

	if (IsStunned())
	{
		return;
	}

	if (Behavior == EPXDemoBotBehavior::StrafeLoop)
	{
		TickStrafeLoop(DeltaTime);
	}
	else if (Behavior == EPXDemoBotBehavior::RifleJumpFiveShotLoop)
	{
		UpdateAimAtTarget();
	}
}

void UPX_DemoBotComponent::StartDemo()
{
	if (!CacheOwner() || !CanRunDemo())
	{
		return;
	}

	StopDemo();
	bRunning = true;
	bPausedByStun = false;

	if (!EnsureDemoInventory())
	{
		GetWorld()->GetTimerManager().SetTimer(StartTimerHandle, this, &UPX_DemoBotComponent::StartDemo, 0.2f, false);
		return;
	}

	switch (Behavior)
	{
	case EPXDemoBotBehavior::StrafeLoop:
		StartStrafeLoop();
		break;
	case EPXDemoBotBehavior::EquipLoop:
		StartEquipLoop();
		break;
	case EPXDemoBotBehavior::FireReloadLoop:
		StartFireReloadLoop();
		break;
	case EPXDemoBotBehavior::RifleJumpFiveShotLoop:
		if (OwnerCharacter)
		{
			EnsureCombatAnimLayer();
			OwnerCharacter->SetIsAiming(true);
		}
		StartRifleJumpFiveShotLoop();
		break;
	default:
		break;
	}
}

void UPX_DemoBotComponent::StopDemo()
{
	CancelActiveCombatAction();

	if (Behavior == EPXDemoBotBehavior::StrafeLoop)
	{
		StopStrafeLoop();
	}

	if (OwnerCharacter)
	{
		OwnerCharacter->SetDemoMoveInputState(false, 0.0f, EMoveDirection::None);
		OwnerCharacter->SetForceDemoAimOffset(false);
		OwnerCharacter->SetIsAiming(false);
	}

	ClearDemoTimers();
	bRunning = false;
}

bool UPX_DemoBotComponent::CacheOwner()
{
	if (!OwnerCharacter)
	{
		OwnerCharacter = Cast<APX_Character>(GetOwner());
	}

	if (!WeaponSystem && OwnerCharacter)
	{
		WeaponSystem = OwnerCharacter->GetWeaponSystemComponent();
	}

	if (!Inventory && OwnerCharacter)
	{
		Inventory = OwnerCharacter->GetInventoryComponent();
	}

	if (!AbilitySystem && OwnerCharacter)
	{
		AbilitySystem = OwnerCharacter->GetAbilitySystemComponent();
	}

	return OwnerCharacter != nullptr;
}

bool UPX_DemoBotComponent::CanRunDemo() const
{
	return OwnerCharacter && OwnerCharacter->HasAuthority() && GetWorld();
}

void UPX_DemoBotComponent::ClearDemoTimers()
{
	if (UWorld* World = GetWorld())
	{
		FTimerManager& TimerManager = World->GetTimerManager();
		TimerManager.ClearTimer(StartTimerHandle);
		TimerManager.ClearTimer(EquipLoopTimerHandle);
		TimerManager.ClearTimer(EquipEndTimerHandle);
		TimerManager.ClearTimer(FireLoopTimerHandle);
		TimerManager.ClearTimer(FireEndTimerHandle);
		TimerManager.ClearTimer(ReloadTimerHandle);
	}
}

bool UPX_DemoBotComponent::IsStunned() const
{
	return AbilitySystem && AbilitySystem->HasMatchingGameplayTag(PX_GameplayTags::State_Condition_Stunned);
}

void UPX_DemoBotComponent::BindStunTagEvent()
{
	if (!AbilitySystem || StunTagChangedHandle.IsValid())
	{
		return;
	}

	StunTagChangedHandle = AbilitySystem->RegisterGameplayTagEvent(PX_GameplayTags::State_Condition_Stunned, EGameplayTagEventType::NewOrRemoved)
		.AddUObject(this, &UPX_DemoBotComponent::HandleStunTagChanged);
}

void UPX_DemoBotComponent::UnbindStunTagEvent()
{
	if (!AbilitySystem || !StunTagChangedHandle.IsValid())
	{
		return;
	}

	AbilitySystem->RegisterGameplayTagEvent(PX_GameplayTags::State_Condition_Stunned, EGameplayTagEventType::NewOrRemoved)
		.Remove(StunTagChangedHandle);
	StunTagChangedHandle.Reset();
}

void UPX_DemoBotComponent::HandleStunTagChanged(const FGameplayTag ChangedTag, int32 NewCount)
{
	(void)ChangedTag;

	if (NewCount > 0)
	{
		bPausedByStun = bRunning;
		CancelActiveCombatAction();
		return;
	}

	if (bPausedByStun && bRunning && Behavior == EPXDemoBotBehavior::FireReloadLoop && CanRunDemo())
	{
		bPausedByStun = false;
		GetWorld()->GetTimerManager().SetTimer(FireLoopTimerHandle, this, &UPX_DemoBotComponent::StartFirePhase, PostReloadDelay, false);
	}
	else if (bPausedByStun && bRunning && Behavior == EPXDemoBotBehavior::RifleJumpFiveShotLoop && CanRunDemo())
	{
		bPausedByStun = false;
		GetWorld()->GetTimerManager().SetTimer(FireLoopTimerHandle, this, &UPX_DemoBotComponent::StartRifleJumpFiveShotLoop, PostJumpAttackDelay, false);
	}
	else
	{
		bPausedByStun = false;
	}
}

void UPX_DemoBotComponent::CancelActiveCombatAction()
{
	if (UWorld* World = GetWorld())
	{
		FTimerManager& TimerManager = World->GetTimerManager();
		TimerManager.ClearTimer(FireLoopTimerHandle);
		TimerManager.ClearTimer(FireEndTimerHandle);
		TimerManager.ClearTimer(ReloadTimerHandle);
	}

	if (WeaponSystem)
	{
		if (bFireOnceInProgress)
		{
			WeaponSystem->Authoritative_CancelFireOnce();
		}

		if (bReloadInProgress)
		{
			WeaponSystem->Authoritative_CancelReload();
		}
	}

	bFireOnceInProgress = false;
	bReloadInProgress = false;
	RemainingJumpAttackShots = 0;
	TargetCharacter = nullptr;

	if (OwnerCharacter)
	{
		OwnerCharacter->SetIsAiming(Behavior == EPXDemoBotBehavior::RifleJumpFiveShotLoop && bRunning);
		OwnerCharacter->SetLocomotionJumping(false);
		if (Behavior != EPXDemoBotBehavior::RifleJumpFiveShotLoop || !bRunning)
		{
			OwnerCharacter->SetForceDemoAimOffset(false);
		}
	}
}

void UPX_DemoBotComponent::ApplyStrafeAnimLayer()
{
	if (!OwnerCharacter || !StrafeAnimInstanceClass)
	{
		return;
	}

	USkeletalMeshComponent* MeshComp = OwnerCharacter->GetMesh();
	if (!MeshComp)
	{
		return;
	}

	if (StrafeBaseAnimInstanceClass && MeshComp->GetAnimClass() == StrafeAnimInstanceClass)
	{
		MeshComp->SetAnimInstanceClass(StrafeBaseAnimInstanceClass);
	}

	if (UAnimInstance* AnimInstance = MeshComp->GetAnimInstance())
	{
		AnimInstance->LinkAnimClassLayers(StrafeAnimInstanceClass);
	}
}

bool UPX_DemoBotComponent::EnsureDemoInventory()
{
	if (!Inventory)
	{
		return Behavior == EPXDemoBotBehavior::StrafeLoop;
	}

	if (!Inventory->IsInventoryReady())
	{
		return false;
	}

	if (Behavior == EPXDemoBotBehavior::EquipLoop && EquipItemDataAsset)
	{
		return Inventory->AddWeaponItemFromDataToSlot(EquipItemDataAsset, EquipSlotIndex);
	}

	if ((Behavior == EPXDemoBotBehavior::FireReloadLoop || Behavior == EPXDemoBotBehavior::RifleJumpFiveShotLoop) && CombatItemDataAsset)
	{
		return Inventory->AddWeaponItemFromDataToSlot(CombatItemDataAsset, CombatSlotIndex);
	}

	return true;
}

void UPX_DemoBotComponent::StartStrafeLoop()
{
	StrafeOrigin = OwnerCharacter->GetActorLocation();
	StrafeAxis = OwnerCharacter->GetActorRightVector().GetSafeNormal();
	StrafeDirection = 1;
	InitialActorRotation = OwnerCharacter->GetActorRotation();

	ApplyStrafeAnimLayer();

	if (bFaceInitialDirectionWhileStrafing)
	{
		OwnerCharacter->SetActorRotation(FRotator(0.0f, InitialActorRotation.Yaw, 0.0f));
		OwnerCharacter->SetDemoAimYaw(InitialActorRotation.Yaw);
	}

	if (UCharacterMovementComponent* MovementComp = OwnerCharacter->GetCharacterMovement())
	{
		if (!bStrafeMovementSettingsApplied)
		{
			bOriginalUseControllerRotationYaw = OwnerCharacter->bUseControllerRotationYaw;
			bOriginalOrientRotationToMovement = MovementComp->bOrientRotationToMovement;
			bOriginalUseControllerDesiredRotation = MovementComp->bUseControllerDesiredRotation;
			bStrafeMovementSettingsApplied = true;
		}

		OriginalMaxWalkSpeed = MovementComp->MaxWalkSpeed;
		MovementComp->MaxWalkSpeed = StrafeSpeed;
		OwnerCharacter->bUseControllerRotationYaw = false;
		MovementComp->bOrientRotationToMovement = false;
		MovementComp->bUseControllerDesiredRotation = false;
	}
}

void UPX_DemoBotComponent::TickStrafeLoop(float DeltaTime)
{
	if (!OwnerCharacter)
	{
		return;
	}

	const FVector Offset = OwnerCharacter->GetActorLocation() - StrafeOrigin;
	const float SignedDistance = FVector::DotProduct(Offset, StrafeAxis);

	if (SignedDistance >= StrafeDistance)
	{
		StrafeDirection = -1;
	}
	else if (SignedDistance <= -StrafeDistance)
	{
		StrafeDirection = 1;
	}

	if (bFaceInitialDirectionWhileStrafing)
	{
		OwnerCharacter->SetActorRotation(FRotator(0.0f, InitialActorRotation.Yaw, 0.0f));
		OwnerCharacter->SetDemoAimYaw(InitialActorRotation.Yaw);
	}

	OwnerCharacter->AddMovementInput(StrafeAxis, static_cast<float>(StrafeDirection), true);
	OwnerCharacter->SetDemoMoveInputState(true, StrafeSpeed, StrafeDirection > 0 ? EMoveDirection::Right : EMoveDirection::Left);
}

void UPX_DemoBotComponent::StopStrafeLoop()
{
	if (OwnerCharacter)
	{
		if (UCharacterMovementComponent* MovementComp = OwnerCharacter->GetCharacterMovement())
		{
			if (OriginalMaxWalkSpeed > 0.0f)
			{
				MovementComp->MaxWalkSpeed = OriginalMaxWalkSpeed;
			}
			MovementComp->StopMovementImmediately();
			if (bStrafeMovementSettingsApplied)
			{
				OwnerCharacter->bUseControllerRotationYaw = bOriginalUseControllerRotationYaw;
				MovementComp->bOrientRotationToMovement = bOriginalOrientRotationToMovement;
				MovementComp->bUseControllerDesiredRotation = bOriginalUseControllerDesiredRotation;
				bStrafeMovementSettingsApplied = false;
			}
		}
	}
}

void UPX_DemoBotComponent::StartEquipLoop()
{
	EquipSelectedSlot();
}

void UPX_DemoBotComponent::EquipSelectedSlot()
{
	if (!bRunning || !CanRunDemo())
	{
		return;
	}

	TryEquipSlot(EquipSlotIndex);
	GetWorld()->GetTimerManager().SetTimer(EquipLoopTimerHandle, this, &UPX_DemoBotComponent::EquipBareHands, EquipInterval, false);
}

void UPX_DemoBotComponent::EquipBareHands()
{
	if (!bRunning || !CanRunDemo())
	{
		return;
	}

	TryEquipSlot(BareHandSlotIndex);
	GetWorld()->GetTimerManager().SetTimer(EquipLoopTimerHandle, this, &UPX_DemoBotComponent::EquipSelectedSlot, UnequipInterval, false);
}

bool UPX_DemoBotComponent::TryEquipSlot(int32 SlotIndex)
{
	if (!WeaponSystem)
	{
		return false;
	}

	if (!WeaponSystem->Authoritative_EquipBySlot(SlotIndex))
	{
		return false;
	}

	WeaponSystem->Multicast_EquipBySlot(SlotIndex);

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(EquipEndTimerHandle);
		World->GetTimerManager().SetTimer(EquipEndTimerHandle, this, &UPX_DemoBotComponent::FinishEquipAction, EquipActionDuration, false);
	}

	return true;
}

void UPX_DemoBotComponent::FinishEquipAction()
{
	if (WeaponSystem)
	{
		WeaponSystem->Authoritative_EquipBySlotEnd();
	}
}

void UPX_DemoBotComponent::StartFireReloadLoop()
{
	EquipCombatSlot();
}

void UPX_DemoBotComponent::EquipCombatSlot()
{
	TryEquipSlot(CombatSlotIndex);

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(FireLoopTimerHandle, this, &UPX_DemoBotComponent::StartFirePhase, EquipActionDuration + 0.1f, false);
	}
}

void UPX_DemoBotComponent::StartFirePhase()
{
	if (!bRunning || bPausedByStun || IsStunned() || !CanRunDemo() || !WeaponSystem || !WeaponSystem->GetWeaponInstance())
	{
		return;
	}

	if (bFaceInitialDirectionWhileAttacking)
	{
		OwnerCharacter->SetActorRotation(FRotator(0.0f, InitialActorRotation.Yaw, 0.0f));
		OwnerCharacter->SetDemoAimYaw(InitialActorRotation.Yaw);
	}
	OwnerCharacter->SetIsAiming(true);
	FirePhaseEndTime = GetWorld()->GetTimeSeconds() + FireDuration;
	FireOnceForDemo();
}

void UPX_DemoBotComponent::FireOnceForDemo()
{
	if (!bRunning || bPausedByStun || IsStunned() || !CanRunDemo() || !WeaponSystem || !WeaponSystem->GetWeaponInstance())
	{
		return;
	}

	if (GetWorld()->GetTimeSeconds() >= FirePhaseEndTime)
	{
		StartReloadPhase();
		return;
	}

	if (!WeaponSystem->Authoritative_FireOnce(nullptr))
	{
		StartReloadPhase();
		return;
	}

	bFireOnceInProgress = true;
	WeaponSystem->Multicast_FireOnce();
	GetWorld()->GetTimerManager().SetTimer(FireEndTimerHandle, this, &UPX_DemoBotComponent::FinishFireOnceForDemo, FireInterval, false);
}

void UPX_DemoBotComponent::FinishFireOnceForDemo()
{
	if (!bRunning || bPausedByStun || IsStunned() || !CanRunDemo() || !WeaponSystem)
	{
		return;
	}

	WeaponSystem->Authoritative_FireOnceEnd();
	bFireOnceInProgress = false;

	if (GetWorld()->GetTimeSeconds() >= FirePhaseEndTime)
	{
		StartReloadPhase();
	}
	else
	{
		GetWorld()->GetTimerManager().SetTimer(FireLoopTimerHandle, this, &UPX_DemoBotComponent::FireOnceForDemo, KINDA_SMALL_NUMBER, false);
	}
}

void UPX_DemoBotComponent::StartReloadPhase()
{
	if (!bRunning || bPausedByStun || IsStunned() || !CanRunDemo() || !WeaponSystem || !WeaponSystem->GetWeaponInstance())
	{
		return;
	}

	OwnerCharacter->SetIsAiming(true);

	if (WeaponSystem->Authoritative_Reload())
	{
		bReloadInProgress = true;
		WeaponSystem->Multicast_Reload();
		GetWorld()->GetTimerManager().SetTimer(ReloadTimerHandle, this, &UPX_DemoBotComponent::FinishReloadPhase, ReloadDuration, false);
	}
	else
	{
		GetWorld()->GetTimerManager().SetTimer(ReloadTimerHandle, this, &UPX_DemoBotComponent::StartFirePhase, PostReloadDelay, false);
	}
}

void UPX_DemoBotComponent::FinishReloadPhase()
{
	if (!bRunning || bPausedByStun || IsStunned() || !CanRunDemo() || !WeaponSystem)
	{
		return;
	}

	WeaponSystem->Authoritative_ReloadEnd();
	bReloadInProgress = false;
	GetWorld()->GetTimerManager().SetTimer(FireLoopTimerHandle, this, &UPX_DemoBotComponent::StartFirePhase, PostReloadDelay, false);
}

void UPX_DemoBotComponent::StartRifleJumpFiveShotLoop()
{
	if (!bRunning || bPausedByStun || IsStunned() || !CanRunDemo())
	{
		return;
	}

	EnsureCombatAnimLayer();
	TryEquipSlot(CombatSlotIndex);

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(ReloadTimerHandle, this, &UPX_DemoBotComponent::StartRifleJumpReloadPhase, EquipActionDuration + 0.1f, false);
	}
}

void UPX_DemoBotComponent::StartRifleJumpReloadPhase()
{
	if (!bRunning || bPausedByStun || IsStunned() || !CanRunDemo() || !WeaponSystem || !WeaponSystem->GetWeaponInstance())
	{
		return;
	}

	OwnerCharacter->SetIsAiming(true);

	if (WeaponSystem->Authoritative_Reload())
	{
		bReloadInProgress = true;
		WeaponSystem->Multicast_Reload();
		GetWorld()->GetTimerManager().SetTimer(ReloadTimerHandle, this, &UPX_DemoBotComponent::FinishRifleJumpReloadPhase, ReloadDuration, false);
	}
	else
	{
		GetWorld()->GetTimerManager().SetTimer(ReloadTimerHandle, this, &UPX_DemoBotComponent::StartRifleJumpAttackPhase, PostReloadDelay, false);
	}
}

void UPX_DemoBotComponent::FinishRifleJumpReloadPhase()
{
	if (!bRunning || bPausedByStun || IsStunned() || !CanRunDemo() || !WeaponSystem)
	{
		return;
	}

	WeaponSystem->Authoritative_ReloadEnd();
	bReloadInProgress = false;
	StartRifleJumpAttackPhase();
}

void UPX_DemoBotComponent::StartRifleJumpAttackPhase()
{
	if (!bRunning || bPausedByStun || IsStunned() || !CanRunDemo() || !WeaponSystem || !WeaponSystem->GetWeaponInstance())
	{
		return;
	}

	TargetCharacter = FindNearestTargetCharacter();
	if (!UpdateAimAtTarget() && bFaceInitialDirectionWhileAttacking)
	{
		OwnerCharacter->SetActorRotation(FRotator(0.0f, InitialActorRotation.Yaw, 0.0f));
		OwnerCharacter->SetDemoAimRotation(FRotator(0.0f, InitialActorRotation.Yaw, 0.0f));
	}

	OwnerCharacter->SetIsAiming(true);
	if (UCharacterMovementComponent* MovementComp = OwnerCharacter->GetCharacterMovement())
	{
		MovementComp->JumpZVelocity = JumpAttackJumpZVelocity;
	}
	OwnerCharacter->SetLocomotionJumping(true);
	RemainingJumpAttackShots = FMath::Max(1, JumpAttackShotCount);
	FireRifleJumpShot();
}

void UPX_DemoBotComponent::FireRifleJumpShot()
{
	if (!bRunning || bPausedByStun || IsStunned() || !CanRunDemo() || !WeaponSystem || !WeaponSystem->GetWeaponInstance())
	{
		return;
	}

	if (RemainingJumpAttackShots <= 0)
	{
		FinishRifleJumpAttackPhase();
		return;
	}

	UpdateAimAtTarget();

	FGameplayAbilityTargetDataHandle FireTargetData;
	const FGameplayAbilityTargetDataHandle* FireTargetDataPtr = MakeTargetedFireData(FireTargetData) ? &FireTargetData : nullptr;
	if (!WeaponSystem->Authoritative_FireOnce(FireTargetDataPtr))
	{
		FinishRifleJumpAttackPhase();
		return;
	}

	--RemainingJumpAttackShots;
	bFireOnceInProgress = true;
	WeaponSystem->Multicast_FireOnce();
	GetWorld()->GetTimerManager().SetTimer(FireEndTimerHandle, this, &UPX_DemoBotComponent::FinishRifleJumpShot, JumpAttackFireInterval, false);
}

void UPX_DemoBotComponent::FinishRifleJumpShot()
{
	if (!bRunning || bPausedByStun || IsStunned() || !CanRunDemo() || !WeaponSystem)
	{
		return;
	}

	WeaponSystem->Authoritative_FireOnceEnd();
	bFireOnceInProgress = false;

	if (RemainingJumpAttackShots <= 0)
	{
		FinishRifleJumpAttackPhase();
	}
	else
	{
		GetWorld()->GetTimerManager().SetTimer(FireLoopTimerHandle, this, &UPX_DemoBotComponent::FireRifleJumpShot, KINDA_SMALL_NUMBER, false);
	}
}

void UPX_DemoBotComponent::FinishRifleJumpAttackPhase()
{
	if (OwnerCharacter)
	{
		OwnerCharacter->SetLocomotionJumping(false);
		OwnerCharacter->SetIsAiming(true);
	}

	RemainingJumpAttackShots = 0;
	TargetCharacter = nullptr;

	if (bRunning && !bPausedByStun && !IsStunned() && CanRunDemo())
	{
		GetWorld()->GetTimerManager().SetTimer(FireLoopTimerHandle, this, &UPX_DemoBotComponent::StartRifleJumpFiveShotLoop, PostJumpAttackDelay, false);
	}
}

APX_Character* UPX_DemoBotComponent::FindNearestTargetCharacter() const
{
	if (!OwnerCharacter || !GetWorld())
	{
		return nullptr;
	}

	APX_Character* BestTarget = nullptr;
	float BestDistanceSq = FMath::Square(TargetSearchRadius);
	const FVector OwnerLocation = OwnerCharacter->GetActorLocation();

	for (TActorIterator<APX_Character> It(GetWorld()); It; ++It)
	{
		APX_Character* Candidate = *It;
		if (!Candidate || Candidate == OwnerCharacter || Candidate->IsPendingKillPending())
		{
			continue;
		}

		const float DistanceSq = FVector::DistSquared(OwnerLocation, Candidate->GetActorLocation());
		if (DistanceSq < BestDistanceSq)
		{
			BestDistanceSq = DistanceSq;
			BestTarget = Candidate;
		}
	}

	return BestTarget;
}

bool UPX_DemoBotComponent::UpdateAimAtTarget()
{
	if (!OwnerCharacter)
	{
		return false;
	}

	if (TargetCharacter && FVector::DistSquared(OwnerCharacter->GetActorLocation(), TargetCharacter->GetActorLocation()) > FMath::Square(TargetSearchRadius))
	{
		TargetCharacter = nullptr;
	}

	if (!TargetCharacter || TargetCharacter->IsPendingKillPending())
	{
		TargetCharacter = FindNearestTargetCharacter();
	}

	if (!TargetCharacter)
	{
		return false;
	}

	const FVector AimOrigin = OwnerCharacter->GetActorLocation() + FVector(0.0f, 0.0f, OwnerCharacter->BaseEyeHeight);
	const FVector AimDirection = (GetTargetAimPoint(TargetCharacter) - AimOrigin).GetSafeNormal();
	if (AimDirection.IsNearlyZero())
	{
		return false;
	}

	const FRotator AimRotation = AimDirection.Rotation();
	EnsureCombatAnimLayer();
	OwnerCharacter->SetActorRotation(FRotator(0.0f, AimRotation.Yaw, 0.0f));
	OwnerCharacter->SetDemoAimRotation(AimRotation);
	return true;
}

bool UPX_DemoBotComponent::MakeTargetedFireData(FGameplayAbilityTargetDataHandle& OutTargetData) const
{
	if (!OwnerCharacter || !WeaponSystem || !TargetCharacter)
	{
		return false;
	}

	APX_Weapon* CurrentWeapon = WeaponSystem->GetWeapon();
	USkeletalMeshComponent* WeaponMesh = CurrentWeapon ? CurrentWeapon->GetMesh() : nullptr;
	if (!CurrentWeapon || !WeaponMesh)
	{
		return false;
	}

	static const FName MuzzleSocketName(TEXT("Muzzle"));
	const FTransform MuzzleTransform = WeaponMesh->DoesSocketExist(MuzzleSocketName)
		? WeaponMesh->GetSocketTransform(MuzzleSocketName, RTS_World)
		: CurrentWeapon->GetActorTransform();
	const FVector MuzzleLocation = MuzzleTransform.GetLocation();
	const FVector TargetPoint = GetTargetAimPoint(TargetCharacter);
	FVector MuzzleAimDirection = (TargetPoint - MuzzleLocation).GetSafeNormal();
	if (MuzzleAimDirection.IsNearlyZero())
	{
		MuzzleAimDirection = OwnerCharacter->GetActorForwardVector();
	}

	const FVector CameraLocation = OwnerCharacter->GetActorLocation() + FVector(0.0f, 0.0f, OwnerCharacter->BaseEyeHeight);
	FVector CameraAimDirection = (TargetPoint - CameraLocation).GetSafeNormal();
	if (CameraAimDirection.IsNearlyZero())
	{
		CameraAimDirection = MuzzleAimDirection;
	}

	FGameplayAbilityTargetData_LocationInfo* AimTargetData = new FGameplayAbilityTargetData_LocationInfo();
	AimTargetData->SourceLocation.LocationType = EGameplayAbilityTargetingLocationType::LiteralTransform;
	AimTargetData->SourceLocation.LiteralTransform = FTransform(MuzzleAimDirection.Rotation(), MuzzleLocation);
	AimTargetData->TargetLocation.LocationType = EGameplayAbilityTargetingLocationType::LiteralTransform;
	AimTargetData->TargetLocation.LiteralTransform = FTransform(MuzzleAimDirection.Rotation(), TargetPoint);
	OutTargetData.Add(AimTargetData);

	FGameplayAbilityTargetData_LocationInfo* CameraTargetData = new FGameplayAbilityTargetData_LocationInfo();
	CameraTargetData->SourceLocation.LocationType = EGameplayAbilityTargetingLocationType::LiteralTransform;
	CameraTargetData->SourceLocation.LiteralTransform = FTransform(CameraAimDirection.Rotation(), CameraLocation);
	CameraTargetData->TargetLocation.LocationType = EGameplayAbilityTargetingLocationType::LiteralTransform;
	CameraTargetData->TargetLocation.LiteralTransform = FTransform(CameraAimDirection.Rotation(), TargetPoint);
	OutTargetData.Add(CameraTargetData);

	return true;
}

FVector UPX_DemoBotComponent::GetTargetAimPoint(const APX_Character* InTargetCharacter) const
{
	return InTargetCharacter ? InTargetCharacter->GetActorLocation() + TargetAimOffset : FVector::ZeroVector;
}

void UPX_DemoBotComponent::EnsureCombatAnimLayer()
{
	if (!OwnerCharacter)
	{
		OwnerCharacter = Cast<APX_Character>(GetOwner());
	}

	if (!OwnerCharacter || !CombatItemDataAsset)
	{
		return;
	}

	const UPX_WeaponDataAsset* WeaponDataAsset = Cast<UPX_WeaponDataAsset>(CombatItemDataAsset);
	if (!WeaponDataAsset || !WeaponDataAsset->CharacterAnimClass)
	{
		return;
	}

	if (!OwnerCharacter->GetLayerAnimInstance() || EnsuredCombatAnimLayerClass != WeaponDataAsset->CharacterAnimClass)
	{
		OwnerCharacter->SetLayerAnimInstanceByClass(WeaponDataAsset->CharacterAnimClass);
		EnsuredCombatAnimLayerClass = WeaponDataAsset->CharacterAnimClass;
	}
}
