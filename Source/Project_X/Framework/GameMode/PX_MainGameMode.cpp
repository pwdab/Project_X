// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/GameMode/PX_MainGameMode.h"
#include "Entity/PX_Character.h"
#include "Framework/Controller/PX_PlayerController.h"
#include "Framework/HUD/PX_HUD.h"

APX_MainGameMode::APX_MainGameMode()
{
    DefaultPawnClass = APX_Character::StaticClass();
    //DefaultPawnClass = APX_CharacterClient::StaticClass();

    PlayerControllerClass = APX_PlayerController::StaticClass();
    HUDClass = APX_HUD::StaticClass();
}
