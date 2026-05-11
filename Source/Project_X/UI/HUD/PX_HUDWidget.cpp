// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/HUD/PX_HUDWidget.h"
#include "AbilitySystem/Tags/PX_GameplayTags.h"
#include "Entity/PX_Character.h"
#include "Component/Inventory/PX_InventoryComponent.h"
#include "Component/Inventory/PX_WeaponItemInstance.h"
#include "Component/Weapon/PX_WeaponSystemComponent.h"
#include "Component/UI/PX_TargetStatusComponent.h"
#include "UI/HUD/PX_CurrentWeaponWidget.h"
#include "UI/HUD/PX_KeyIconWidget.h"
#include "UI/HUD/PX_SkillIconWidget.h"
#include "UI/HUD/PX_StaminaGaugeWidget.h"
#include "UI/HUD/PX_TargetHealthWidget.h"
#include "UI/HUD/PX_WeaponListsWidget.h"
#include "Component/Weapon/PX_WeaponDataAsset.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/Attributes/PX_ResourceAttributeSet.h"
#include "Components/Image.h"
#include "Components/SizeBox.h"
#include "Components/Widget.h"
#include "Camera/PlayerCameraManager.h"
#include "DrawDebugHelpers.h"
#include "Engine/Texture2D.h"
#include "GameFramework/PlayerController.h"
#include "Settings/PX_GameplayPredictionSettings.h"

namespace
{
    constexpr float PX_StatusGaugeFullWidth = 300.0f;
    const FLinearColor PX_ShieldGaugeForegroundColor(0.25f, 0.82f, 1.0f, 1.0f);
    const FLinearColor PX_ShieldGaugeLostColor(0.55f, 0.72f, 0.82f, 0.9f);
    const FLinearColor PX_HungerGaugeForegroundColor(0.86f, 0.56f, 0.22f, 1.0f);
    const FLinearColor PX_HungerGaugeLostColor(0.95f, 0.73f, 0.42f, 0.9f);
    const FLinearColor PX_WaterGaugeForegroundColor(0.18f, 0.62f, 0.95f, 1.0f);
    const FLinearColor PX_WaterGaugeLostColor(0.48f, 0.82f, 1.0f, 0.9f);

    static UTexture2D* PX_LoadHUDTexture(const TCHAR* TexturePath)
    {
        return LoadObject<UTexture2D>(nullptr, TexturePath);
    }

    static FText PX_AttackModeTagToText(const FGameplayTag& AttackModeTag)
    {
        if ( AttackModeTag == PX_GameplayTags::Weapon_AttackMode_Single ) return FText::FromString(TEXT("Single"));
        if ( AttackModeTag == PX_GameplayTags::Weapon_AttackMode_Burst ) return FText::FromString(TEXT("Burst"));
        if ( AttackModeTag == PX_GameplayTags::Weapon_AttackMode_Auto ) return FText::FromString(TEXT("Auto"));
        if ( AttackModeTag == PX_GameplayTags::Weapon_AttackMode_Charge ) return FText::FromString(TEXT("Charge"));
        if ( AttackModeTag == PX_GameplayTags::Weapon_AttackMode_Combo ) return FText::FromString(TEXT("Combo"));
        return AttackModeTag.IsValid() ? FText::FromString(AttackModeTag.GetTagName().ToString()) : FText::FromString(TEXT("None"));
    }

    static FText PX_AmmoTypeTagToText(const FGameplayTag& AmmoTypeTag)
    {
        return AmmoTypeTag.IsValid() ? FText::FromString(AmmoTypeTag.GetTagName().ToString()) : FText::FromString(TEXT("None"));
    }
}


void UPX_HUDWidget::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    //InitHUD();

    if ( BottomRightWidget )
    {
        CurrentWeaponWidget = Cast<UPX_CurrentWeaponWidget>(BottomRightWidget->GetWidgetFromName(TEXT("WBP_HUD_CurrentWeapon")));
        WeaponListsWidget = Cast<UPX_WeaponListsWidget>(BottomRightWidget->GetWidgetFromName(TEXT("WBP_HUD_WeaponLists")));
    }
    CacheStatusWidgets();
}

void UPX_HUDWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    TickStatusGauge(HealthGaugeWidget, HealthGaugeRuntime, InDeltaTime);
    TickStatusGauge(ShieldGaugeWidget, ShieldGaugeRuntime, InDeltaTime);
    TickStatusGauge(HungerGaugeWidget, HungerGaugeRuntime, InDeltaTime);
    TickStatusGauge(WaterGaugeWidget, WaterGaugeRuntime, InDeltaTime);
    const bool bShouldUsePredictedStaminaUI = UPX_GameplayPredictionSettings::IsClientPredictedStaminaUIEnabled() && bUseClientPredictedStaminaUI;
    if ( bShouldUsePredictedStaminaUI && bShowingPredictedStamina )
    {
        StaminaPredictionElapsed += InDeltaTime;
        if ( StaminaPredictionElapsed >= StaminaPredictionMaxHoldTime )
        {
            ClearPredictedStamina();
        }
    }
    else if ( !bShouldUsePredictedStaminaUI && bShowingPredictedStamina )
    {
        ClearPredictedStamina();
    }
    UpdateTargetStatus(InDeltaTime);
}

void UPX_HUDWidget::NativeDestruct()
{
    UnbindHUD();

    Super::NativeDestruct();
}

