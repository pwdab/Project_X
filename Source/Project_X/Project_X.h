// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

DECLARE_LOG_CATEGORY_EXTERN(Project_X, Log, All);

#define ECC_PX_Combat ECC_GameTraceChannel1
#define ECC_PX_SkelMesh ECC_GameTraceChannel2
#define ECC_PX_CombatTrace ECC_GameTraceChannel3

#define PX_LOG_CALLINFO (FString(__FUNCTION__) + TEXT("(") + FString::FromInt(__LINE__) + TEXT(")"))
// 기본 카테고리(Project_X) 사용
#define PX_LOG(Verbosity, Format, ...) UE_LOG(Project_X, Verbosity, TEXT("%s %s"), *PX_LOG_CALLINFO, *FString::Printf(Format, ##__VA_ARGS__))
// 카테고리 지정
#define PX_LOGC(Category, Verbosity, Format, ...) UE_LOG(Category, Verbosity, TEXT("%s %s"), *PX_LOG_CALLINFO, *FString::Printf(Format, ##__VA_ARGS__))
