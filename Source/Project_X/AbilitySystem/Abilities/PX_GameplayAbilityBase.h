// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Project_X.h"
#include "Abilities/GameplayAbility.h"
#include "AbilitySystem/Component/PX_AbilitySystemComponent.h"
#include "PX_GameplayAbilityBase.generated.h"

class APX_Character;
class UPX_WeaponSystemComponent;

/**
 * 
 */
UCLASS()
class PROJECT_X_API UPX_GameplayAbilityBase : public UGameplayAbility
{
	GENERATED_BODY()
	
public:
	FORCEINLINE const FGameplayTag& GetInputTag() const { return InputTag; }
	FORCEINLINE bool HasActivationBlockedTag(const FGameplayTag& Tag) const { return ActivationBlockedTags.HasTagExact(Tag); }
	void CancelFromExternal();
	void CancelFromStun();

protected:
	virtual bool ShouldUseClientPrediction(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo* ActivationInfo = nullptr) const;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PX|Input", meta = (Categories = "Input"))
	FGameplayTag InputTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PX|Prediction")
	bool bUseClientPrediction = true;
	
	// Getter
	APX_Character* GetPXCharacter(const FGameplayAbilityActorInfo* ActorInfo) const;
	UPX_WeaponSystemComponent* GetWeaponSystemComponent(const FGameplayAbilityActorInfo* ActorInfo) const;
	UAbilitySystemComponent* GetAbilitySystemComponent(const FGameplayAbilityActorInfo* ActorInfo) const;
};