void UPX_HUDWidget::BindHUD(APawn* InPawn)
{
    //PX_LOG(Log, TEXT(""));
    if ( !InPawn ) return;

    // 중복 바인딩 처리
    if ( InPawn == OwnerPawn && bHUDBounded )
    {
        RefreshHUD();
        return;
    }

    // 기존 바인딩 해제
    UnbindHUD();

    // HUD 초기화
    OwnerPawn = InPawn;
    APX_Character* PX_Character = Cast<APX_Character>(OwnerPawn);
    if ( !PX_Character ) return;

    Inventory = PX_Character->GetInventoryComponent();
    WeaponSystem = PX_Character->GetWeaponSystemComponent();
    ResourceASC = PX_Character->GetAbilitySystemComponent();

    CacheStatusWidgets();
    InitHUD();

    if ( !Inventory || !WeaponSystem )
    {
        OwnerPawn = nullptr;
        Inventory = nullptr;
        WeaponSystem = nullptr;
        ResourceASC = nullptr;
        bHUDBounded = false;
        return;
    }

    // 델리게이트 바인딩
    Inventory->OnInventoryReady.AddUObject(this, &UPX_HUDWidget::HandleInventoryReady);
    Inventory->OnInventorySlotUpdated.AddUObject(this, &UPX_HUDWidget::HandleWeaponSlotChanged);
    WeaponSystem->OnCurrentWeaponUpdated.AddUObject(this, &UPX_HUDWidget::HandleCurrentWeaponChanged);
    WeaponSystem->OnCurrentAmmoUpdated.AddUObject(this, &UPX_HUDWidget::HandleAmmoChanged);
    WeaponSystem->OnAttackModeTagUpdated.AddUObject(this, &UPX_HUDWidget::HandleAttackModeChanged);
    // WeaponSystem->OnWeaponSlotsChanged.AddUObject(this, &UPX_HUDWidget::HandleWeaponSlotsChanged);
    bHUDBounded = true;
    if ( !TryBindStatusAttributes() )
    {
        if ( UWorld* World = GetWorld() )
        {
            World->GetTimerManager().SetTimer(StatusBindRetryTimerHandle, this, &UPX_HUDWidget::RetryBindStatusAttributes, 0.2f, true);
        }
    }

    // InventoryReady 처리
    if ( Inventory->IsInventoryReady() ) HandleInventoryReady();
    RefreshStatusAttributes();
}

void UPX_HUDWidget::UnbindHUD()
{
    if ( UWorld* World = GetWorld() )
    {
        World->GetTimerManager().ClearTimer(StatusBindRetryTimerHandle);
    }

    UnbindStatusAttributes();
    if ( WBP_HUD_Stamina )
    {
        WBP_HUD_Stamina->UnbindFromAbilitySystemComponent();
    }
    if ( WBP_HUD_SkillIcon )
    {
        WBP_HUD_SkillIcon->UnbindFromAbilitySystemComponent();
    }

    if ( Inventory && WeaponSystem && bHUDBounded )
    {
        // 델리게이트 해제

    }

    OwnerPawn = nullptr;
    Inventory = nullptr;
    WeaponSystem = nullptr;
    ResourceASC = nullptr;
    bHUDBounded = false;
    bStatusAttributesBound = false;
    HealthGaugeRuntime = FPXStatusGaugeRuntime{};
    ShieldGaugeRuntime = FPXStatusGaugeRuntime{};
    HungerGaugeRuntime = FPXStatusGaugeRuntime{};
    WaterGaugeRuntime = FPXStatusGaugeRuntime{};
    ClearPredictedStamina();
    LookedTargetStatus = nullptr;
    LastLookedTargetStatus = nullptr;
    CombatTargetStatuses.Reset();
    LastSeenTargetResourceValues.Reset();
    LastLookedTargetSeenTime = -1000.0;
    ApplyVisibleTargetStatuses(TArray<UPX_TargetStatusComponent*>());
    VisibleTargetStatuses.Reset();
    SetDisplayedTargetStatus(nullptr);
}

void UPX_HUDWidget::RefreshHUD()
{
    if ( !bHUDBounded || !OwnerPawn || !Inventory || !WeaponSystem )
    {
        return;
    }

    if ( !bStatusAttributesBound )
    {
        TryBindStatusAttributes();
    }
    else if ( WBP_HUD_Stamina && ResourceASC )
    {
        WBP_HUD_Stamina->BindToAbilitySystemComponent(ResourceASC);
    }
    if ( WBP_HUD_SkillIcon && ResourceASC )
    {
        WBP_HUD_SkillIcon->BindToAbilitySystemComponent(ResourceASC);
    }

    RefreshStatusAttributes();
    RefreshSkillIconAvailability();

    // “전체 UI를 현재 상태로 다시 그리는” 용도 (초기 1회/복구용)
    // 실제 데이터 Pull은 네 컴포넌트 API에 맞춰 작성해야 함.

    // (예시) 슬롯 변경/무기리스트/탄약을 각각 다시 갱신하는 흐름
    //HandleWeaponSlotChanged();

    // 현재 슬롯 인덱스가 있다면 같이 갱신
    // TODO: WeaponSystem의 현재 슬롯 인덱스 getter로 수정
    // int32 CurrentSlot = WeaponSystem->GetCurrentWeaponSlotIndex();
    // HandleCurrentWeaponChanged(CurrentSlot);
}

void UPX_HUDWidget::ApplyPredictedStaminaCost(float StaminaCost)
{
    if ( !UPX_GameplayPredictionSettings::IsClientPredictedStaminaUIEnabled() || !bUseClientPredictedStaminaUI )
    {
        ClearPredictedStamina();
        return;
    }

    if ( StaminaCost <= 0.0f )
    {
        return;
    }

    if ( ResourceASC )
    {
        ServerMaxStamina = FMath::Max(ResourceASC->GetNumericAttribute(UPX_ResourceAttributeSet::GetMaxStaminaAttribute()), 0.0f);
        ServerStamina = FMath::Clamp(ResourceASC->GetNumericAttribute(UPX_ResourceAttributeSet::GetStaminaAttribute()), 0.0f, ServerMaxStamina);
    }

    const float PredictionBase = bShowingPredictedStamina ? PredictedStamina : ServerStamina;
    PredictedStamina = FMath::Clamp(PredictionBase - StaminaCost, 0.0f, ServerMaxStamina);
    StaminaPredictionElapsed = 0.0f;
    bShowingPredictedStamina = true;

    if ( WBP_HUD_Stamina )
    {
        WBP_HUD_Stamina->ApplyPredictedStaminaCost(StaminaCost);
    }
}

void UPX_HUDWidget::InitHUD()
{
    if ( CurrentWeaponWidget ) CurrentWeaponWidget->ClearWidget();
    if ( WeaponListsWidget ) WeaponListsWidget->ClearWidgets();
    RefreshStatusAttributes();
}

