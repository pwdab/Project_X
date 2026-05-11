// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Project_X.h"
#include "GameplayTagContainer.h"
#include "AbilitySystem/Tags/PX_GameplayTags.h"
#include "Blueprint/UserWidget.h"
#include "TimerManager.h"
#include "PX_HUDWidget.generated.h"

class UPX_InventoryComponent;
class UPX_WeaponSystemComponent;
class UPX_CurrentWeaponWidget;
class UPX_SkillIconWidget;
class UPX_StaminaGaugeWidget;
class UPX_TargetHealthWidget;
class UPX_TargetStatusComponent;
class UPX_WeaponListsWidget;
class UAbilitySystemComponent;
class APawn;
class UTexture2D;
//enum class EPXInventorySlotTarget : uint8;
struct FPXInventorySlot;
struct FPXCurrentWeaponHUDData;
struct FOnAttributeChangeData;
//enum class EPXWeaponAttackMode : uint8;

struct FPXStatusGaugeRuntime
{
    float CurrentPercent = 1.0f;
    float LostPercent = 1.0f;
    float LostStartPercent = 1.0f;
    float LostTargetPercent = 1.0f;
    float LostFollowElapsed = 0.0f;
    bool bLostFollowing = false;
    bool bInitialized = false;
};

/**
 * 
 */
UCLASS()
class PROJECT_X_API UPX_HUDWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
    // --- Blueprint Functions -----------------------------------------------------
    // Pawn을 주입하고 바인딩
    UFUNCTION(BlueprintCallable, Category = "PX_InventoryWidget")
    void BindHUD(APawn* InPawn);
    // HUD 바인딩 해제
    UFUNCTION(BlueprintCallable, Category = "PX_InventoryWidget")
    void UnbindHUD();
    // HUD 새로고침
    UFUNCTION(BlueprintCallable, Category = "PX_InventoryWidget")
    void RefreshHUD();
    UFUNCTION(BlueprintCallable, Category = "PX_InventoryWidget")
    void ApplyPredictedStaminaCost(float StaminaCost);

protected:
    virtual void NativeOnInitialized() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
    virtual void NativeDestruct() override;

