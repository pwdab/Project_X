// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/PX_GA_Locomotion.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystem/Attributes/PX_MovementAttributeSet.h"
#include "AbilitySystem/Attributes/PX_ResourceAttributeSet.h"
#include "AbilitySystem/Tags/PX_GameplayTags.h"
#include "Entity/PX_Character.h"
#include "Framework/HUD/PX_HUD.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Settings/PX_GameplayPredictionSettings.h"
#include "TimerManager.h"
#include "UI/HUD/PX_HUDWidget.h"

UPX_GA_LocomotionBase::UPX_GA_LocomotionBase()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}

bool UPX_GA_LocomotionBase::ShouldUseClientPrediction(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo* ActivationInfo) const
{
	if ( !ActorInfo || !ActorInfo->IsLocallyControlled() )
	{
		return false;
	}

	if ( ActivationInfo && HasAuthority(ActivationInfo) )
	{
		return false;
	}

	if ( ActorInfo->AvatarActor.IsValid() && ActorInfo->AvatarActor->HasAuthority() )
	{
		return false;
	}

	return true;
}

bool UPX_GA_LocomotionBase::HasEnoughStamina(const FGameplayAbilityActorInfo* ActorInfo, float Cost) const
{
	if ( Cost <= 0.0f )
	{
		return true;
	}

	const UAbilitySystemComponent* ASC = GetAbilitySystemComponent(ActorInfo);
	if ( !ASC )
	{
		return false;
	}

	return ASC->GetNumericAttribute(UPX_ResourceAttributeSet::GetStaminaAttribute()) >= Cost;
}

bool UPX_GA_LocomotionBase::ApplyStaminaCost(const FGameplayAbilityActorInfo* ActorInfo, float Cost) const
{
	if ( Cost <= 0.0f )
	{
		return true;
	}

	UAbilitySystemComponent* ASC = GetAbilitySystemComponent(ActorInfo);
	if ( !ASC || !HasEnoughStamina(ActorInfo, Cost) )
	{
		return false;
	}

	if ( ActorInfo && ActorInfo->AvatarActor.IsValid() && ActorInfo->AvatarActor->HasAuthority() )
	{
		if ( const UPX_ResourceAttributeSet* ResourceSet = ASC->GetSet<UPX_ResourceAttributeSet>() )
		{
			ResourceSet->NotifyStaminaConsumed();
		}

		ASC->ApplyModToAttribute(UPX_ResourceAttributeSet::GetStaminaAttribute(), EGameplayModOp::Additive, -Cost);
	}
	else
	{
		ApplyPredictedStaminaCostToHUD(ActorInfo, Cost);
	}

	return true;
}

bool UPX_GA_LocomotionBase::ApplyStaminaCostAllowPartial(const FGameplayAbilityActorInfo* ActorInfo, float Cost) const
{
	if ( Cost <= 0.0f )
	{
		return true;
	}

	UAbilitySystemComponent* ASC = GetAbilitySystemComponent(ActorInfo);
	if ( !ASC )
	{
		return false;
	}

	if ( ActorInfo && ActorInfo->AvatarActor.IsValid() && ActorInfo->AvatarActor->HasAuthority() )
	{
		const float CurrentStamina = ASC->GetNumericAttribute(UPX_ResourceAttributeSet::GetStaminaAttribute());
		const float ActualCost = FMath::Min(CurrentStamina, Cost);
		if ( ActualCost > 0.0f )
		{
			if ( const UPX_ResourceAttributeSet* ResourceSet = ASC->GetSet<UPX_ResourceAttributeSet>() )
			{
				ResourceSet->NotifyStaminaConsumed();
			}

			ASC->ApplyModToAttribute(UPX_ResourceAttributeSet::GetStaminaAttribute(), EGameplayModOp::Additive, -ActualCost);
		}
	}
	else
	{
		ApplyPredictedStaminaCostToHUD(ActorInfo, Cost);
	}

	return true;
}

