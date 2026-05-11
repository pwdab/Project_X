// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/Controller/PX_PlayerController.h"
#include "Framework/HUD/PX_HUD.h"
#include "Entity/PX_Character.h"
#include "Component/Inventory/PX_InventoryComponent.h"
#include "UI/Inventory/PX_InventoryWidget.h"
#include "UI/HUD/PX_HUDWidget.h"
#include "Framework/PlayerState/PX_PlayerState.h"
#include "AbilitySystem/Component/PX_AbilitySystemComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/GameViewportClient.h"
#include "InputMappingContext.h"
#include "UObject/ConstructorHelpers.h"

APX_PlayerController::APX_PlayerController()
{
    static ConstructorHelpers::FObjectFinder<UInputMappingContext> IMC_UI(TEXT("/Game/Project_X/Input/PX_IMC_UI.PX_IMC_UI"));
    if ( IMC_UI.Succeeded() )
    {
        InventoryMappingContext = IMC_UI.Object;
    }
}

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

void APX_PlayerController::PostProcessInput(const float DeltaTime, const bool bGamePaused)
{
    Super::PostProcessInput(DeltaTime, bGamePaused);

    if ( bInventoryOpen )
    {
        return;
    }

    UPX_AbilitySystemComponent* PX_AbilitySystemComponent = GetPXAbilitySystemComponent();
    if ( !PX_AbilitySystemComponent ) return;
    PX_AbilitySystemComponent->ProcessAbilityInput(DeltaTime, bGamePaused);
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
        ClearGameplayInputForInventory(true);
        ApplyInventoryInputContext();
        SetIgnoreMoveInput(true);

        FInputModeGameAndUI Mode;
        if ( UPX_InventoryWidget* InventoryWidget = PX_HUD->GetInventoryWidget() )
        {
            Mode.SetWidgetToFocus(InventoryWidget->TakeWidget());
        }
        Mode.SetHideCursorDuringCapture(false);
        Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
        SetInputMode(Mode);
        if ( UGameViewportClient* GameViewportClient = GetWorld()->GetGameViewport() )
        {
            GameViewportClient->SetMouseCaptureMode(EMouseCaptureMode::NoCapture);
            GameViewportClient->SetMouseLockMode(EMouseLockMode::DoNotLock);
            GameViewportClient->SetHideCursorDuringCapture(false);
        }
        bShowMouseCursor = true;
        bEnableClickEvents = true;
        bEnableMouseOverEvents = true;

        bInventoryOpen = true;
    }
    else
    {
        PX_LOG(Log, TEXT("Hide Inventory."));
        PX_HUD->HideInventory();
        ClearGameplayInputForInventory(false);
        ApplyGameplayInputContext();
        SetIgnoreMoveInput(false);

        FInputModeGameOnly Mode;
        SetInputMode(Mode);
        bShowMouseCursor = false;
        bEnableClickEvents = false;
        bEnableMouseOverEvents = false;

        bInventoryOpen = false;
    }
}

void APX_PlayerController::ApplyGameplayInputContext()
{
    if ( !IsLocalController() ) return;

    APX_Character* PXCharacter = Cast<APX_Character>(GetPawn());
    if ( !PXCharacter ) return;

    ULocalPlayer* LocalPlayer = GetLocalPlayer();
    if ( !LocalPlayer ) return;

    UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LocalPlayer);
    if ( !Subsystem ) return;

    if ( InventoryMappingContext )
    {
        Subsystem->RemoveMappingContext(InventoryMappingContext);
    }

    if ( UInputMappingContext* GameplayMappingContext = PXCharacter->GetDefaultMappingContext() )
    {
        Subsystem->AddMappingContext(GameplayMappingContext, 0);
    }
}

void APX_PlayerController::ApplyInventoryInputContext()
{
    if ( !IsLocalController() ) return;

    APX_Character* PXCharacter = Cast<APX_Character>(GetPawn());
    if ( !PXCharacter ) return;

    ULocalPlayer* LocalPlayer = GetLocalPlayer();
    if ( !LocalPlayer ) return;

    UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LocalPlayer);
    if ( !Subsystem ) return;

    if ( UInputMappingContext* GameplayMappingContext = PXCharacter->GetDefaultMappingContext() )
    {
        Subsystem->RemoveMappingContext(GameplayMappingContext);
    }

    if ( InventoryMappingContext )
    {
        Subsystem->AddMappingContext(InventoryMappingContext, 10);
    }
    else
    {
        PX_LOG(Warning, TEXT("Invalid InventoryMappingContext. Create /Game/Project_X/Input/PX_IMC_UI and map ToggleInventory to close inventory."));
    }
}

void APX_PlayerController::ClearGameplayInputForInventory(bool bBlockGameplayInput)
{
    if ( UPX_AbilitySystemComponent* PX_ASC = GetPXAbilitySystemComponent() )
    {
        PX_ASC->ClearAbilityInput();
    }

    if ( APX_Character* PXCharacter = Cast<APX_Character>(GetPawn()) )
    {
        PXCharacter->SetGameplayInputBlockedForUI(bBlockGameplayInput);
        PXCharacter->ClearGameplayInputStateForUI();
    }
}

UPX_AbilitySystemComponent* APX_PlayerController::GetPXAbilitySystemComponent() const
{
    APX_PlayerState* PX_PlayerState = GetPlayerState<APX_PlayerState>();
    if ( !PX_PlayerState ) return nullptr;

    return Cast<UPX_AbilitySystemComponent>(PX_PlayerState->GetAbilitySystemComponent());
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


