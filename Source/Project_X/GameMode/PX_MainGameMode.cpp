// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/PX_MainGameMode.h"
//#include "../Entity/PX_Character.h"
#include "../Entity/Client/PX_CharacterClient.h"

APX_MainGameMode::APX_MainGameMode()
{
    //DefaultPawnClass = APX_Character::StaticClass();
    DefaultPawnClass = APX_CharacterClient::StaticClass();
}
