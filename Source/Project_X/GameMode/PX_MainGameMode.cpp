// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/PX_MainGameMode.h"
#include "../Entity/PX_Character.h"

APX_MainGameMode::APX_MainGameMode()    // New
{
    DefaultPawnClass = APX_Character::StaticClass();    // New
}
