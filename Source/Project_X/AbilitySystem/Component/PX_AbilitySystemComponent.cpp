// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Component/PX_AbilitySystemComponent.h"

#include "AbilitySystem/Abilities/PX_GameplayAbilityBase.h"
#include "AbilitySystem/Abilities/PX_GameplayAbility_GunFire.h"
#include "AbilitySystem/Abilities/PX_GA_WeaponStatusImbue.h"
#include "AbilitySystem/Tags/PX_GameplayTags.h"
#include "Entity/PX_Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "TimerManager.h"

namespace
{
	constexpr float PX_ServerFallbackActivationDelay = 0.05f;
}

void UPX_AbilitySystemComponent::BeginPlay()
{
	Super::BeginPlay();

	RegisterGameplayTagEvent(PX_GameplayTags::State_Condition_Stunned, EGameplayTagEventType::NewOrRemoved).AddUObject(this, &UPX_AbilitySystemComponent::HandleStunTagChanged);
	OnActiveGameplayEffectAddedDelegateToSelf.AddUObject(this, &UPX_AbilitySystemComponent::HandleGameplayEffectAppliedToSelf);
}

void UPX_AbilitySystemComponent::OnGiveAbility(FGameplayAbilitySpec& AbilitySpec)
{
	Super::OnGiveAbility(AbilitySpec);

	OnAbilityListChanged.Broadcast();
}

void UPX_AbilitySystemComponent::OnRemoveAbility(FGameplayAbilitySpec& AbilitySpec)
{
	Super::OnRemoveAbility(AbilitySpec);

	OnAbilityListChanged.Broadcast();
}

void UPX_AbilitySystemComponent::OnRep_ActivateAbilities()
{
	Super::OnRep_ActivateAbilities();

	OnAbilityListChanged.Broadcast();
}

void UPX_AbilitySystemComponent::HandleStunTagChanged(const FGameplayTag ChangedTag, int32 NewCount)
{
	(void)ChangedTag;

	if ( NewCount <= 0 )
	{
		if ( APX_Character* PX_Character = Cast<APX_Character>(GetAvatarActor()) )
		{
			PX_Character->ApplyLocomotionSpeedMode();
		}
		return;
	}

	CancelAbilitiesBlockedByTag(PX_GameplayTags::State_Condition_Stunned);
}

void UPX_AbilitySystemComponent::HandleGameplayEffectAppliedToSelf(UAbilitySystemComponent* TargetASC, const FGameplayEffectSpec& SpecApplied, FActiveGameplayEffectHandle ActiveHandle)
{
	(void)TargetASC;
	(void)SpecApplied;
	(void)ActiveHandle;

	if ( HasMatchingGameplayTag(PX_GameplayTags::State_Condition_Stunned) )
	{
		CancelAbilitiesBlockedByTag(PX_GameplayTags::State_Condition_Stunned);
	}
}

void UPX_AbilitySystemComponent::CancelAbilitiesBlockedByTag(const FGameplayTag& BlockingTag)
{
	if ( !BlockingTag.IsValid() )
	{
		return;
	}

	TArray<FGameplayAbilitySpecHandle> AbilityHandlesToCancel;
	TArray<TWeakObjectPtr<UPX_GameplayAbilityBase>> AbilityInstancesToCancel;

	for ( FGameplayAbilitySpec& Spec : GetActivatableAbilities() )
	{
		if ( !Spec.IsActive() || !Spec.Ability )
		{
			continue;
		}

		const UPX_GameplayAbilityBase* PXAbility = Cast<UPX_GameplayAbilityBase>(Spec.Ability);
		if ( PXAbility && PXAbility->HasActivationBlockedTag(BlockingTag) )
		{
			AbilityHandlesToCancel.Add(Spec.Handle);

			for ( UGameplayAbility* AbilityInstance : Spec.GetAbilityInstances() )
			{
				if ( UPX_GameplayAbilityBase* PXAbilityInstance = Cast<UPX_GameplayAbilityBase>(AbilityInstance) )
				{
					AbilityInstancesToCancel.Add(PXAbilityInstance);
				}
			}
		}
	}

	if ( AbilityHandlesToCancel.Num() > 0 )
	{
		PX_LOG(Log, TEXT("Cancel Abilities Blocked By Tag: %s, Count: %d"), *BlockingTag.ToString(), AbilityHandlesToCancel.Num());
	}

	for ( const TWeakObjectPtr<UPX_GameplayAbilityBase>& AbilityInstance : AbilityInstancesToCancel )
	{
		if ( AbilityInstance.IsValid() )
		{
			AbilityInstance->CancelFromStun();
		}
	}

	for ( const FGameplayAbilitySpecHandle& Handle : AbilityHandlesToCancel )
	{
		if ( const FGameplayAbilitySpec* Spec = FindAbilitySpecFromHandle(Handle) )
		{
			if ( Spec->IsActive() )
			{
				CancelAbilityHandle(Handle);
			}
		}
	}

	ClearAbilityInput();

	if ( BlockingTag == PX_GameplayTags::State_Condition_Stunned )
	{
		if ( APX_Character* PX_Character = Cast<APX_Character>(GetAvatarActor()) )
		{
			PX_Character->SetLocomotionJumping(false);
			PX_Character->SetLocomotionCrouching(false);
			PX_Character->ApplyLocomotionSpeedMode();

			if ( UCharacterMovementComponent* MovementComp = PX_Character->GetCharacterMovement() )
			{
				MovementComp->StopMovementImmediately();
			}
		}
	}
}

