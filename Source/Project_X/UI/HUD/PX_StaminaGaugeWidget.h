// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PX_StaminaGaugeWidget.generated.h"

class UAbilitySystemComponent;
class UImage;
class UMaterialInstanceDynamic;
struct FOnAttributeChangeData;

UCLASS()
class PROJECT_X_API UPX_StaminaGaugeWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Stamina")
	void BindToAbilitySystemComponent(UAbilitySystemComponent* InASC);
	UFUNCTION(BlueprintCallable, Category = "Stamina")
	void UnbindFromAbilitySystemComponent();

	UFUNCTION(BlueprintCallable, Category = "Stamina")
	void UpdateStamina(float CurrentStamina, float MaxStamina);
	UFUNCTION(BlueprintCallable, Category = "Stamina")
	void ApplyPredictedStaminaCost(float StaminaCost);
	bool GetDisplayedStamina(float& OutCurrentStamina, float& OutMaxStamina) const;

protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	virtual void NativeDestruct() override;

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Image_Stamina_FG;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Image_Stamina_Lost;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Image_Stamina_BG;

private:
	void SetMaterialPercent(UMaterialInstanceDynamic* MID, float Percent);
	void InitializeMaterialInstances();
	void ApplyGaugeVisibility(float Ratio);
	void TryBindToOwningPawnAbilitySystem();
	void RefreshStaminaFromAttributeSet();
	void HandleStaminaAttributeChanged(const FOnAttributeChangeData& ChangeData);
	void UpdateStaminaGauge(float CurrentStamina, float MaxStamina);
	void ClearPredictedStamina();

private:
	UPROPERTY(Transient)
	TObjectPtr<UAbilitySystemComponent> BoundASC;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> FG_MID;
	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> Lost_MID;
	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> BG_MID;

	// Duration for the delayed lost gauge to follow the foreground gauge.
	UPROPERTY(EditAnywhere, Category = "Stamina|Gauge")
	float LostFollowDuration = 0.5f;

	UPROPERTY(EditAnywhere, Category = "Stamina|Gauge")
	FName PercentParameterName = TEXT("Percent");
	UPROPERTY(EditAnywhere, Category = "Stamina|Gauge")
	TArray<FName> AdditionalPercentParameterNames = { TEXT("Progress"), TEXT("Value"), TEXT("Ratio") };
	UPROPERTY(EditAnywhere, Category = "Stamina|Gauge")
	FName ColorParameterName = TEXT("Color");
	UPROPERTY(EditAnywhere, Category = "Stamina|Gauge")
	FLinearColor GaugeForegroundColor = FLinearColor::White;
	UPROPERTY(EditAnywhere, Category = "Stamina|Gauge")
	FLinearColor GaugeLostColor = FLinearColor(1.0f, 0.74f, 0.18f, 0.75f);
	UPROPERTY(EditAnywhere, Category = "Stamina|Gauge")
	FLinearColor GaugeBackgroundColor = FLinearColor(0.02f, 0.025f, 0.035f, 0.45f);
	UPROPERTY(EditAnywhere, Category = "Stamina|Gauge")
	float FullStaminaHiddenTolerance = 0.001f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina|Prediction", meta = (AllowPrivateAccess = "true"))
	bool bUseClientPredictedStamina = true;
	UPROPERTY(EditAnywhere, Category = "Stamina|Prediction")
	float PredictionMaxHoldTime = 0.75f;
	UPROPERTY(EditAnywhere, Category = "Stamina|Prediction")
	float PredictionServerTolerance = 0.25f;

	float ServerStamina = 1.0f;
	float ServerMaxStamina = 1.0f;
	float PredictedStamina = 1.0f;
	float PredictionElapsed = 0.0f;
	bool bShowingPredictedStamina = false;

	float CurrentRatio = 1.0f;
	float LostRatio = 1.0f;
	float LostStartRatio = 1.0f;
	float LostTargetRatio = 1.0f;

	float LostFollowElapsed = 0.0f;
	bool bLostFollowing = false;

	FTimerHandle BindRetryTimerHandle;
	FDelegateHandle StaminaChangedHandle;
	FDelegateHandle MaxStaminaChangedHandle;
};
