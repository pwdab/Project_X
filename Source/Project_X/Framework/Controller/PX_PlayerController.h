// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Project_X.h"
#include "GameFramework/PlayerController.h"
#include "PX_PlayerController.generated.h"

class UInputMappingContext;
class UPX_AbilitySystemComponent;

/**
 * 
 */
UCLASS()
class PROJECT_X_API APX_PlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	APX_PlayerController();

protected:
	virtual void BeginPlay() override;
	virtual void PostProcessInput(const float DeltaTime, const bool bGamePaused) override;

public:
	void ToggleInventory();

private:
	void ApplyGameplayInputContext();
	void ApplyInventoryInputContext();
	void ClearGameplayInputForInventory(bool bBlockGameplayInput);
	UPX_AbilitySystemComponent* GetPXAbilitySystemComponent() const;

	void BindInventoryUI();
	void UnbindInventoryUI();

	void BindHUDUI();
	void UnbindHUDUI();

	bool bInventoryOpen = false;

	UPROPERTY(EditDefaultsOnly, Category = "PX|Input")
	TObjectPtr<UInputMappingContext> InventoryMappingContext;
	
};
