// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"

DECLARE_LOG_CATEGORY_EXTERN(Project_X, Log, All);

#define ECC_PX_Combat ECC_GameTraceChannel1
#define ECC_PX_SkelMesh ECC_GameTraceChannel2
#define ECC_PX_CombatTrace ECC_GameTraceChannel3

#define PX_LOG_CALLINFO (FString(__FUNCTION__) + TEXT("(") + FString::FromInt(__LINE__) + TEXT(")"))
// 기본 카테고리(Project_X) 사용
//#define PX_LOG(Verbosity, Format, ...) UE_LOG(Project_X, Verbosity, TEXT("%s %s"), *PX_LOG_CALLINFO, *FString::Printf(Format, ##__VA_ARGS__))
#define PX_LOG(Verbosity, Format, ...) UE_LOG(Project_X, Verbosity, TEXT("%s %s %s"), *PX_GetNetModeString(this), *PX_LOG_CALLINFO, *FString::Printf(Format, ##__VA_ARGS__))
// 카테고리 지정
//#define PX_LOGC(Category, Verbosity, Format, ...) UE_LOG(Category, Verbosity, TEXT("%s %s"), *PX_LOG_CALLINFO, *FString::Printf(Format, ##__VA_ARGS__))
#define PX_LOGC(Category, Verbosity, Format, ...) UE_LOG(Category, Verbosity, TEXT("%s %s %s"), *PX_GetNetModeString(this), *PX_LOG_CALLINFO, *FString::Printf(Format, ##__VA_ARGS__))


FORCEINLINE FString PX_GetNetModeString(const UObject* WorldContextObject)
{
	FString Label = TEXT("Unknown");

	if ( WorldContextObject )
	{
		if ( const UWorld* World = WorldContextObject->GetWorld() )
		{
			const ENetMode NetMode = World->GetNetMode();

			// Actor 추출
			const AActor* Actor = nullptr;

			// GameplayAbility는 AvatarActor 기준
			if ( const UGameplayAbility* Ability = Cast<UGameplayAbility>(WorldContextObject) )
			{
				if ( Ability->GetCurrentActorInfo() )
				{
					Actor = Ability->GetCurrentActorInfo()->AvatarActor.Get();
				}
			}
			// ASC는 AvatarActor 기준
			else if ( const UAbilitySystemComponent* ASC = Cast<UAbilitySystemComponent>(WorldContextObject) )
			{
				if ( ASC->AbilityActorInfo.IsValid() )
				{
					Actor = ASC->AbilityActorInfo->AvatarActor.Get();
				}

				if ( !Actor )
				{
					Actor = ASC->GetAvatarActor();
				}

				if ( !Actor )
				{
					Actor = ASC->GetOwnerActor();
				}
			}
			// Actor면 그대로 사용
			else if ( const AActor* AsActor = Cast<AActor>(WorldContextObject) )
			{
				Actor = AsActor;
			}
			// 일반 Component는 Owner 사용
			else if ( const UActorComponent* Comp = Cast<UActorComponent>(WorldContextObject) )
			{
				Actor = Comp->GetOwner();
			}

			// Client
			if ( NetMode == NM_Client )
			{
				if ( Actor )
				{
					const APawn* Pawn = Cast<APawn>(Actor);

					if ( !Pawn )
					{
						Pawn = Cast<APawn>(Actor->GetOwner());
					}

					if ( Pawn && Pawn->IsLocallyControlled() )
					{
						Label = TEXT("LocalClient");
					}
					else
					{
						Label = TEXT("RemoteClient");
					}
				}
				else
				{
					Label = TEXT("Client");
				}
			}
			// Server
			else if ( NetMode == NM_ListenServer || NetMode == NM_DedicatedServer )
			{
				Label = TEXT("Server");
			}
			else
			{
				Label = TEXT("Standalone");
			}
		}
	}

	Label.InsertAt(0, TEXT("["));
	Label.Append(TEXT("]"));

	return FString::Printf(TEXT("%-13s "), *Label);
}