void UPX_AbilitySystemComponent::ClearAbilityInput()
{
	PressedSpecHandles.Reset();
	HeldSpecHandles.Reset();
	ReleasedSpecHandles.Reset();
	PressedInputTagsBySpecHandle.Reset();
	ReleasedInputTagsBySpecHandle.Reset();
	ProcessingInputTag = FGameplayTag();
	bProcessingInputTagPressed = false;
}

void UPX_AbilitySystemComponent::QueueServerFallbackActivation(const FGameplayAbilitySpecHandle& Handle)
{
	if ( !Handle.IsValid() )
	{
		return;
	}

	PendingServerFallbackActivationHandles.AddUnique(Handle);

	UWorld* World = GetWorld();
	if ( !World )
	{
		ProcessServerFallbackActivations();
		return;
	}

	if ( !World->GetTimerManager().IsTimerActive(ServerFallbackActivationTimerHandle) )
	{
		World->GetTimerManager().SetTimer(ServerFallbackActivationTimerHandle, this, &UPX_AbilitySystemComponent::ProcessServerFallbackActivations, PX_ServerFallbackActivationDelay, false);
	}
}

void UPX_AbilitySystemComponent::ClearServerFallbackActivation(const FGameplayAbilitySpecHandle& Handle)
{
	PendingServerFallbackActivationHandles.Remove(Handle);
}

void UPX_AbilitySystemComponent::ProcessServerFallbackActivations()
{
	TArray<FGameplayAbilitySpecHandle> HandlesToActivate = PendingServerFallbackActivationHandles;
	PendingServerFallbackActivationHandles.Reset();

	for ( const FGameplayAbilitySpecHandle& Handle : HandlesToActivate )
	{
		FGameplayAbilitySpec* Spec = FindAbilitySpecFromHandle(Handle);
		if ( !Spec || !Spec->Ability || Spec->IsActive() || !IsAbilitySpecHandleHeld(Handle) )
		{
			continue;
		}

		if ( !Spec->Ability->GetClass()->IsChildOf(UPX_GameplayAbility_GunFire::StaticClass()) )
		{
			continue;
		}

		PX_LOG(Log, TEXT("Server fallback TryActivate gun fire ability: %s"), *GetNameSafe(Spec->Ability));
		TryActivateAbility(Handle);
	}
}

