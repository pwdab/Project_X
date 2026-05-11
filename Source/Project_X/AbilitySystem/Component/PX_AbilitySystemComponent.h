// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Project_X.h"
#include "AbilitySystemComponent.h"
#include "GameplayTagContainer.h"
#include "TimerManager.h"
#include "PX_AbilitySystemComponent.generated.h"

DECLARE_MULTICAST_DELEGATE(FPXOnAbilityListChanged);

/**
 * 
 */
UCLASS()
class PROJECT_X_API UPX_AbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()
	
public:
	FPXOnAbilityListChanged OnAbilityListChanged;

	void AbilityInputTagPressed(const FGameplayTag& InputTag);
	void AbilityInputTagReleased(const FGameplayTag& InputTag);
	void ProcessAbilityInput(float DeltaTime, bool bGamePaused);
	void ClearAbilityInput();
	void CancelInputActivatedAbilities();
	void ClearServerFallbackActivation(const FGameplayAbilitySpecHandle& Handle);

	FORCEINLINE FGameplayTag GetLastInputTag() const { return LastInputTag; }
	FORCEINLINE bool IsLastInputTagPressed() const { return bLastInputTagPressed; }
	FORCEINLINE FGameplayTag GetProcessingInputTag() const { return ProcessingInputTag.IsValid() ? ProcessingInputTag : LastInputTag; }
	FORCEINLINE bool IsProcessingInputTagPressed() const { return ProcessingInputTag.IsValid() ? bProcessingInputTagPressed : bLastInputTagPressed; }

	/*
	// 하위 태그까지 검색
	bool IsInputTagPressed(const FGameplayTag& InputTag) const;
	bool IsInputTagHeld(const FGameplayTag& InputTag) const;
	bool IsInputTagReleased(const FGameplayTag& InputTag) const;
	// 세부 태그만 검색
	bool IsInputTagPressedExact(const FGameplayTag& InputTag) const;
	bool IsInputTagHeldExact(const FGameplayTag& InputTag) const;
	bool IsInputTagReleasedExact(const FGameplayTag& InputTag) const;
	*/

	bool IsAbilitySpecHandlePressed(const FGameplayAbilitySpecHandle& Handle) const;
	bool IsAbilitySpecHandleHeld(const FGameplayAbilitySpecHandle& Handle) const;
	void CancelAbilitiesBlockedByTag(const FGameplayTag& BlockingTag);

protected:
	virtual void BeginPlay() override;
	virtual void OnGiveAbility(FGameplayAbilitySpec& AbilitySpec) override;
	virtual void OnRemoveAbility(FGameplayAbilitySpec& AbilitySpec) override;
	virtual void OnRep_ActivateAbilities() override;

	void HandleStunTagChanged(const FGameplayTag ChangedTag, int32 NewCount);
	void HandleGameplayEffectAppliedToSelf(UAbilitySystemComponent* TargetASC, const FGameplayEffectSpec& SpecApplied, FActiveGameplayEffectHandle ActiveHandle);
	void QueueServerFallbackActivation(const FGameplayAbilitySpecHandle& Handle);
	void ProcessServerFallbackActivations();

	UFUNCTION(Server, Reliable)
	void ServerAbilityInputTagPressed(const FGameplayTag& InputTag);
	UFUNCTION(Server, Reliable)
	void ServerAbilityInputTagReleased(const FGameplayTag& InputTag);

protected:
	// 어떤 Ability Spec이 입력 상태인가
	UPROPERTY()
	TArray<FGameplayAbilitySpecHandle> PressedSpecHandles;
	UPROPERTY()
	TArray<FGameplayAbilitySpecHandle> HeldSpecHandles;
	UPROPERTY()
	TArray<FGameplayAbilitySpecHandle> ReleasedSpecHandles;

	TMap<FGameplayAbilitySpecHandle, TArray<FGameplayTag>> PressedInputTagsBySpecHandle;
	TMap<FGameplayAbilitySpecHandle, TArray<FGameplayTag>> ReleasedInputTagsBySpecHandle;

	TArray<FGameplayAbilitySpecHandle> PendingServerFallbackActivationHandles;
	FTimerHandle ServerFallbackActivationTimerHandle;

	// 마지막으로 눌린 InputTag
	UPROPERTY(Transient)
	FGameplayTag LastInputTag;
	// 마지막으로 눌린 InputTag가 Released 되었는지
	UPROPERTY(Transient)
	bool bLastInputTagPressed = false;

	UPROPERTY(Transient)
	FGameplayTag ProcessingInputTag;
	UPROPERTY(Transient)
	bool bProcessingInputTagPressed = false;
	
	/*
	// 어떤 InputTag가 현재 눌려 있는가
	UPROPERTY()
	TSet<FGameplayTag> PressedInputTags;
	UPROPERTY()
	TSet<FGameplayTag> HeldInputTags;
	UPROPERTY()
	TSet<FGameplayTag> ReleasedInputTags;
	*/
};
