// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/HUD/PX_HUD.h"
#include "UObject/ConstructorHelpers.h"
//#include "Blueprint/UserWidget.h"
#include "UI/Inventory/PX_InventoryWidget.h"
#include "UI/HUD/PX_HUDWidget.h"

APX_HUD::APX_HUD()
{
    // Setup User Widget
    static ConstructorHelpers::FClassFinder<UPX_HUDWidget> HUDBPClass(TEXT("/Game/Project_X/UMG/HUD/WBP_HUD_Root"));
    if ( HUDBPClass.Succeeded() )
    {
        HUDWidgetClass = HUDBPClass.Class;
    }

    static ConstructorHelpers::FClassFinder<UPX_InventoryWidget> InventoryBPClass(TEXT("/Game/Project_X/UMG/UI/Inventory/WBP_UI_Inventory_Root"));
    if ( InventoryBPClass.Succeeded() )
    {
        InventoryWidgetClass = InventoryBPClass.Class;
    }
}

void APX_HUD::BeginPlay()
{
    Super::BeginPlay();
}

void APX_HUD::CreateHUDWidget(APlayerController* PlayerController)
{
    if ( !PlayerController || !PlayerController->IsLocalController() ) return;
    if ( HUDWidget || !HUDWidgetClass ) return;

    HUDWidget = CreateWidget<UPX_HUDWidget>(PlayerController, HUDWidgetClass);
    if ( HUDWidget ) HUDWidget->AddToViewport(0);
    PX_LOG(Log, TEXT(""));
}

void APX_HUD::ShowInventory(APlayerController* PlayerController)
{
    if ( !PlayerController || !PlayerController->IsLocalController() || !InventoryWidgetClass ) return;

    if ( !InventoryWidget ) InventoryWidget = CreateWidget<UPX_InventoryWidget>(PlayerController, InventoryWidgetClass);
    if ( InventoryWidget && !InventoryWidget->IsInViewport() ) InventoryWidget->AddToViewport(10);

    InventoryWidget->SetVisibility(ESlateVisibility::Visible);
}

void APX_HUD::HideInventory()
{
    /*
    if ( InventoryWidget )
        InventoryWidget->RemoveFromParent();
        */
    if ( InventoryWidget ) InventoryWidget->SetVisibility(ESlateVisibility::Collapsed);
}

bool APX_HUD::IsInventoryInViewport() const
{
    return InventoryWidget && InventoryWidget->IsInViewport();
}