void UPX_HUDWidget::CacheStatusWidgets()
{
    UUserWidget* StatusWidget = WBP_HUD_Status.Get();
    if ( !StatusWidget )
    {
        StatusWidget = Cast<UUserWidget>(GetWidgetFromName(TEXT("WBP_HUD_Status")));
        WBP_HUD_Status = StatusWidget;
    }

    HealthGaugeWidget = StatusWidget ? Cast<UUserWidget>(StatusWidget->GetWidgetFromName(TEXT("WBP_HUD_IconGauge_1"))) : nullptr;
    ShieldGaugeWidget = StatusWidget ? Cast<UUserWidget>(StatusWidget->GetWidgetFromName(TEXT("WBP_HUD_IconGauge_2"))) : nullptr;
    HungerGaugeWidget = StatusWidget ? Cast<UUserWidget>(StatusWidget->GetWidgetFromName(TEXT("WBP_HUD_IconGauge_3"))) : nullptr;
    WaterGaugeWidget = StatusWidget ? Cast<UUserWidget>(StatusWidget->GetWidgetFromName(TEXT("WBP_HUD_IconGauge_4"))) : nullptr;

    if ( !WBP_HUD_Stamina )
    {
        WBP_HUD_Stamina = Cast<UPX_StaminaGaugeWidget>(GetWidgetFromName(TEXT("WBP_HUD_Stamina")));
    }
    if ( !WBP_HUD_TargetHealth )
    {
        WBP_HUD_TargetHealth = Cast<UPX_TargetHealthWidget>(GetWidgetFromName(TEXT("WBP_HUD_TargetHealth")));
    }
    if ( !WBP_HUD_SkillIcon )
    {
        if ( UUserWidget* SkillKeyIconWidget = Cast<UUserWidget>(GetWidgetFromName(TEXT("WBP_HUD_SkillKeyIcon"))) )
        {
            WBP_HUD_SkillIcon = Cast<UPX_SkillIconWidget>(SkillKeyIconWidget->GetWidgetFromName(TEXT("WBP_HUD_SkillIcon")));
            if ( WBP_HUD_SkillIcon )
            {
                UPX_KeyIconWidget* KeyIconWidget = Cast<UPX_KeyIconWidget>(SkillKeyIconWidget->GetWidgetFromName(TEXT("WBP_HUD_KeyIcon")));
                WBP_HUD_SkillIcon->SetKeyIconWidget(KeyIconWidget);
            }
        }
        if ( !WBP_HUD_SkillIcon )
        {
            WBP_HUD_SkillIcon = Cast<UPX_SkillIconWidget>(GetWidgetFromName(TEXT("WBP_HUD_SkillIcon")));
        }
        if ( !WBP_HUD_SkillIcon )
        {
            WBP_HUD_SkillIcon = Cast<UPX_SkillIconWidget>(GetWidgetFromName(TEXT("SkillIcon")));
        }
    }

    ConfigureStatusWidgets();

    PX_LOG(Log, TEXT("StatusWidget: %s, HealthGauge: %s, ShieldGauge: %s, StaminaGauge: %s, RadialStamina: %s, TargetHealth: %s, SkillIcon: %s"),
        *GetNameSafe(StatusWidget),
        *GetNameSafe(HealthGaugeWidget),
        *GetNameSafe(ShieldGaugeWidget),
        *GetNameSafe(HungerGaugeWidget),
        *GetNameSafe(WaterGaugeWidget),
        *GetNameSafe(WBP_HUD_Stamina),
        *GetNameSafe(WBP_HUD_TargetHealth),
        *GetNameSafe(WBP_HUD_SkillIcon));
}

void UPX_HUDWidget::ConfigureStatusWidgets()
{
    ConfigureStatusIconGauge(
        HealthGaugeWidget,
        PX_LoadHUDTexture(TEXT("/Game/Project_X/Images/Heart_Icon.Heart_Icon")),
        false,
        FLinearColor::White,
        false,
        FLinearColor::White);

    ConfigureStatusIconGauge(
        ShieldGaugeWidget,
        PX_LoadHUDTexture(TEXT("/Game/Project_X/Images/Shield_Icon.Shield_Icon")),
        true,
        PX_ShieldGaugeForegroundColor,
        true,
        PX_ShieldGaugeLostColor);

    ConfigureStatusIconGauge(
        HungerGaugeWidget,
        PX_LoadHUDTexture(TEXT("/Game/Project_X/Images/Bread_Icon.Bread_Icon")),
        true,
        PX_HungerGaugeForegroundColor,
        true,
        PX_HungerGaugeLostColor);

    ConfigureStatusIconGauge(
        WaterGaugeWidget,
        PX_LoadHUDTexture(TEXT("/Game/Project_X/Images/Water_Icon.Water_Icon")),
        true,
        PX_WaterGaugeForegroundColor,
        true,
        PX_WaterGaugeLostColor);
}

void UPX_HUDWidget::ConfigureStatusIconGauge(UUserWidget* IconGaugeWidget, UTexture2D* IconTexture, bool bOverrideForegroundColor, FLinearColor NewForegroundColor, bool bOverrideLostColor, FLinearColor LostColor)
{
    if ( !IconGaugeWidget )
    {
        return;
    }

    if ( IconTexture )
    {
        if ( UUserWidget* IconWidget = Cast<UUserWidget>(IconGaugeWidget->GetWidgetFromName(TEXT("WBP_HUD_Icon"))) )
        {
            if ( UImage* IconImage = Cast<UImage>(IconWidget->GetWidgetFromName(TEXT("Image_Icon"))) )
            {
                IconImage->SetBrushFromTexture(IconTexture, true);
            }
        }
    }

    UUserWidget* GaugeWidget = Cast<UUserWidget>(IconGaugeWidget->GetWidgetFromName(TEXT("WBP_HUD_Gauge")));
    if ( !GaugeWidget )
    {
        return;
    }

    if ( bOverrideForegroundColor )
    {
        if ( UImage* ForegroundImage = Cast<UImage>(GaugeWidget->GetWidgetFromName(TEXT("Image_Gauge_FG"))) )
        {
            ForegroundImage->SetColorAndOpacity(NewForegroundColor);
        }
    }

    if ( bOverrideLostColor )
    {
        if ( UImage* LostImage = Cast<UImage>(GaugeWidget->GetWidgetFromName(TEXT("Image_Gauge_Lost"))) )
        {
            LostImage->SetColorAndOpacity(LostColor);
        }
    }
}

void UPX_HUDWidget::RetryBindStatusAttributes()
{
    TryBindStatusAttributes();
}