void UPX_AbilitySystemComponent::CancelInputActivatedAbilities()
{
	TArray<FGameplayAbilitySpecHandle> AbilityHandlesToCancel;
	TArray<TWeakObjectPtr<UPX_GameplayAbilityBase>> AbilityInstancesToCancel;

	for ( FGameplayAbilitySpec& Spec : GetActivatableAbilities() )
	{
		if ( !Spec.IsActive() || !Spec.Ability )
		{
			continue;
		}

		const UPX_GameplayAbilityBase* PXAbility = Cast<UPX_GameplayAbilityBase>(Spec.Ability);
		const bool bHasInputTag = PXAbility && PXAbility->GetInputTag().IsValid();
		const bool bHasDynamicInputTag = !Spec.DynamicAbilityTags.IsEmpty();
		if ( !bHasInputTag && !bHasDynamicInputTag )
		{
			continue;
		}

		AbilityHandlesToCancel.Add(Spec.Handle);

		for ( UGameplayAbility* AbilityInstance : Spec.GetAbilityInstances() )
		{
			if ( UPX_GameplayAbilityBase* PXAbilityInstance = Cast<UPX_GameplayAbilityBase>(AbilityInstance) )
			{
				AbilityInstancesToCancel.Add(PXAbilityInstance);
			}
		}
	}

	if ( AbilityHandlesToCancel.Num() > 0 )
	{
		PX_LOG(Log, TEXT("Cancel Input Activated Abilities. Count: %d"), AbilityHandlesToCancel.Num());
	}

	for ( const TWeakObjectPtr<UPX_GameplayAbilityBase>& AbilityInstance : AbilityInstancesToCancel )
	{
		if ( AbilityInstance.IsValid() )
		{
			AbilityInstance->CancelFromExternal();
		}
	}

	for ( const FGameplayAbilitySpecHandle& Handle : AbilityHandlesToCancel )
	{
		if ( const FGameplayAbilitySpec* Spec = FindAbilitySpecFromHandle(Handle) )
		{
			if ( Spec->IsActive() )
			{
				CancelAbilityHandle(Handle);
			}
		}
	}

	ClearAbilityInput();
}

void UPX_AbilitySystemComponent::AbilityInputTagPressed(const FGameplayTag& InputTag)
{	
	if ( !InputTag.IsValid() )
	{
		PX_LOG(Warning, TEXT("Invalid InputTag: %s"), *InputTag.ToString());
		return;
	}

	PX_LOG(Log, TEXT("Pressed Tag: %s"), *InputTag.ToString());

	/*
	PressedInputTags.Add(InputTag);
	HeldInputTags.Add(InputTag);
	ReleasedInputTags.Remove(InputTag);
	*/

	LastInputTag = InputTag;
	bLastInputTagPressed = true;

	// 깊이가 가장 깊은 Spec만 추가
	FGameplayAbilitySpec* BestMatchedSpec = nullptr;
	int32 BestMatchedDepth = -1;

	for ( FGameplayAbilitySpec& Spec : GetActivatableAbilities() )
	{
		/* 일치하는 하위 태그까지 모두 추가
		for ( const FGameplayTag& SpecTag : Spec.DynamicAbilityTags )
		{
			if ( InputTag.MatchesTag(SpecTag) )
			{
				PX_LOG(Log, TEXT("Add Pressed Tag: %s to Spec: %s"), *InputTag.ToString(), *GetNameSafe(Spec.Ability));
				PressedSpecHandles.AddUnique(Spec.Handle);
				HeldSpecHandles.AddUnique(Spec.Handle);
				ReleasedSpecHandles.Remove(Spec.Handle);
				break;
			}
		}
		*/

		/* 정확히 일치하는 태그만 추가
		if ( Spec.DynamicAbilityTags.HasTagExact(InputTag) )
		{
			PX_LOG(Log, TEXT("Add Pressed Tag: %s to Spec: %s"), *InputTag.ToString(), *GetNameSafe(Spec.Ability));
			PressedSpecHandles.AddUnique(Spec.Handle);
			HeldSpecHandles.AddUnique(Spec.Handle);
			ReleasedSpecHandles.Remove(Spec.Handle);
		}
		*/

		
		// 깊이가 가장 깊은 Spec만 추가
		for ( const FGameplayTag& SpecTag : Spec.DynamicAbilityTags )
		{
			if ( InputTag.MatchesTag(SpecTag) )
			{
				// Tag String에서 
				const FString TagString = SpecTag.ToString();
				// '.'의 개수를 셈
				int32 Depth = 0;
				for ( const TCHAR Char : TagString )
				{
					if ( Char == TEXT('.') )
					{
						++Depth;
					}
				}

				// 가장 깊은 스펙만 고름
				if ( Depth > BestMatchedDepth )
				{
					BestMatchedDepth = Depth;
					BestMatchedSpec = &Spec;
				}
			}
		}
	}

	if ( BestMatchedSpec )
	{
		PX_LOG(Log, TEXT("Add Pressed Tag: %s to Spec: %s"), *InputTag.ToString(), *GetNameSafe(BestMatchedSpec->Ability));
		PressedSpecHandles.AddUnique(BestMatchedSpec->Handle);
		HeldSpecHandles.AddUnique(BestMatchedSpec->Handle);
		ReleasedSpecHandles.Remove(BestMatchedSpec->Handle);
		PressedInputTagsBySpecHandle.FindOrAdd(BestMatchedSpec->Handle).Add(InputTag);
		ReleasedInputTagsBySpecHandle.Remove(BestMatchedSpec->Handle);
	}

	if ( GetOwnerRole() != ROLE_Authority )
	{
		ServerAbilityInputTagPressed(InputTag);
	}
}

