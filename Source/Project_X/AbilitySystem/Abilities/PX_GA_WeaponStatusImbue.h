// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Project_X.h"
#include "AbilitySystem/Abilities/PX_GameplayAbilityBase.h"
#include "PX_GA_WeaponStatusImbue.generated.h"

class UGameplayEffect;
class UTexture2D;

UCLASS(Abstract)
class PROJECT_X_API UPX_GA_WeaponStatusImbueBase : public UPX_GameplayAbilityBase
{
	GENERATED_BODY()

public:
	UPX_GA_WeaponStatusImbueBase();
	FORCEINLINE FGameplayTag GetCooldownTag() const { return CooldownTag; }
	FORCEINLINE UTexture2D* GetSkillIcon() const { return SkillIcon; }

protected:
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual const FGameplayTagContainer* GetCooldownTags() const override;
	virtual void ApplyCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const override;

	UPROPERTY(EditDefaultsOnly, Category = "PX|Status")
	TSubclassOf<UGameplayEffect> StatusGameplayEffectClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PX|Cooldown")
	float Cooldown = 5.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PX|Cooldown", meta = (Categories = "Cooldown"))
	FGameplayTag CooldownTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PX|Cooldown")
	TSubclassOf<UGameplayEffect> ImbueCooldownGameplayEffectClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PX|UI")
	TObjectPtr<UTexture2D> SkillIcon;

	mutable FGameplayTagContainer TempCooldownTags;
};

UCLASS()
class PROJECT_X_API UPX_GA_ImbueBurn : public UPX_GA_WeaponStatusImbueBase
{
	GENERATED_BODY()

public:
	UPX_GA_ImbueBurn();
};

UCLASS()
class PROJECT_X_API UPX_GA_ImbueSlow : public UPX_GA_WeaponStatusImbueBase
{
	GENERATED_BODY()

public:
	UPX_GA_ImbueSlow();
};

UCLASS()
class PROJECT_X_API UPX_GA_ImbueStun : public UPX_GA_WeaponStatusImbueBase
{
	GENERATED_BODY()

public:
	UPX_GA_ImbueStun();
};