bool UPX_HUDWidget::TryBindStatusAttributes()
{
    if ( bStatusAttributesBound )
    {
        return true;
    }

    if ( !ResourceASC )
    {
        if ( APX_Character* PX_Character = Cast<APX_Character>(OwnerPawn) )
        {
            ResourceASC = PX_Character->GetAbilitySystemComponent();
        }
    }

    if ( !ResourceASC )
    {
        return false;
    }

    BindStatusAttributes();
    if ( WBP_HUD_Stamina )
    {
        WBP_HUD_Stamina->BindToAbilitySystemComponent(ResourceASC);
    }
    if ( WBP_HUD_SkillIcon )
    {
        WBP_HUD_SkillIcon->BindToAbilitySystemComponent(ResourceASC);
    }
    RefreshStatusAttributes();

    if ( bStatusAttributesBound )
    {
        if ( UWorld* World = GetWorld() )
        {
            World->GetTimerManager().ClearTimer(StatusBindRetryTimerHandle);
        }
    }

    return bStatusAttributesBound;
}

void UPX_HUDWidget::BindStatusAttributes()
{
    if ( !ResourceASC || bStatusAttributesBound )
    {
        return;
    }

    HealthChangedHandle = ResourceASC->GetGameplayAttributeValueChangeDelegate(UPX_ResourceAttributeSet::GetHealthAttribute()).AddUObject(this, &UPX_HUDWidget::HandleStatusAttributeChanged);
    MaxHealthChangedHandle = ResourceASC->GetGameplayAttributeValueChangeDelegate(UPX_ResourceAttributeSet::GetMaxHealthAttribute()).AddUObject(this, &UPX_HUDWidget::HandleStatusAttributeChanged);
    ShieldChangedHandle = ResourceASC->GetGameplayAttributeValueChangeDelegate(UPX_ResourceAttributeSet::GetShieldAttribute()).AddUObject(this, &UPX_HUDWidget::HandleStatusAttributeChanged);
    MaxShieldChangedHandle = ResourceASC->GetGameplayAttributeValueChangeDelegate(UPX_ResourceAttributeSet::GetMaxShieldAttribute()).AddUObject(this, &UPX_HUDWidget::HandleStatusAttributeChanged);
    StaminaChangedHandle = ResourceASC->GetGameplayAttributeValueChangeDelegate(UPX_ResourceAttributeSet::GetStaminaAttribute()).AddUObject(this, &UPX_HUDWidget::HandleStatusAttributeChanged);
    MaxStaminaChangedHandle = ResourceASC->GetGameplayAttributeValueChangeDelegate(UPX_ResourceAttributeSet::GetMaxStaminaAttribute()).AddUObject(this, &UPX_HUDWidget::HandleStatusAttributeChanged);
    bStatusAttributesBound = true;

    PX_LOG(Log, TEXT("Bound HUD status attributes. ASC: %s"), *GetNameSafe(ResourceASC));
}

void UPX_HUDWidget::UnbindStatusAttributes()
{
    if ( !ResourceASC || !bStatusAttributesBound )
    {
        return;
    }

    ResourceASC->GetGameplayAttributeValueChangeDelegate(UPX_ResourceAttributeSet::GetHealthAttribute()).Remove(HealthChangedHandle);
    ResourceASC->GetGameplayAttributeValueChangeDelegate(UPX_ResourceAttributeSet::GetMaxHealthAttribute()).Remove(MaxHealthChangedHandle);
    ResourceASC->GetGameplayAttributeValueChangeDelegate(UPX_ResourceAttributeSet::GetShieldAttribute()).Remove(ShieldChangedHandle);
    ResourceASC->GetGameplayAttributeValueChangeDelegate(UPX_ResourceAttributeSet::GetMaxShieldAttribute()).Remove(MaxShieldChangedHandle);
    ResourceASC->GetGameplayAttributeValueChangeDelegate(UPX_ResourceAttributeSet::GetStaminaAttribute()).Remove(StaminaChangedHandle);
    ResourceASC->GetGameplayAttributeValueChangeDelegate(UPX_ResourceAttributeSet::GetMaxStaminaAttribute()).Remove(MaxStaminaChangedHandle);

    if ( WBP_HUD_Stamina )
    {
        WBP_HUD_Stamina->UnbindFromAbilitySystemComponent();
    }

    HealthChangedHandle.Reset();
    MaxHealthChangedHandle.Reset();
    ShieldChangedHandle.Reset();
    MaxShieldChangedHandle.Reset();
    StaminaChangedHandle.Reset();
    MaxStaminaChangedHandle.Reset();
    bStatusAttributesBound = false;
}

void UPX_HUDWidget::HandleStatusAttributeChanged(const FOnAttributeChangeData& ChangeData)
{
    RefreshStatusAttributes();
}

void UPX_HUDWidget::RefreshStatusAttributes()
{
    if ( !ResourceASC )
    {
        UpdateStatusGauge(HealthGaugeWidget, 0.0f, 1.0f);
        UpdateStatusGauge(ShieldGaugeWidget, 0.0f, 1.0f);
        UpdateStatusGauge(HungerGaugeWidget, 1.0f, 1.0f);
        UpdateStatusGauge(WaterGaugeWidget, 1.0f, 1.0f);
        ClearPredictedStamina();
        if ( WBP_HUD_Stamina )
        {
            WBP_HUD_Stamina->UpdateStamina(0.0f, 1.0f);
        }
        return;
    }

    UpdateStatusGauge(HealthGaugeWidget,
        ResourceASC->GetNumericAttribute(UPX_ResourceAttributeSet::GetHealthAttribute()),
        ResourceASC->GetNumericAttribute(UPX_ResourceAttributeSet::GetMaxHealthAttribute()));
    UpdateStatusGauge(ShieldGaugeWidget,
        ResourceASC->GetNumericAttribute(UPX_ResourceAttributeSet::GetShieldAttribute()),
        ResourceASC->GetNumericAttribute(UPX_ResourceAttributeSet::GetMaxShieldAttribute()));
    ReconcilePredictedStamina(
        ResourceASC->GetNumericAttribute(UPX_ResourceAttributeSet::GetStaminaAttribute()),
        ResourceASC->GetNumericAttribute(UPX_ResourceAttributeSet::GetMaxStaminaAttribute()));
    UpdateStatusGauge(HungerGaugeWidget, 1.0f, 1.0f);
    UpdateStatusGauge(WaterGaugeWidget, 1.0f, 1.0f);
    if ( WBP_HUD_Stamina )
    {
        WBP_HUD_Stamina->UpdateStamina(
            ResourceASC->GetNumericAttribute(UPX_ResourceAttributeSet::GetStaminaAttribute()),
            ResourceASC->GetNumericAttribute(UPX_ResourceAttributeSet::GetMaxStaminaAttribute()));
    }

    PX_LOG(Log, TEXT("HUD Status: Health %.1f / %.1f, Shield %.1f / %.1f, Stamina %.1f / %.1f"),
        ResourceASC->GetNumericAttribute(UPX_ResourceAttributeSet::GetHealthAttribute()),
        ResourceASC->GetNumericAttribute(UPX_ResourceAttributeSet::GetMaxHealthAttribute()),
        ResourceASC->GetNumericAttribute(UPX_ResourceAttributeSet::GetShieldAttribute()),
        ResourceASC->GetNumericAttribute(UPX_ResourceAttributeSet::GetMaxShieldAttribute()),
        ResourceASC->GetNumericAttribute(UPX_ResourceAttributeSet::GetStaminaAttribute()),
        ResourceASC->GetNumericAttribute(UPX_ResourceAttributeSet::GetMaxStaminaAttribute()));
}

