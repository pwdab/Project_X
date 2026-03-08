// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Project_X.h"
#include "GameFramework/PlayerController.h"
#include "PX_PlayerController.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_X_API APX_PlayerController : public APlayerController
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;

public:
	void ToggleInventory();

private:
	void BindInventoryUI();
	void UnbindInventoryUI();

	void BindHUDUI();
	void UnbindHUDUI();

	bool bInventoryOpen = false;
	
};