void UPX_AbilitySystemComponent::ServerAbilityInputTagPressed_Implementation(const FGameplayTag& InputTag)
{
	AbilityInputTagPressed(InputTag);

	for ( const FGameplayAbilitySpecHandle& Handle : PressedSpecHandles )
	{
		FGameplayAbilitySpec* Spec = FindAbilitySpecFromHandle(Handle);
		if ( !Spec || !Spec->Ability || Spec->IsActive() )
		{
			continue;
		}

		if ( Spec->Ability->GetClass()->IsChildOf(UPX_GA_WeaponStatusImbueBase::StaticClass()) )
		{
			PX_LOG(Log, TEXT("Server fallback TryActivate status imbue ability: %s"), *GetNameSafe(Spec->Ability));
			TryActivateAbility(Handle);
		}
		else if ( Spec->Ability->GetClass()->IsChildOf(UPX_GameplayAbility_GunFire::StaticClass()) )
		{
			QueueServerFallbackActivation(Handle);
		}
	}

	// LocalPredicted activation is already sent to the server through GAS.
	// The custom input RPC only mirrors held-state for server-side loops such as auto fire.
	PressedSpecHandles.Reset();
	ReleasedSpecHandles.Reset();
	PressedInputTagsBySpecHandle.Reset();
	ReleasedInputTagsBySpecHandle.Reset();
}

void UPX_AbilitySystemComponent::AbilityInputTagReleased(const FGameplayTag& InputTag)
{
	//PX_LOG(Log, TEXT("Released Tag: %s"), *InputTag.ToString());

	if ( !InputTag.IsValid() )
	{
		PX_LOG(Warning, TEXT("Invalid InputTag: %s"), *InputTag.ToString());
		return;
	}

	/*
	PressedInputTags.Remove(InputTag);
	HeldInputTags.Remove(InputTag);
	ReleasedInputTags.Add(InputTag);
	*/

	LastInputTag = InputTag;
	bLastInputTagPressed = false;

	// 깊이가 가장 깊은 Spec만 추가
	FGameplayAbilitySpec* BestMatchedSpec = nullptr;
	int32 BestMatchedDepth = -1;

	for ( FGameplayAbilitySpec& Spec : GetActivatableAbilities() )
	{
		/* 일치하는 하위 태그까지 모두 추가
		for ( const FGameplayTag& SpecTag : Spec.DynamicAbilityTags )
		{
			if ( InputTag.MatchesTag(SpecTag) )
			{
				PX_LOG(Log, TEXT("Add Released Tag: %s to Spec: %s"), *InputTag.ToString(), *GetNameSafe(Spec.Ability));
				PressedSpecHandles.Remove(Spec.Handle);
				HeldSpecHandles.Remove(Spec.Handle);
				ReleasedSpecHandles.AddUnique(Spec.Handle);
				break;
			}
		}
		*/

		/* 정확히 일치하는 태그만 검사
		if ( Spec.DynamicAbilityTags.HasTagExact(InputTag) )
		{
			PX_LOG(Log, TEXT("Add Released Tag: %s to Spec: %s"), *InputTag.ToString(), *GetNameSafe(Spec.Ability));
			PressedSpecHandles.Remove(Spec.Handle);
			HeldSpecHandles.Remove(Spec.Handle);
			ReleasedSpecHandles.AddUnique(Spec.Handle);
		}
		*/

		for ( const FGameplayTag& SpecTag : Spec.DynamicAbilityTags )
		{
			if ( InputTag.MatchesTag(SpecTag) )
			{
				const FString TagString = SpecTag.ToString();

				int32 Depth = 0;
				for ( const TCHAR Char : TagString )
				{
					if ( Char == TEXT('.') )
					{
						++Depth;
					}
				}

				if ( Depth > BestMatchedDepth )
				{
					BestMatchedDepth = Depth;
					BestMatchedSpec = &Spec;
				}
			}
		}
	}

	if ( BestMatchedSpec )
	{
		PX_LOG(Log, TEXT("Add Released Tag: %s to Spec: %s"), *InputTag.ToString(), *GetNameSafe(BestMatchedSpec->Ability));
		PressedSpecHandles.Remove(BestMatchedSpec->Handle);
		HeldSpecHandles.Remove(BestMatchedSpec->Handle);
		ReleasedSpecHandles.AddUnique(BestMatchedSpec->Handle);
		PressedInputTagsBySpecHandle.Remove(BestMatchedSpec->Handle);
		ReleasedInputTagsBySpecHandle.FindOrAdd(BestMatchedSpec->Handle).Add(InputTag);
	}

	if ( GetOwnerRole() != ROLE_Authority )
	{
		ServerAbilityInputTagReleased(InputTag);
	}
}