void UPX_HUDWidget::ClearPredictedStamina()
{
    if ( !bShowingPredictedStamina )
    {
        return;
    }

    bShowingPredictedStamina = false;
    StaminaPredictionElapsed = 0.0f;
    PredictedStamina = ServerStamina;
}

void UPX_HUDWidget::ReconcilePredictedStamina(float NewServerStamina, float NewServerMaxStamina)
{
    ServerMaxStamina = FMath::Max(NewServerMaxStamina, 0.0f);
    ServerStamina = FMath::Clamp(NewServerStamina, 0.0f, ServerMaxStamina);

    if ( !UPX_GameplayPredictionSettings::IsClientPredictedStaminaUIEnabled() || !bUseClientPredictedStaminaUI )
    {
        bShowingPredictedStamina = false;
        StaminaPredictionElapsed = 0.0f;
        PredictedStamina = ServerStamina;
        return;
    }

    if ( !bShowingPredictedStamina )
    {
        PredictedStamina = ServerStamina;
        return;
    }

    const bool bServerCaughtPrediction = ServerStamina <= PredictedStamina + StaminaPredictionServerTolerance;
    const bool bServerRejectedPrediction = ServerStamina >= ServerMaxStamina - StaminaPredictionServerTolerance;
    if ( bServerCaughtPrediction || bServerRejectedPrediction )
    {
        bShowingPredictedStamina = false;
        StaminaPredictionElapsed = 0.0f;
        PredictedStamina = ServerStamina;
    }
}

void UPX_HUDWidget::UpdateTargetStatus(float DeltaTime)
{
    if ( !OwnerPawn )
    {
        SetDisplayedTargetStatus(nullptr);
        return;
    }

    TargetTraceElapsed += DeltaTime;
    if ( TargetTraceElapsed >= TargetTraceInterval )
    {
        TargetTraceElapsed = 0.0f;
        if ( UPX_TargetStatusComponent* TracedTargetStatus = TraceLookedTargetStatus() )
        {
            LookedTargetStatus = TracedTargetStatus;
            LastLookedTargetStatus = TracedTargetStatus;
            LastLookedTargetSeenTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0;
        }
        else
        {
            const double CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0;
            const bool bWithinLookGrace = LastLookedTargetStatus && CurrentTime - LastLookedTargetSeenTime <= LookedTargetVisibleGraceDuration;
            LookedTargetStatus = bWithinLookGrace ? LastLookedTargetStatus : nullptr;
        }
    }

    for ( int32 Index = CombatTargetStatuses.Num() - 1; Index >= 0; --Index )
    {
        UPX_TargetStatusComponent* TargetStatus = CombatTargetStatuses[Index];
        if ( !TargetStatus || !TargetStatus->IsInCombatWith(OwnerPawn) )
        {
            CombatTargetStatuses.RemoveAtSwap(Index);
        }
    }

    TArray<UPX_TargetStatusComponent*> NewVisibleTargetStatuses;
    if ( LookedTargetStatus )
    {
        NewVisibleTargetStatuses.AddUnique(LookedTargetStatus);

        const float CurrentResourceValue = LookedTargetStatus->GetHealth() + LookedTargetStatus->GetShield();
        if ( const float* PreviousResourceValue = LastSeenTargetResourceValues.Find(LookedTargetStatus) )
        {
            if ( CurrentResourceValue < *PreviousResourceValue - KINDA_SMALL_NUMBER )
            {
                LookedTargetStatus->NotifyCombatWith(OwnerPawn);
                CombatTargetStatuses.AddUnique(LookedTargetStatus);
            }
        }
        LastSeenTargetResourceValues.Add(LookedTargetStatus, CurrentResourceValue);
    }

    for ( UPX_TargetStatusComponent* CombatTargetStatus : CombatTargetStatuses )
    {
        if ( CombatTargetStatus && CombatTargetStatus->IsInCombatWith(OwnerPawn) )
        {
            NewVisibleTargetStatuses.AddUnique(CombatTargetStatus);
        }
    }

    ApplyVisibleTargetStatuses(NewVisibleTargetStatuses);
    SetDisplayedTargetStatus(nullptr);
}

