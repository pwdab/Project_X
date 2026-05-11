// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Project_X.h"
#include "EnhancedInputComponent.h"
#include "GameplayTagContainer.h"
#include "Input/Config/PX_InputConfigDataAsset.h"
#include "PX_EnhancedInputComponent.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_X_API UPX_EnhancedInputComponent : public UEnhancedInputComponent
{
	GENERATED_BODY()
	
public:
	template <class UserClass, typename FuncType>
	void BindNativeInputAction(const UPX_InputConfigDataAsset* InputConfigDataAsset, const FGameplayTag& InputTag, ETriggerEvent TriggerEvent, UserClass* Object, FuncType Func);

	template <class UserClass, typename PressedFuncType, typename ReleasedFuncType>
	void BindAbilityInputActions(const UPX_InputConfigDataAsset* InputConfigDataAsset, UserClass* Object, PressedFuncType PressedFunc, ReleasedFuncType ReleasedFunc);
	
};

template<class UserClass, typename FuncType>
inline void UPX_EnhancedInputComponent::BindNativeInputAction(const UPX_InputConfigDataAsset* InputConfigDataAsset, const FGameplayTag& InputTag, ETriggerEvent TriggerEvent, UserClass* Object, FuncType Func)
{
	if ( !InputConfigDataAsset || !InputTag.IsValid() || !Object || !Func ) return;

	if ( const UInputAction* InputAction = InputConfigDataAsset->FindNativeInputActionByTag(InputTag) )
	{
		BindAction(InputAction, TriggerEvent, Object, Func);
	}
}

template<class UserClass, typename PressedFuncType, typename ReleasedFuncType>
inline void UPX_EnhancedInputComponent::BindAbilityInputActions(const UPX_InputConfigDataAsset* InputConfigDataAsset, UserClass* Object, PressedFuncType PressedFunc, ReleasedFuncType ReleasedFunc)
{
	if ( !InputConfigDataAsset || !Object ) return;

	for ( const FPXInputActionTagPair& Pair : InputConfigDataAsset->AbilityInputActions )
	{
		if ( !Pair.InputAction || !Pair.InputTag.IsValid() ) continue;

		if ( PressedFunc )
		{
			BindAction(Pair.InputAction, ETriggerEvent::Started, Object, PressedFunc, Pair.InputTag);
		}

		if ( ReleasedFunc )
		{
			BindAction(Pair.InputAction, ETriggerEvent::Completed, Object, ReleasedFunc, Pair.InputTag);
		}
	}
}
