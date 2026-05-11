// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Project_X.h"
#include "AbilitySystem/Abilities/PX_GameplayAbilityBase.h"
#include "PX_GA_Aim.generated.h"

// 현재 Aim 상태
UENUM(BlueprintType)
enum class EAimState : uint8
{
	Idle = 0			UMETA(DisplayName = "Idle"),		// 비조준
	HipFire = 1 << 1	UMETA(DisplayName = "HipFire"),     // HipFire
	OTS = 1 << 2		UMETA(DisplayName = "OTS"),			// Over The Shoulder
	ADS = 1 << 3		UMETA(DisplayName = "ADS"),			// Aim Down Sight
};

// 내부 Aim 상태를 관리하는 구조체
USTRUCT(BlueprintType)
struct FAimBitSetState
{
	GENERATED_BODY()

private:
	uint8 PrevBits = 0;
	uint8 CurBits = 0;

public:
	void Clear()
	{
		PrevBits = 0;
		CurBits = 0;
	}

	void SavePrevious()
	{
		PrevBits = CurBits;
	}

	// 현재 Aim 상태에 새로운 Aim 상태를 추가
	void AddState(EAimState NewState)
	{
		CurBits |= static_cast<uint8>(NewState);
	}

	// 현재 Aim 상태에 특정 Aim 상태를 제거
	void RemoveState(EAimState StateToRemove)
	{
		CurBits &= ~static_cast<uint8>(StateToRemove);
	}

	bool Has(EAimState State) const { return (CurBits & static_cast<uint8>(State)) != 0; }						// 현재 Aim 상태가 특정 Aim 상태를 가지는지
	bool IsADS() const { return Has(EAimState::ADS); }															// ADS Aim 상태인지
	bool IsOTS() const { return Has(EAimState::OTS); }															// OTS Aim 상태인지
	bool IsHipFire() const { return Has(EAimState::HipFire); }													// Hip Fire Aim 상태인지
	bool IsAiming() const { return Has(EAimState::HipFire) || Has(EAimState::OTS) || Has(EAimState::ADS);; }	// 조준 상태인지
	bool IsIdle() const { return CurBits == 0; }																// 비조준 상태인지
	bool WasAiming() const { return (PrevBits & 0b1110) != 0; }													// 이전 상태가 조준 상태였는지
	bool ShouldBeginAim() const { return !WasAiming() && IsAiming(); }											// 조준을 시작해야 하는지
	bool ShouldEndAim() const { return WasAiming() && !IsAiming(); }											// 조준을 종료해야 하는지
};

/**
 *
 */
UCLASS()
class PROJECT_X_API UPX_GA_Aim : public UPX_GameplayAbilityBase
{
	GENERATED_BODY()

public:
	UPX_GA_Aim();
	void EndHipFireForReload();

protected:
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	virtual void InputPressed(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) override;
	virtual void InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) override;

protected:
	void UpdateAimState();
	void ClearAimStateTags();
	void ApplyAiming(bool bNewIsAiming);
	void ApplyAimCameraMode();

private:
	// 현재 내부 Aim 상태를 저장
	UPROPERTY(Transient)
	FAimBitSetState AimBitState;

	UPROPERTY(Transient)
	bool bAppliedIsAiming = false;
};