UPX_TargetStatusComponent* UPX_HUDWidget::TraceLookedTargetStatus() const
{
    const APlayerController* PlayerController = GetOwningPlayer();
    if ( !PlayerController || !PlayerController->PlayerCameraManager )
    {
        return nullptr;
    }

    const FVector TraceDirection = PlayerController->PlayerCameraManager->GetCameraRotation().Vector().GetSafeNormal();
    const FVector CameraLocation = PlayerController->PlayerCameraManager->GetCameraLocation();
    const FVector TraceStart = CameraLocation + TraceDirection * FMath::Max(TargetTraceStartOffset, 0.0f);
    const FVector TraceEnd = TraceStart + TraceDirection * TargetTraceDistance;

    if ( bDrawTargetTraceDebug && GetWorld() )
    {
        constexpr int32 DebugSegments = 16;
        const FColor DebugColor = FColor::Magenta;
        const FVector CapsuleCenter = (TraceStart + TraceEnd) * 0.5f;
        const float CapsuleHalfHeight = TargetTraceDistance * 0.5f + TargetTraceRadius;
        const FQuat CapsuleRotation = FRotationMatrix::MakeFromZ(TraceDirection).ToQuat();
        DrawDebugCapsule(GetWorld(), CapsuleCenter, CapsuleHalfHeight, TargetTraceRadius, CapsuleRotation, DebugColor, false, TargetTraceDebugDuration, 0, 1.5f);
    }

    FCollisionQueryParams Params(SCENE_QUERY_STAT(PXTargetStatusTrace), false);
    if ( OwnerPawn )
    {
        Params.AddIgnoredActor(OwnerPawn);
    }

    UWorld* World = GetWorld();
    if ( !World )
    {
        return nullptr;
    }

    const FCollisionShape TraceShape = FCollisionShape::MakeSphere(FMath::Max(TargetTraceRadius, 1.0f));

    TArray<FHitResult> Hits;
    World->SweepMultiByChannel(Hits, TraceStart, TraceEnd, FQuat::Identity, ECC_PX_CombatTrace, TraceShape, Params);

    FCollisionObjectQueryParams ObjectParams;
    ObjectParams.AddObjectTypesToQuery(ECC_Pawn);
    ObjectParams.AddObjectTypesToQuery(ECC_PX_SkelMesh);
    World->SweepMultiByObjectType(Hits, TraceStart, TraceEnd, FQuat::Identity, ObjectParams, TraceShape, Params);

    UPX_TargetStatusComponent* BestTargetStatus = nullptr;
    float BestScore = TNumericLimits<float>::Max();

    for ( const FHitResult& Hit : Hits )
    {
        UPX_TargetStatusComponent* TargetStatus = ResolveTargetStatusFromActor(Hit.GetActor());
        if ( !TargetStatus && Hit.GetComponent() )
        {
            TargetStatus = ResolveTargetStatusFromActor(Hit.GetComponent()->GetOwner());
        }

        if ( !TargetStatus )
        {
            continue;
        }

        const FVector HitLocation = FVector(Hit.ImpactPoint);
        const FVector ToTarget = HitLocation - TraceStart;
        const float ForwardDistance = FVector::DotProduct(ToTarget, TraceDirection);
        const float ClampedForwardDistance = FMath::Clamp(ForwardDistance, 0.0f, TargetTraceDistance);
        const FVector ClosestPointOnTraceSegment = TraceStart + TraceDirection * ClampedForwardDistance;
        const float DistanceToCapsule = FVector::Dist(HitLocation, ClosestPointOnTraceSegment);
        if ( DistanceToCapsule > TargetTraceRadius + KINDA_SMALL_NUMBER )
        {
            continue;
        }

        const float Score = DistanceToCapsule * 10.0f + FMath::Max(ForwardDistance, 0.0f) * 0.01f;
        if ( Score < BestScore )
        {
            BestScore = Score;
            BestTargetStatus = TargetStatus;
        }
    }

    return BestTargetStatus;
}

UPX_TargetStatusComponent* UPX_HUDWidget::ResolveTargetStatusFromActor(AActor* Actor) const
{
    if ( !Actor || Actor == OwnerPawn )
    {
        return nullptr;
    }

    return Actor->FindComponentByClass<UPX_TargetStatusComponent>();
}

void UPX_HUDWidget::SetDisplayedTargetStatus(UPX_TargetStatusComponent* NewTargetStatus)
{
    if ( DisplayedTargetStatus == NewTargetStatus )
    {
        return;
    }

    UnbindDisplayedTargetStatusAttributes();
    DisplayedTargetStatus = NewTargetStatus;
    BindDisplayedTargetStatusAttributes();

    if ( WBP_HUD_TargetHealth )
    {
        if ( DisplayedTargetStatus )
        {
            WBP_HUD_TargetHealth->SetTargetStatus(DisplayedTargetStatus);
        }
        else
        {
            WBP_HUD_TargetHealth->ClearTargetStatus();
        }
    }
}

void UPX_HUDWidget::ApplyVisibleTargetStatuses(const TArray<UPX_TargetStatusComponent*>& NewVisibleTargetStatuses)
{
    for ( int32 Index = VisibleTargetStatuses.Num() - 1; Index >= 0; --Index )
    {
        UPX_TargetStatusComponent* PreviousTargetStatus = VisibleTargetStatuses[Index];
        if ( !PreviousTargetStatus || !NewVisibleTargetStatuses.Contains(PreviousTargetStatus) )
        {
            if ( PreviousTargetStatus )
            {
                PreviousTargetStatus->SetStatusWidgetVisible(false);
            }
            VisibleTargetStatuses.RemoveAtSwap(Index);
        }
    }

    for ( UPX_TargetStatusComponent* NewTargetStatus : NewVisibleTargetStatuses )
    {
        if ( !NewTargetStatus )
        {
            continue;
        }

        NewTargetStatus->SetStatusWidgetVisible(true);
        VisibleTargetStatuses.AddUnique(NewTargetStatus);
    }
}

void UPX_HUDWidget::BindDisplayedTargetStatusAttributes()
{
    DisplayedTargetASC = DisplayedTargetStatus ? DisplayedTargetStatus->GetTargetAbilitySystemComponent() : nullptr;
    if ( !DisplayedTargetASC )
    {
        return;
    }

    TargetHealthChangedHandle = DisplayedTargetASC->GetGameplayAttributeValueChangeDelegate(UPX_ResourceAttributeSet::GetHealthAttribute()).AddUObject(this, &UPX_HUDWidget::HandleTargetStatusAttributeChanged);
    TargetMaxHealthChangedHandle = DisplayedTargetASC->GetGameplayAttributeValueChangeDelegate(UPX_ResourceAttributeSet::GetMaxHealthAttribute()).AddUObject(this, &UPX_HUDWidget::HandleTargetStatusAttributeChanged);
    TargetShieldChangedHandle = DisplayedTargetASC->GetGameplayAttributeValueChangeDelegate(UPX_ResourceAttributeSet::GetShieldAttribute()).AddUObject(this, &UPX_HUDWidget::HandleTargetStatusAttributeChanged);
}

void UPX_HUDWidget::UnbindDisplayedTargetStatusAttributes()
{
    if ( DisplayedTargetASC )
    {
        if ( TargetHealthChangedHandle.IsValid() )
        {
            DisplayedTargetASC->GetGameplayAttributeValueChangeDelegate(UPX_ResourceAttributeSet::GetHealthAttribute()).Remove(TargetHealthChangedHandle);
        }
        if ( TargetMaxHealthChangedHandle.IsValid() )
        {
            DisplayedTargetASC->GetGameplayAttributeValueChangeDelegate(UPX_ResourceAttributeSet::GetMaxHealthAttribute()).Remove(TargetMaxHealthChangedHandle);
        }
        if ( TargetShieldChangedHandle.IsValid() )
        {
            DisplayedTargetASC->GetGameplayAttributeValueChangeDelegate(UPX_ResourceAttributeSet::GetShieldAttribute()).Remove(TargetShieldChangedHandle);
        }
    }

    TargetHealthChangedHandle.Reset();
    TargetMaxHealthChangedHandle.Reset();
    TargetShieldChangedHandle.Reset();
    DisplayedTargetASC = nullptr;
}

