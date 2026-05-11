// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/GameMode/PX_MainGameMode.h"
#include "Entity/PX_Character.h"
#include "Framework/HUD/PX_HUD.h"
#include "Framework/Controller/PX_PlayerController.h"
#include "Framework/PlayerState/PX_PlayerState.h"
#include "Project_X.h"

APX_MainGameMode::APX_MainGameMode()
{
    DefaultPawnClass = APX_Character::StaticClass();
    HUDClass = APX_HUD::StaticClass();
    PlayerControllerClass = APX_PlayerController::StaticClass();
    PlayerStateClass = APX_PlayerState::StaticClass();
}

UClass* APX_MainGameMode::GetDefaultPawnClassForController_Implementation(AController* InController)
{
    if ( DefaultPawnClassOverride )
    {
        PX_LOG(Log, TEXT("DefaultPawnClassOverride selected. GameMode: %s, Controller: %s, PawnClass: %s"),
            *GetNameSafe(GetClass()),
            *GetNameSafe(InController),
            *GetNameSafe(DefaultPawnClassOverride.Get()));
        return DefaultPawnClassOverride;
    }

    UClass* SelectedPawnClass = Super::GetDefaultPawnClassForController_Implementation(InController);
    PX_LOG(Log, TEXT("DefaultPawnClass selected. GameMode: %s, Controller: %s, PawnClass: %s"),
        *GetNameSafe(GetClass()),
        *GetNameSafe(InController),
        *GetNameSafe(SelectedPawnClass));
    return SelectedPawnClass;
}
