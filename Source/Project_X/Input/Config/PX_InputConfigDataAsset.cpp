// Fill out your copyright notice in the Description page of Project Settings.


#include "Input/Config/PX_InputConfigDataAsset.h"

const UInputAction* UPX_InputConfigDataAsset::FindNativeInputActionByTag(const FGameplayTag& InputTag) const
{
	if ( !InputTag.IsValid() ) return nullptr;

	for ( const FPXInputActionTagPair& Pair : NativeInputActions )
	{
		if ( Pair.InputAction && Pair.InputTag.MatchesTagExact(InputTag) )
		{
			return Pair.InputAction;
		}
	}

	return nullptr;
}

const UInputAction* UPX_InputConfigDataAsset::FindAbilityInputActionByTag(const FGameplayTag& InputTag) const
{
	if ( !InputTag.IsValid() ) return nullptr;

	for ( const FPXInputActionTagPair& Pair : AbilityInputActions )
	{
		if ( Pair.InputAction && Pair.InputTag.MatchesTagExact(InputTag) )
		{
			return Pair.InputAction;
		}
	}

	return nullptr;
}