void UPX_HUDWidget::HandleTargetStatusAttributeChanged(const FOnAttributeChangeData& ChangeData)
{
    if ( DisplayedTargetStatus && OwnerPawn )
    {
        DisplayedTargetStatus->NotifyCombatWith(OwnerPawn);
        CombatTargetStatuses.AddUnique(DisplayedTargetStatus);
    }

    RefreshTargetStatusWidget();
}

void UPX_HUDWidget::RefreshTargetStatusWidget()
{
    if ( !WBP_HUD_TargetHealth )
    {
        return;
    }

    if ( DisplayedTargetStatus )
    {
        WBP_HUD_TargetHealth->RefreshTargetStatus();
    }
    else
    {
        WBP_HUD_TargetHealth->ClearTargetStatus();
    }
}

void UPX_HUDWidget::RefreshSkillIconAvailability()
{
    if ( WBP_HUD_SkillIcon )
    {
        WBP_HUD_SkillIcon->RefreshAbilityAvailability();
        RefreshSkillIconExternalBlockedState();
    }
}

void UPX_HUDWidget::RefreshSkillIconExternalBlockedState()
{
    if ( !WBP_HUD_SkillIcon )
    {
        return;
    }

    const bool bBlockedByAmmo = WeaponSystem && WeaponSystem->GetWeaponSlotIndex() != 4 && WeaponSystem->GetCurrentAmmoInMag() <= 0;
    WBP_HUD_SkillIcon->SetExternalBlocked(bBlockedByAmmo);
}

void UPX_HUDWidget::UpdateStatusGauge(UUserWidget* IconGaugeWidget, float CurrentValue, float MaxValue)
{
    if ( !IconGaugeWidget )
    {
        return;
    }

    const float Percent = MaxValue > KINDA_SMALL_NUMBER ? FMath::Clamp(CurrentValue / MaxValue, 0.0f, 1.0f) : 0.0f;

    FPXStatusGaugeRuntime* Runtime = nullptr;
    if ( IconGaugeWidget == HealthGaugeWidget )
    {
        Runtime = &HealthGaugeRuntime;
    }
    else if ( IconGaugeWidget == ShieldGaugeWidget )
    {
        Runtime = &ShieldGaugeRuntime;
    }
    else if ( IconGaugeWidget == HungerGaugeWidget )
    {
        Runtime = &HungerGaugeRuntime;
    }
    else if ( IconGaugeWidget == WaterGaugeWidget )
    {
        Runtime = &WaterGaugeRuntime;
    }

    if ( !Runtime )
    {
        ApplyStatusGaugeWidths(IconGaugeWidget, Percent, Percent);
        return;
    }

    if ( !Runtime->bInitialized )
    {
        Runtime->CurrentPercent = Percent;
        Runtime->LostPercent = Percent;
        Runtime->LostStartPercent = Percent;
        Runtime->LostTargetPercent = Percent;
        Runtime->LostFollowElapsed = 0.0f;
        Runtime->bLostFollowing = false;
        Runtime->bInitialized = true;
        ApplyStatusGaugeWidths(IconGaugeWidget, Runtime->CurrentPercent, Runtime->LostPercent);
        return;
    }

    if ( FMath::IsNearlyEqual(Runtime->CurrentPercent, Percent, KINDA_SMALL_NUMBER) )
    {
        ApplyStatusGaugeWidths(IconGaugeWidget, Runtime->CurrentPercent, Runtime->LostPercent);
        return;
    }

    Runtime->CurrentPercent = Percent;

    if ( Percent < Runtime->LostPercent )
    {
        Runtime->LostStartPercent = Runtime->LostPercent;
        Runtime->LostTargetPercent = Percent;
        Runtime->LostFollowElapsed = 0.0f;
        Runtime->bLostFollowing = true;
    }
    else
    {
        Runtime->LostPercent = Percent;
        Runtime->LostStartPercent = Percent;
        Runtime->LostTargetPercent = Percent;
        Runtime->LostFollowElapsed = 0.0f;
        Runtime->bLostFollowing = false;
    }

    ApplyStatusGaugeWidths(IconGaugeWidget, Runtime->CurrentPercent, Runtime->LostPercent);
}

void UPX_HUDWidget::TickStatusGauge(UUserWidget* IconGaugeWidget, FPXStatusGaugeRuntime& Runtime, float DeltaTime)
{
    if ( !IconGaugeWidget || !Runtime.bInitialized || !Runtime.bLostFollowing )
    {
        return;
    }

    Runtime.LostFollowElapsed += DeltaTime;
    const float Alpha = StatusGaugeLostFollowDuration > 0.0f ? FMath::Clamp(Runtime.LostFollowElapsed / StatusGaugeLostFollowDuration, 0.0f, 1.0f) : 1.0f;
    Runtime.LostPercent = FMath::Lerp(Runtime.LostStartPercent, Runtime.LostTargetPercent, Alpha);

    if ( Alpha >= 1.0f )
    {
        Runtime.LostPercent = Runtime.LostTargetPercent;
        Runtime.LostFollowElapsed = 0.0f;
        Runtime.bLostFollowing = false;
    }

    ApplyStatusGaugeWidths(IconGaugeWidget, Runtime.CurrentPercent, Runtime.LostPercent);
}