void UPX_GA_LocomotionBase::ApplyPredictedStaminaCostToHUD(const FGameplayAbilityActorInfo* ActorInfo, float Cost) const
{
	if ( !UPX_GameplayPredictionSettings::IsClientPredictedStaminaUIEnabled() || !bUseClientPredictedStaminaUI || Cost <= 0.0f || !ActorInfo || !ShouldUseClientPrediction(ActorInfo) )
	{
		return;
	}

	AActor* AvatarActor = ActorInfo->AvatarActor.Get();
	if ( AvatarActor && AvatarActor->HasAuthority() )
	{
		return;
	}

	APlayerController* PlayerController = ActorInfo->PlayerController.Get();
	if ( !PlayerController )
	{
		if ( const APawn* Pawn = Cast<APawn>(AvatarActor) )
		{
			PlayerController = Cast<APlayerController>(Pawn->GetController());
		}
	}

	APX_HUD* PX_HUD = PlayerController ? Cast<APX_HUD>(PlayerController->GetHUD()) : nullptr;
	UPX_HUDWidget* HUDWidget = PX_HUD ? PX_HUD->GetHUDWidget() : nullptr;
	if ( HUDWidget )
	{
		HUDWidget->ApplyPredictedStaminaCost(Cost);
	}
}

void UPX_GA_LocomotionBase::RefreshLocomotionSpeed(const FGameplayAbilityActorInfo* ActorInfo) const
{
	if ( APX_Character* PX_Character = GetCharacterChecked(ActorInfo) )
	{
		PX_Character->ApplyLocomotionSpeedMode();
	}
}

APX_Character* UPX_GA_LocomotionBase::GetCharacterChecked(const FGameplayAbilityActorInfo* ActorInfo) const
{
	return GetPXCharacter(ActorInfo);
}

UCharacterMovementComponent* UPX_GA_LocomotionBase::GetMovementComponent(const FGameplayAbilityActorInfo* ActorInfo) const
{
	const APX_Character* PX_Character = GetCharacterChecked(ActorInfo);
	if ( !PX_Character )
	{
		return nullptr;
	}

	return PX_Character->GetCharacterMovement();
}

UPX_GA_Jump::UPX_GA_Jump()
{
	InputTag = PX_GameplayTags::Input_Locomotion_Jump;

	AbilityTags.AddTag(PX_GameplayTags::Ability_Movement_Jump);
	ActivationOwnedTags.AddTag(PX_GameplayTags::State_Locomotion_Jumping);

	ActivationBlockedTags.AddTag(PX_GameplayTags::State_Locomotion_Jumping);
	ActivationBlockedTags.AddTag(PX_GameplayTags::State_Locomotion_Crouching);
	ActivationBlockedTags.AddTag(PX_GameplayTags::State_Condition_Dead);
	ActivationBlockedTags.AddTag(PX_GameplayTags::State_Condition_Down);
	ActivationBlockedTags.AddTag(PX_GameplayTags::State_Condition_Disabled);
	ActivationBlockedTags.AddTag(PX_GameplayTags::State_Condition_Stunned);
}

bool UPX_GA_Jump::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if ( !Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags) ) return false;

	const UCharacterMovementComponent* MovementComp = GetMovementComponent(ActorInfo);
	if ( !MovementComp || MovementComp->IsFalling() )
	{
		return false;
	}

	return true;
}

void UPX_GA_Jump::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if ( !HasAuthority(&ActivationInfo) && ActorInfo && ActorInfo->IsLocallyControlled() && !ShouldUseClientPrediction(ActorInfo, &ActivationInfo) )
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
		return;
	}

	if ( !CommitAbility(Handle, ActorInfo, ActivationInfo) || !ApplyStaminaCostAllowPartial(ActorInfo, JumpStaminaCost) )
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	APX_Character* PX_Character = GetCharacterChecked(ActorInfo);
	UCharacterMovementComponent* MovementComp = GetMovementComponent(ActorInfo);
	UAbilitySystemComponent* ASC = GetAbilitySystemComponent(ActorInfo);
	if ( !PX_Character || !MovementComp || !ASC )
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	MovementComp->JumpZVelocity = ASC->GetNumericAttribute(UPX_MovementAttributeSet::GetJumpZVelocityAttribute());
	PX_Character->SetLocomotionJumping(true);
}

void UPX_GA_Jump::InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