private:
    // HUD 초기화
    void InitHUD();

    // --- Handler Functions -----------------------------------------------------
    void HandleInventoryReady();
    void HandleCurrentWeaponChanged(int32 NewSlot);
    void HandleAmmoChanged(int32 InAmmoInMag, int32 InReserved);
    //void HandleAttackModeChanged(EPXWeaponAttackMode InAttackMode);
    void HandleAttackModeChanged(FGameplayTag InAttackModeTag);
    //void HandleWeaponSlotChanged(EPXInventorySlotTarget Target, int32 SlotIndex, const FPXInventorySlot& InventorySlot);
    void HandleWeaponSlotChanged(FGameplayTag TargetTag, int32 SlotIndex, const FPXInventorySlot& InventorySlot);
    void HandleStatusAttributeChanged(const FOnAttributeChangeData& ChangeData);

    // --- Helper Functions -----------------------------------------------------
    bool BuildCurrentWeaponHUDDataFromSlot(int32 SlotIndex, FPXCurrentWeaponHUDData& OutData) const;
    void CacheStatusWidgets();
    void ConfigureStatusWidgets();
    void ConfigureStatusIconGauge(UUserWidget* IconGaugeWidget, UTexture2D* IconTexture, bool bOverrideForegroundColor, FLinearColor NewForegroundColor, bool bOverrideLostColor, FLinearColor LostColor);
    void RetryBindStatusAttributes();
    bool TryBindStatusAttributes();
    void BindStatusAttributes();
    void UnbindStatusAttributes();
    void RefreshStatusAttributes();
    void ClearPredictedStamina();
    void ReconcilePredictedStamina(float NewServerStamina, float NewServerMaxStamina);
    void UpdateTargetStatus(float DeltaTime);
    UPX_TargetStatusComponent* TraceLookedTargetStatus() const;
    UPX_TargetStatusComponent* ResolveTargetStatusFromActor(AActor* Actor) const;
    void SetDisplayedTargetStatus(UPX_TargetStatusComponent* NewTargetStatus);
    void ApplyVisibleTargetStatuses(const TArray<UPX_TargetStatusComponent*>& NewVisibleTargetStatuses);
    void BindDisplayedTargetStatusAttributes();
    void UnbindDisplayedTargetStatusAttributes();
    void HandleTargetStatusAttributeChanged(const FOnAttributeChangeData& ChangeData);
    void RefreshTargetStatusWidget();
    void RefreshSkillIconAvailability();
    void RefreshSkillIconExternalBlockedState();
    void UpdateStatusGauge(UUserWidget* IconGaugeWidget, float CurrentValue, float MaxValue);
    void TickStatusGauge(UUserWidget* IconGaugeWidget, FPXStatusGaugeRuntime& Runtime, float DeltaTime);
    void ApplyStatusGaugeWidths(UUserWidget* IconGaugeWidget, float ForegroundPercent, float LostPercent);
    float GetStatusGaugeFullWidth(UUserWidget* IconGaugeWidget, UUserWidget* GaugeWidget) const;

    // --- Child Widget Variables -----------------------------------------------------
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UUserWidget> BottomRightWidget;
    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UUserWidget> WBP_HUD_Status;
    UPROPERTY()
    TObjectPtr<UPX_CurrentWeaponWidget> CurrentWeaponWidget;
    UPROPERTY()
    TObjectPtr<UPX_WeaponListsWidget> WeaponListsWidget;
    UPROPERTY()
    TObjectPtr<UUserWidget> HealthGaugeWidget;
    UPROPERTY()
    TObjectPtr<UUserWidget> ShieldGaugeWidget;
    UPROPERTY()
    TObjectPtr<UUserWidget> HungerGaugeWidget;
    UPROPERTY()
    TObjectPtr<UUserWidget> WaterGaugeWidget;
    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UPX_StaminaGaugeWidget> WBP_HUD_Stamina;
    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UPX_TargetHealthWidget> WBP_HUD_TargetHealth;
    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UPX_SkillIconWidget> WBP_HUD_SkillIcon;

    // --- Variables -----------------------------------------------------
    // 현재 바인딩된 Pawn
    UPROPERTY(Transient)
    TObjectPtr<APawn> OwnerPawn;
    // 현재 바인딩된 InventoryComponent
    UPROPERTY(Transient)
    TObjectPtr<UPX_InventoryComponent> Inventory;
    // 현재 바인딩된 WeaponSystemComponent
    UPROPERTY(Transient)
    TObjectPtr<UPX_WeaponSystemComponent> WeaponSystem;
    UPROPERTY(Transient)
    TObjectPtr<UAbilitySystemComponent> ResourceASC;
    // 중복 바인딩 방지용
    UPROPERTY(Transient)
    bool bHUDBounded = false;
    UPROPERTY(Transient)
    bool bStatusAttributesBound = false;

    FPXStatusGaugeRuntime HealthGaugeRuntime;
    FPXStatusGaugeRuntime ShieldGaugeRuntime;
    FPXStatusGaugeRuntime HungerGaugeRuntime;
    FPXStatusGaugeRuntime WaterGaugeRuntime;

    float ServerStamina = 1.0f;
    float ServerMaxStamina = 1.0f;
    float PredictedStamina = 1.0f;
    float StaminaPredictionElapsed = 0.0f;
    bool bShowingPredictedStamina = false;

    UPROPERTY(EditAnywhere, Category = "Status|Gauge")
    float StatusGaugeLostFollowDuration = 0.5f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status|Prediction", meta = (AllowPrivateAccess = "true"))
    bool bUseClientPredictedStaminaUI = true;
    UPROPERTY(EditAnywhere, Category = "Status|Prediction")
    float StaminaPredictionMaxHoldTime = 0.75f;
    UPROPERTY(EditAnywhere, Category = "Status|Prediction")
    float StaminaPredictionServerTolerance = 0.25f;

    UPROPERTY(Transient)
    TObjectPtr<UPX_TargetStatusComponent> LookedTargetStatus;
    UPROPERTY(Transient)
    TObjectPtr<UPX_TargetStatusComponent> LastLookedTargetStatus;
    UPROPERTY(Transient)
    TObjectPtr<UPX_TargetStatusComponent> DisplayedTargetStatus;
    UPROPERTY(Transient)
    TObjectPtr<UAbilitySystemComponent> DisplayedTargetASC;
    UPROPERTY(Transient)
    TArray<TObjectPtr<UPX_TargetStatusComponent>> CombatTargetStatuses;
    UPROPERTY(Transient)
    TArray<TObjectPtr<UPX_TargetStatusComponent>> VisibleTargetStatuses;
    TMap<TWeakObjectPtr<UPX_TargetStatusComponent>, float> LastSeenTargetResourceValues;

    UPROPERTY(EditAnywhere, Category = "Target Status")
    float TargetTraceInterval = 0.05f;
    UPROPERTY(EditAnywhere, Category = "Target Status")
    float TargetTraceDistance = 2500.0f;
    UPROPERTY(EditAnywhere, Category = "Target Status")
    float TargetTraceRadius = 120.0f;
    UPROPERTY(EditAnywhere, Category = "Target Status")
    float TargetTraceStartOffset = 400.0f;
    UPROPERTY(EditAnywhere, Category = "Target Status|Debug")
    bool bDrawTargetTraceDebug = false;
    UPROPERTY(EditAnywhere, Category = "Target Status|Debug")
    float TargetTraceDebugDuration = 0.06f;
    UPROPERTY(EditAnywhere, Category = "Target Status")
    float LookedTargetVisibleGraceDuration = 3.0f;

    float TargetTraceElapsed = 0.0f;
    double LastLookedTargetSeenTime = -1000.0;

    FTimerHandle StatusBindRetryTimerHandle;
    FDelegateHandle HealthChangedHandle;
    FDelegateHandle MaxHealthChangedHandle;
    FDelegateHandle ShieldChangedHandle;
    FDelegateHandle MaxShieldChangedHandle;
    FDelegateHandle StaminaChangedHandle;
    FDelegateHandle MaxStaminaChangedHandle;
    FDelegateHandle TargetHealthChangedHandle;
    FDelegateHandle TargetMaxHealthChangedHandle;
    FDelegateHandle TargetShieldChangedHandle;


};
