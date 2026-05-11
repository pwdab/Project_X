// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Project_X.h"
#include "Engine/DataAsset.h"
#include "GameplayAbilitySpec.h"
#include "PX_AbilitiesDataAsset.generated.h"

class UGameplayAbility;

USTRUCT(BlueprintType)
struct FPX_AbilityEntry
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UGameplayAbility> AbilityClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 AbilityLevel = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 InputID = INDEX_NONE;
};

/**
 * 
 */
UCLASS()
class PROJECT_X_API UPX_AbilitiesDataAsset : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<FPX_AbilityEntry> Abilities;

};
