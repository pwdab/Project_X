// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Project_X.h"
#include "Component/Inventory/PX_ItemInstance.h"
#include "PX_EquippableItemInstance.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(PX_EquippableItemInstance, Log, All);

class UAbilitySystemComponent;
struct FGameplayAbilitySpecHandle;

/**
 * Item instance that can be equipped and grant abilities / tags.
 */
UCLASS()
class PROJECT_X_API UPX_EquippableItemInstance : public UPX_ItemInstance
{
	GENERATED_BODY()

public:
	// Ability 부여
	virtual void GiveAbilities(UAbilitySystemComponent* ASC);
	// Ability 회수
	virtual void RemoveAbilities(UAbilitySystemComponent* ASC);
	// Tag 부여
	virtual void AddTags(UAbilitySystemComponent* ASC);
	// Tag 회수
	virtual void RemoveTags(UAbilitySystemComponent* ASC);

	FORCEINLINE bool HasGivenAbilities() const { return GivenAbilityHandles.Num() > 0; }

protected:
	// 이 아이템이 ASC에 부여한 Ability Handle 목록
	UPROPERTY()
	TArray<FGameplayAbilitySpecHandle> GivenAbilityHandles;
};