void UPX_GA_Jump::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if ( APX_Character* PX_Character = GetCharacterChecked(ActorInfo) )
	{
		PX_Character->SetLocomotionJumping(false);
	}

	if ( !IsActive() )
	{
		return;
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

UPX_GA_Walk::UPX_GA_Walk()
{
	InputTag = PX_GameplayTags::Input_Locomotion_Walk;

	AbilityTags.AddTag(PX_GameplayTags::Ability_Movement_Walk);
	ActivationOwnedTags.AddTag(PX_GameplayTags::State_Locomotion_Walking);

	ActivationBlockedTags.AddTag(PX_GameplayTags::State_Locomotion_Walking);
	ActivationBlockedTags.AddTag(PX_GameplayTags::State_Locomotion_Sprinting);
	ActivationBlockedTags.AddTag(PX_GameplayTags::State_Locomotion_Crouching);
	ActivationBlockedTags.AddTag(PX_GameplayTags::State_Condition_Dead);
	ActivationBlockedTags.AddTag(PX_GameplayTags::State_Condition_Down);
	ActivationBlockedTags.AddTag(PX_GameplayTags::State_Condition_Disabled);
	ActivationBlockedTags.AddTag(PX_GameplayTags::State_Condition_Stunned);
}

void UPX_GA_Walk::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if ( !HasAuthority(&ActivationInfo) && ActorInfo && ActorInfo->IsLocallyControlled() && !ShouldUseClientPrediction(ActorInfo, &ActivationInfo) )
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
		return;
	}

	if ( !CommitAbility(Handle, ActorInfo, ActivationInfo) )
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	RefreshLocomotionSpeed(ActorInfo);
}

void UPX_GA_Walk::InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

void UPX_GA_Walk::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if ( !IsActive() )
	{
		return;
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
	RefreshLocomotionSpeed(ActorInfo);
}

UPX_GA_Sprint::UPX_GA_Sprint()
{
	InputTag = PX_GameplayTags::Input_Locomotion_Sprint;

	AbilityTags.AddTag(PX_GameplayTags::Ability_Movement_Sprint);
	ActivationOwnedTags.AddTag(PX_GameplayTags::State_Locomotion_Sprinting);

	ActivationBlockedTags.AddTag(PX_GameplayTags::State_Locomotion_Sprinting);
	ActivationBlockedTags.AddTag(PX_GameplayTags::State_Locomotion_Walking);
	ActivationBlockedTags.AddTag(PX_GameplayTags::State_Locomotion_Crouching);
	ActivationBlockedTags.AddTag(PX_GameplayTags::State_Condition_Dead);
	ActivationBlockedTags.AddTag(PX_GameplayTags::State_Condition_Down);
	ActivationBlockedTags.AddTag(PX_GameplayTags::State_Condition_Disabled);
	ActivationBlockedTags.AddTag(PX_GameplayTags::State_Condition_Stunned);
}

bool UPX_GA_Sprint::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if ( !Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags) ) return false;

	const UCharacterMovementComponent* MovementComp = GetMovementComponent(ActorInfo);
	if ( !MovementComp || MovementComp->IsFalling() )
	{
		return false;
	}

	return HasEnoughStamina(ActorInfo, KINDA_SMALL_NUMBER);
}

void UPX_GA_Sprint::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if ( !HasAuthority(&ActivationInfo) && ActorInfo && ActorInfo->IsLocallyControlled() && !ShouldUseClientPrediction(ActorInfo, &ActivationInfo) )
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
		return;
	}

	if ( !CommitAbility(Handle, ActorInfo, ActivationInfo) )
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	RefreshLocomotionSpeed(ActorInfo);

	const bool bShouldDrainStamina = HasAuthority(&ActivationInfo) || ShouldUseClientPrediction(ActorInfo, &ActivationInfo);
	if ( bShouldDrainStamina )
	{
		if ( UWorld* World = GetWorld() )
		{
			LastSprintStaminaDrainTime = World->GetTimeSeconds();
			World->GetTimerManager().SetTimer(SprintStaminaTimerHandle, this, &UPX_GA_Sprint::DrainSprintStamina, SprintStaminaTickInterval, true, SprintStaminaTickInterval);
		}
	}
}

void UPX_GA_Sprint::InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

void UPX_GA_Sprint::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if ( UWorld* World = GetWorld() )
	{
		World->GetTimerManager().ClearTimer(SprintStaminaTimerHandle);
	}
	LastSprintStaminaDrainTime = 0.0;

	if ( !IsActive() )
	{
		return;
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
	RefreshLocomotionSpeed(ActorInfo);
}

