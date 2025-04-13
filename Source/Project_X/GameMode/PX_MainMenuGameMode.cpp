// Fill out your copyright notice in the Description page of Project Settings.


#include "PX_MainMenuGameMode.h"
#include "../Entity/PX_Character.h"

APX_MainMenuGameMode::APX_MainMenuGameMode()
{
    DefaultPawnClass = APX_Character::StaticClass();
}