void UPX_HUDWidget::ApplyStatusGaugeWidths(UUserWidget* IconGaugeWidget, float ForegroundPercent, float LostPercent)
{
    if ( !IconGaugeWidget )
    {
        return;
    }

    UUserWidget* GaugeWidget = Cast<UUserWidget>(IconGaugeWidget->GetWidgetFromName(TEXT("WBP_HUD_Gauge")));
    if ( !GaugeWidget )
    {
        return;
    }

    const float FullWidth = GetStatusGaugeFullWidth(IconGaugeWidget, GaugeWidget);
    const float ClampedForegroundPercent = FMath::Clamp(ForegroundPercent, 0.0f, 1.0f);
    const float ClampedLostPercent = FMath::Max(FMath::Clamp(LostPercent, 0.0f, 1.0f), ClampedForegroundPercent);
    const float ForegroundWidth = FullWidth * ClampedForegroundPercent;
    const float LostWidth = FullWidth * ClampedLostPercent;

    if ( USizeBox* GaugeForegroundSizeBox = Cast<USizeBox>(GaugeWidget->GetWidgetFromName(TEXT("SizeBox_Gauge_FG"))) )
    {
        GaugeForegroundSizeBox->SetWidthOverride(ForegroundWidth);
    }

    if ( USizeBox* GaugeLostSizeBox = Cast<USizeBox>(GaugeWidget->GetWidgetFromName(TEXT("SizeBox_Gauge_Lost"))) )
    {
        GaugeLostSizeBox->SetWidthOverride(LostWidth);
    }
}

float UPX_HUDWidget::GetStatusGaugeFullWidth(UUserWidget* IconGaugeWidget, UUserWidget* GaugeWidget) const
{
    if ( !GaugeWidget )
    {
        return PX_StatusGaugeFullWidth;
    }

    const float GaugeDesiredWidth = GaugeWidget->GetDesiredSize().X;
    if ( GaugeDesiredWidth > KINDA_SMALL_NUMBER )
    {
        return GaugeDesiredWidth;
    }

    const float GaugeCachedWidth = GaugeWidget->GetCachedGeometry().GetLocalSize().X;
    if ( GaugeCachedWidth > KINDA_SMALL_NUMBER )
    {
        return GaugeCachedWidth;
    }

    if ( IconGaugeWidget )
    {
        if ( USizeBox* GaugeContainerSizeBox = Cast<USizeBox>(IconGaugeWidget->GetWidgetFromName(TEXT("SizeBox_Gauge"))) )
        {
            const float ContainerWidth = GaugeContainerSizeBox->GetDesiredSize().X;
            if ( ContainerWidth > KINDA_SMALL_NUMBER )
            {
                return ContainerWidth;
            }
        }
    }

    return PX_StatusGaugeFullWidth;
}

void UPX_HUDWidget::HandleInventoryReady()
{
    // Inventory 준비 완료 시점에 한 번 더 갱신하면 안전
    RefreshHUD();
}

void UPX_HUDWidget::HandleCurrentWeaponChanged(int32 NewSlot)
{
    RefreshSkillIconAvailability();
    RefreshSkillIconExternalBlockedState();

    if ( !bHUDBounded || !CurrentWeaponWidget || !WeaponListsWidget ) return;

    FPXCurrentWeaponHUDData Data;
    if ( BuildCurrentWeaponHUDDataFromSlot(NewSlot, Data) )
    {
        CurrentWeaponWidget->UpdateWidget(Data);
    }
    else
    {
        CurrentWeaponWidget->ClearWidget();
    }

    WeaponListsWidget->HighlightWidgets(NewSlot);
}

void UPX_HUDWidget::HandleAmmoChanged(int32 InAmmoInMag, int32 InReserved)
{
    if ( !bHUDBounded || !CurrentWeaponWidget ) return;

    CurrentWeaponWidget->UpdateAmmo(InAmmoInMag, InReserved);
    RefreshSkillIconExternalBlockedState();
}

//void UPX_HUDWidget::HandleAttackModeChanged(EPXWeaponAttackMode InAttackMode)
void UPX_HUDWidget::HandleAttackModeChanged(FGameplayTag InAttackModeTag)
{
    //PX_LOG(Log, TEXT(""));
    if ( !bHUDBounded || !CurrentWeaponWidget ) return;
    if ( WeaponSystem && WeaponSystem->GetWeaponSlotIndex() == 4 )
    {
        CurrentWeaponWidget->ClearWidget();
        return;
    }

    CurrentWeaponWidget->UpdateAttackMode(PX_AttackModeTagToText(InAttackModeTag));
}

//void UPX_HUDWidget::HandleWeaponSlotChanged(EPXInventorySlotTarget Target, int32 SlotIndex, const FPXInventorySlot& InventorySlot)
void UPX_HUDWidget::HandleWeaponSlotChanged(FGameplayTag Target, int32 SlotIndex, const FPXInventorySlot& InventorySlot)
{
    if ( Target != PX_GameplayTags::Item_Inventory_Weapon ) return;
    if ( !bHUDBounded || !WeaponListsWidget ) return;

    UTexture2D* Texture = nullptr;
    if ( InventorySlot.ItemInstance )
    {
        if (UPX_ItemDataAsset* ItemData = InventorySlot.ItemInstance->GetItemDataAsset() )
        {
            Texture = ItemData->Icon;
        }
    }

    if ( Texture )
    {
        WeaponListsWidget->UpdateWidget(SlotIndex, Texture);
    }
    else
    {
        WeaponListsWidget->ClearWidget(SlotIndex);
    }
}

bool UPX_HUDWidget::BuildCurrentWeaponHUDDataFromSlot(int32 SlotIndex, FPXCurrentWeaponHUDData& OutData) const
{
    if ( SlotIndex == 4 ) return false; // BareHand

    //PX_LOG(Log, TEXT(""));
    OutData = FPXCurrentWeaponHUDData{};
    OutData.SlotIndex = SlotIndex;

    UPX_WeaponItemInstance* WeaponItemInstance = Cast<UPX_WeaponItemInstance>(Inventory->GetWeaponItemInstanceBySlot(SlotIndex));
    if ( !WeaponItemInstance ) return false;

    const UPX_ItemDataAsset* ItemData = WeaponItemInstance->GetItemDataAsset();
    if ( !ItemData ) return false;

    const UPX_WeaponDataAsset* WeaponData = WeaponItemInstance->GetWeaponDataAsset();
    if ( !WeaponData ) return false;

    OutData.WeaponName = ItemData->ItemName;
    OutData.WeaponIcon = ItemData->Icon;
    OutData.AmmoType = PX_AmmoTypeTagToText(WeaponData->AmmoTypeTag);
    OutData.AttackMode = PX_AttackModeTagToText(WeaponItemInstance->GetAttackMode());
    OutData.AmmoInMag = WeaponItemInstance->GetAmmo();
    //OutData.Reserved = WeaponItemInstance->GetAmmoReserve();
    OutData.Reserved = 1000;

    return true;
}


