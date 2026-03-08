// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/Controller/PX_PlayerController.h"
#include "Framework/HUD/PX_HUD.h"
#include "Entity/PX_Character.h"
#include "Component/Inventory/PX_InventoryComponent.h"
#include "UI/Inventory/PX_InventoryWidget.h"
#include "UI/HUD/PX_HUDWidget.h"

void APX_PlayerController::BeginPlay()
{
	Super::BeginPlay();

    if ( !IsLocalController() ) return;

    if ( APX_HUD* PX_HUD = GetHUD<APX_HUD>() )
    {
        PX_LOG(Log, TEXT(""));
        PX_HUD->CreateHUDWidget(this);

        BindHUDUI();

        FInputModeGameOnly Mode;
        SetInputMode(Mode);
        bShowMouseCursor = false;
    }
}

void APX_PlayerController::ToggleInventory()
{
    if ( !IsLocalController() ) return;

    APX_HUD* PX_HUD = GetHUD<APX_HUD>();
    if ( !PX_HUD ) return;

    if ( !bInventoryOpen )
    {
        PX_LOG(Log, TEXT("Show Inventory."));
        PX_HUD->ShowInventory(this);

        BindInventoryUI();

        FInputModeGameAndUI Mode;
        SetInputMode(Mode);
        bShowMouseCursor = true;

        bInventoryOpen = true;
    }
    else
    {
        PX_LOG(Log, TEXT("Hide Inventory."));
        PX_HUD->HideInventory();

        FInputModeGameOnly Mode;
        SetInputMode(Mode);
        bShowMouseCursor = false;

        bInventoryOpen = false;
    }
}

void APX_PlayerController::BindInventoryUI()
{
    APX_HUD* PX_HUD = GetHUD<APX_HUD>();
    if ( !PX_HUD ) return;

    UPX_InventoryWidget* InventoryWidget = PX_HUD->GetInventoryWidget();
    if ( !InventoryWidget ) return;

    APX_Character* PXCharacter = Cast<APX_Character>(GetPawn());
    if ( !PXCharacter ) return;

    UPX_InventoryComponent* Inventory = PXCharacter->GetInventoryComponent();
    if ( !Inventory ) return;

    InventoryWidget->BindInventory(Inventory);
}

void APX_PlayerController::UnbindInventoryUI()
{
    APX_HUD* PX_HUD = GetHUD<APX_HUD>();
    if ( !PX_HUD ) return;

    UPX_InventoryWidget* InventoryWidget = PX_HUD->GetInventoryWidget();
    if ( !InventoryWidget ) return;

    InventoryWidget->UnbindInventory();
}

void APX_PlayerController::BindHUDUI()
{
    PX_LOG(Log, TEXT(""));
    APX_HUD* PX_HUD = GetHUD<APX_HUD>();
    if ( !PX_HUD ) return;

    UPX_HUDWidget* HUDWidget = PX_HUD->GetHUDWidget();
    if ( !HUDWidget ) return;

    APX_Character* PXCharacter = Cast<APX_Character>(GetPawn());
    if ( !PXCharacter ) return;

    HUDWidget->BindHUD(GetPawn());
}

void APX_PlayerController::UnbindHUDUI()
{
    APX_HUD* PX_HUD = GetHUD<APX_HUD>();
    if ( !PX_HUD ) return;

    UPX_HUDWidget* HUDWidget = PX_HUD->GetHUDWidget();
    if ( !HUDWidget ) return;

    HUDWidget->UnbindHUD();
}


