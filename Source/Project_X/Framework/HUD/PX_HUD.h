// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Project_X.h"
#include "GameFramework/HUD.h"
#include "PX_HUD.generated.h"

//class UUserWidget;
class UPX_HUDWidget;
class UPX_InventoryWidget;

/**
 * 
 */
UCLASS()
class PROJECT_X_API APX_HUD : public AHUD
{
	GENERATED_BODY()

public:
	APX_HUD();

protected:
	virtual void BeginPlay() override;

public:
	void CreateHUDWidget(APlayerController* PlayerController);

	void ShowInventory(APlayerController* PlayerController);
	void HideInventory();

	bool IsInventoryInViewport() const;
	
	FORCEINLINE UPX_HUDWidget* GetHUDWidget() const { return HUDWidget.Get(); }
	FORCEINLINE UPX_InventoryWidget* GetInventoryWidget() const { return InventoryWidget.Get(); }
private:
	// --- UserWidget -----------------------------------------------------
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UPX_HUDWidget> HUDWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UPX_InventoryWidget> InventoryWidgetClass;

	UPROPERTY()
	TObjectPtr<UPX_HUDWidget> HUDWidget;

	UPROPERTY()
	TObjectPtr<UPX_InventoryWidget> InventoryWidget;
	
};