void UPX_AbilitySystemComponent::ServerAbilityInputTagReleased_Implementation(const FGameplayTag& InputTag)
{
	AbilityInputTagReleased(InputTag);

	for ( const FGameplayAbilitySpecHandle& Handle : ReleasedSpecHandles )
	{
		ClearServerFallbackActivation(Handle);

		FGameplayAbilitySpec* Spec = FindAbilitySpecFromHandle(Handle);
		if ( !Spec || !Spec->Ability || !Spec->IsActive() )
		{
			continue;
		}

		if ( !Spec->Ability->GetClass()->IsChildOf(UPX_GameplayAbility_GunFire::StaticClass()) )
		{
			continue;
		}

		PX_LOG(Log, TEXT("Server immediate release gun fire ability: %s"), *GetNameSafe(Spec->Ability));
		AbilitySpecInputReleased(*Spec);
		InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::InputReleased, Spec->Handle, Spec->ActivationInfo.GetActivationPredictionKey());
	}

	PressedSpecHandles.Reset();
	ReleasedSpecHandles.Reset();
	PressedInputTagsBySpecHandle.Reset();
	ReleasedInputTagsBySpecHandle.Reset();
}

void UPX_AbilitySystemComponent::ProcessAbilityInput(float DeltaTime, bool bGamePaused)
{
	if ( HasMatchingGameplayTag(PX_GameplayTags::State_Condition_Stunned) )
	{
		CancelAbilitiesBlockedByTag(PX_GameplayTags::State_Condition_Stunned);
		return;
	}

	// Pressed 할 때 실행되는 Ability
	//PX_LOG(Log, TEXT("Process Pressed Spec Handle"));
	for ( const FGameplayAbilitySpecHandle& Handle : PressedSpecHandles )
	{
		if ( FGameplayAbilitySpec* Spec = FindAbilitySpecFromHandle(Handle) )
		{
			const TArray<FGameplayTag> InputTags = PressedInputTagsBySpecHandle.FindRef(Handle);
			for ( const FGameplayTag& InputTag : InputTags )
			{
				if ( !InputTag.IsValid() )
				{
					continue;
				}

				ProcessingInputTag = InputTag;
			bProcessingInputTagPressed = true;

			AbilitySpecInputPressed(*Spec);

			// 비활성 상태인 Ability를 활성화하도록 시도
			if ( !Spec->IsActive() )
			{
				PX_LOG(Log, TEXT("Pressed Action %s: , TryActivateAbility"), *GetNameSafe(Spec->Ability));

				// TryActivateAbility()는 비활성 Ability에 대해 실행 가능한지 검사하고, 실행 가능하면 ActivateAbility()를 호출
				TryActivateAbility(Handle);
			}
			// 활성 상태인 Ability는 InputPressed를 호출
			else
			{
				PX_LOG(Log, TEXT("Pressed Action %s: , Call InputPressed()"), *GetNameSafe(Spec->Ability));

				InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::InputPressed, Spec->Handle, Spec->ActivationInfo.GetActivationPredictionKey());
			}
			}
		}
	}

	// Released 할 때 실행되는 Ability
	//PX_LOG(Log, TEXT("Process Released Spec Handle"));
	for ( const FGameplayAbilitySpecHandle& Handle : ReleasedSpecHandles )
	{
		if ( FGameplayAbilitySpec* Spec = FindAbilitySpecFromHandle(Handle) )
		{
			const TArray<FGameplayTag> InputTags = ReleasedInputTagsBySpecHandle.FindRef(Handle);
			for ( const FGameplayTag& InputTag : InputTags )
			{
				if ( !InputTag.IsValid() )
				{
					continue;
				}

				ProcessingInputTag = InputTag;
			bProcessingInputTagPressed = false;

			AbilitySpecInputReleased(*Spec);

			// 활성 상태인 Ability는 InputReleased를 호출
			if ( Spec->IsActive() )
			{
				PX_LOG(Log, TEXT("Released Action %s: , Call InputReleased()"), *GetNameSafe(Spec->Ability));

				// LocalPredicted / Server 처리 모두 포함해서
				// AbilityTask_WaitInputRelease, InputReleased 기반 로직이 받을 수 있도록 이벤트 전달
				InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::InputReleased, Spec->Handle, Spec->ActivationInfo.GetActivationPredictionKey());
			}
			}
		}
	}

	//PressedInputTags.Reset();
	//ReleasedInputTags.Reset();

	PressedSpecHandles.Reset();
	ReleasedSpecHandles.Reset();
	PressedInputTagsBySpecHandle.Reset();
	ReleasedInputTagsBySpecHandle.Reset();
	ProcessingInputTag = FGameplayTag();
	bProcessingInputTagPressed = false;
}

