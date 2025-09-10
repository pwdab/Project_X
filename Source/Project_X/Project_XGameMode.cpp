// Copyright Epic Games, Inc. All Rights Reserved.

#include "Project_XGameMode.h"
#include "Project_XCharacter.h"
#include "UObject/ConstructorHelpers.h"

AProject_XGameMode::AProject_XGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