void UPX_GA_Sprint::DrainSprintStamina()
{
	UWorld* World = GetWorld();
	if ( !World )
	{
		return;
	}

	const double CurrentTime = World->GetTimeSeconds();
	if ( LastSprintStaminaDrainTime <= 0.0 )
	{
		LastSprintStaminaDrainTime = CurrentTime;
		return;
	}

	const float DeltaSeconds = FMath::Max(static_cast<float>(CurrentTime - LastSprintStaminaDrainTime), 0.0f);
	LastSprintStaminaDrainTime = CurrentTime;

	const APX_Character* PX_Character = GetCharacterChecked(CurrentActorInfo);
	if ( !PX_Character || !PX_Character->HasMoveInput() )
	{
		return;
	}

	const UCharacterMovementComponent* MovementComp = GetMovementComponent(CurrentActorInfo);
	if ( !MovementComp || MovementComp->IsFalling() )
	{
		return;
	}

	const float StaminaCost = SprintStaminaCostPerSecond * DeltaSeconds;
	if ( StaminaCost <= 0.0f )
	{
		return;
	}

	UAbilitySystemComponent* ASC = GetAbilitySystemComponent(CurrentActorInfo);
	if ( !ASC )
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	const bool bHasAuthority = PX_Character->HasAuthority();
	const float CurrentStamina = ASC->GetNumericAttribute(UPX_ResourceAttributeSet::GetStaminaAttribute());
	if ( bHasAuthority && CurrentStamina <= 0.0f )
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	if ( !bHasAuthority )
	{
		if ( ShouldUseClientPrediction(CurrentActorInfo, &CurrentActivationInfo) )
		{
			ApplyPredictedStaminaCostToHUD(CurrentActorInfo, StaminaCost);
		}
		return;
	}

	const float ActualCost = FMath::Min(CurrentStamina, StaminaCost);
	if ( const UPX_ResourceAttributeSet* ResourceSet = ASC->GetSet<UPX_ResourceAttributeSet>() )
	{
		ResourceSet->NotifyStaminaConsumed();
	}
	ASC->ApplyModToAttribute(UPX_ResourceAttributeSet::GetStaminaAttribute(), EGameplayModOp::Additive, -ActualCost);
	if ( CurrentActorInfo )
	{
		if ( AActor* OwnerActor = CurrentActorInfo->OwnerActor.Get() )
		{
			OwnerActor->ForceNetUpdate();
		}
		if ( AActor* AvatarActor = CurrentActorInfo->AvatarActor.Get() )
		{
			AvatarActor->ForceNetUpdate();
		}
	}

	if ( CurrentStamina <= StaminaCost + KINDA_SMALL_NUMBER )
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
	}
}

UPX_GA_Crouch::UPX_GA_Crouch()
{
	InputTag = PX_GameplayTags::Input_Locomotion_Crouch;

	AbilityTags.AddTag(PX_GameplayTags::Ability_Movement_Crouch);
	ActivationOwnedTags.AddTag(PX_GameplayTags::State_Locomotion_Crouching);

	ActivationBlockedTags.AddTag(PX_GameplayTags::State_Locomotion_Crouching);
	ActivationBlockedTags.AddTag(PX_GameplayTags::State_Locomotion_Sprinting);
	ActivationBlockedTags.AddTag(PX_GameplayTags::State_Locomotion_Jumping);
	ActivationBlockedTags.AddTag(PX_GameplayTags::State_Condition_Dead);
	ActivationBlockedTags.AddTag(PX_GameplayTags::State_Condition_Down);
	ActivationBlockedTags.AddTag(PX_GameplayTags::State_Condition_Disabled);
	ActivationBlockedTags.AddTag(PX_GameplayTags::State_Condition_Stunned);
}

bool UPX_GA_Crouch::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if ( !Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags) ) return false;

	const UCharacterMovementComponent* MovementComp = GetMovementComponent(ActorInfo);
	return MovementComp && !MovementComp->IsFalling();
}

void UPX_GA_Crouch::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if ( !HasAuthority(&ActivationInfo) && ActorInfo && ActorInfo->IsLocallyControlled() && !ShouldUseClientPrediction(ActorInfo, &ActivationInfo) )
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
		return;
	}

	if ( !CommitAbility(Handle, ActorInfo, ActivationInfo) )
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if ( APX_Character* PX_Character = GetCharacterChecked(ActorInfo) )
	{
		PX_Character->SetLocomotionCrouching(true);
	}
}

void UPX_GA_Crouch::InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

void UPX_GA_Crouch::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if ( APX_Character* PX_Character = GetCharacterChecked(ActorInfo) )
	{
		PX_Character->SetLocomotionCrouching(false);
	}

	if ( !IsActive() )
	{
		return;
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
	RefreshLocomotionSpeed(ActorInfo);
}