/*
bool UPX_AbilitySystemComponent::IsInputTagPressed(const FGameplayTag& InputTag) const
{
	if ( !InputTag.IsValid() ) return false;

	for ( const FGameplayTag& PressedTag : PressedInputTags )
	{
		if ( PressedTag.MatchesTag(InputTag) )
		{
			//PX_LOG(Log, TEXT("InputTag : %s is Pressed"), *InputTag.ToString());
			return true;
		}
	}

	//PX_LOG(Log, TEXT("InputTag : %s is not Pressed"), *InputTag.ToString());
	return false;
}

bool UPX_AbilitySystemComponent::IsInputTagHeld(const FGameplayTag& InputTag) const
{
	if ( !InputTag.IsValid() ) return false;

	for ( const FGameplayTag& HeldTag : HeldInputTags )
	{
		if ( HeldTag.MatchesTag(InputTag) )
		{
			//PX_LOG(Log, TEXT("InputTag : %s is Held"), *InputTag.ToString());
			return true;
		}
	}

	//PX_LOG(Log, TEXT("InputTag : %s is not Held"), *InputTag.ToString());
	return false;
}

bool UPX_AbilitySystemComponent::IsInputTagReleased(const FGameplayTag& InputTag) const
{
	if ( !InputTag.IsValid() ) return false;

	for ( const FGameplayTag& ReleasedTag : ReleasedInputTags )
	{
		if ( ReleasedTag.MatchesTag(InputTag) )
		{
			//PX_LOG(Log, TEXT("InputTag : %s is Released"), *InputTag.ToString());
			return true;
		}
	}

	//PX_LOG(Log, TEXT("InputTag : %s is not Released"), *InputTag.ToString());
	return false;
}

bool UPX_AbilitySystemComponent::IsInputTagPressedExact(const FGameplayTag& InputTag) const
{
	if ( !InputTag.IsValid() ) return false;

	//PX_LOG(Log, TEXT("InputTag : %s is %sPressed"), *InputTag.ToString(), PressedInputTags.Contains(InputTag) ? TEXT("") : TEXT("not "));

	return PressedInputTags.Contains(InputTag);
}

bool UPX_AbilitySystemComponent::IsInputTagHeldExact(const FGameplayTag& InputTag) const
{
	if ( !InputTag.IsValid() ) return false;

	//PX_LOG(Log, TEXT("InputTag : %s is %sHeld"), *InputTag.ToString(), HeldInputTags.Contains(InputTag) ? TEXT("") : TEXT("not "));

	return HeldInputTags.Contains(InputTag);
}

bool UPX_AbilitySystemComponent::IsInputTagReleasedExact(const FGameplayTag& InputTag) const
{
	if ( !InputTag.IsValid() ) return false;

	//PX_LOG(Log, TEXT("InputTag : %s is %sReleased"), *InputTag.ToString(), ReleasedInputTags.Contains(InputTag) ? TEXT("") : TEXT("not "));

	return ReleasedInputTags.Contains(InputTag);
}
*/

bool UPX_AbilitySystemComponent::IsAbilitySpecHandlePressed(const FGameplayAbilitySpecHandle& Handle) const
{
	return PressedSpecHandles.Contains(Handle);
}

bool UPX_AbilitySystemComponent::IsAbilitySpecHandleHeld(const FGameplayAbilitySpecHandle& Handle) const
{
	return HeldSpecHandles.Contains(Handle);
}
