// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Project_X.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "InputAction.h"
#include "PX_InputConfigDataAsset.generated.h"

/**
 * 
 */

USTRUCT(BlueprintType)
struct FPXInputActionTagPair
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<const UInputAction> InputAction = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (Categories = "Input"))
	FGameplayTag InputTag;
};

UCLASS()
class PROJECT_X_API UPX_InputConfigDataAsset : public UDataAsset
{
	GENERATED_BODY()
	
public:
	// Move, Look, ToggleInventory 같이 AbilitySystem을 사용하지 않는 Native 입력용
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TArray<FPXInputActionTagPair> NativeInputActions;

	// Jump, Fire, Reload, ADS 같이 AbilitySystem을 사용하는 Ability 입력용
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TArray<FPXInputActionTagPair> AbilityInputActions;

public:
	const UInputAction* FindNativeInputActionByTag(const FGameplayTag& InputTag) const;
	const UInputAction* FindAbilityInputActionByTag(const FGameplayTag& InputTag) const;
	
	
};
