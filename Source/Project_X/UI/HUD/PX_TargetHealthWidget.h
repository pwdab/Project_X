// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PX_TargetHealthWidget.generated.h"

class UProgressBar;
class USizeBox;
class UPX_TargetStatusComponent;
class UTextBlock;
class UWidget;

USTRUCT()
struct FPXTargetHealthGaugeRuntime
{
	GENERATED_BODY()

	float CurrentPercent = 1.0f;
	float LostPercent = 1.0f;
	float LostStartPercent = 1.0f;
	float LostTargetPercent = 1.0f;
	float LostFollowElapsed = 0.0f;
	bool bLostFollowing = false;
	bool bInitialized = false;
};

UCLASS()
class PROJECT_X_API UPX_TargetHealthWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Target Health")
	void SetTargetStatus(UPX_TargetStatusComponent* InTargetStatus);

	UFUNCTION(BlueprintCallable, Category = "Target Health")
	void ClearTargetStatus();

	UFUNCTION(BlueprintCallable, Category = "Target Health")
	void RefreshTargetStatus();

	UFUNCTION(BlueprintCallable, Category = "Target Health")
	FVector2D GetTargetHealthDrawSize();

protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

protected:
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UProgressBar> ProgressBar_TargetHealth;
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UUserWidget> WBP_HUD_Gauge;
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UUserWidget> WBP_HUD_ShieldGauge;
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UUserWidget> WBP_HUD_HealthGauge;
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<USizeBox> SizeBox_ShieldGauge;
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<USizeBox> SizeBox_HealthGauge;
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<USizeBox> SizeBox_TargetHealth;
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_TargetName;
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_TargetHealth;

private:
	void CacheGaugeWidgets();
	void UpdateGaugeRuntime(FPXTargetHealthGaugeRuntime& Runtime, UUserWidget* GaugeWidget, float NewPercent);
	void TickGaugeRuntime(FPXTargetHealthGaugeRuntime& Runtime, UUserWidget* GaugeWidget, float DeltaTime);
	void ApplyGaugePercents(UUserWidget* GaugeWidget, float ForegroundPercent, float LostPercent);
	void ApplyGaugeFullWidth(UUserWidget* GaugeWidget, float FullWidth);
	void ApplyOuterGaugeWidth(UUserWidget* GaugeWidget, float FullWidth);
	float GetGaugeFullWidth(UUserWidget* GaugeWidget);
	float GetTargetHealthFullWidth();
	float GetTargetHealthHeight() const;
	UWidget* FindGaugeWidgetByName(UUserWidget* GaugeWidget, const FName& WidgetName) const;

private:
	UPROPERTY(Transient)
	TObjectPtr<UPX_TargetStatusComponent> TargetStatus;

	UPROPERTY(EditAnywhere, Category = "Target Health|Gauge")
	float GaugeFullWidth = 180.0f;
	UPROPERTY(EditAnywhere, Category = "Target Health|Gauge")
	float LostFollowDuration = 0.5f;

	FPXTargetHealthGaugeRuntime ShieldGaugeRuntime;
	FPXTargetHealthGaugeRuntime HealthGaugeRuntime;
};
