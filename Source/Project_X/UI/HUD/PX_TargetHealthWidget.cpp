// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/HUD/PX_TargetHealthWidget.h"

#include "Component/UI/PX_TargetStatusComponent.h"
#include "Components/ProgressBar.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Image.h"
#include "Components/OverlaySlot.h"
#include "Blueprint/WidgetTree.h"

void UPX_TargetHealthWidget::NativeConstruct()
{
	Super::NativeConstruct();

	CacheGaugeWidgets();
	SetVisibility(ESlateVisibility::Hidden);
}

void UPX_TargetHealthWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	TickGaugeRuntime(ShieldGaugeRuntime, WBP_HUD_ShieldGauge, InDeltaTime);
	TickGaugeRuntime(HealthGaugeRuntime, WBP_HUD_HealthGauge, InDeltaTime);
}

void UPX_TargetHealthWidget::SetTargetStatus(UPX_TargetStatusComponent* InTargetStatus)
{
	if ( TargetStatus == InTargetStatus )
	{
		RefreshTargetStatus();
		return;
	}

	TargetStatus = InTargetStatus;
	RefreshTargetStatus();
}

void UPX_TargetHealthWidget::ClearTargetStatus()
{
	TargetStatus = nullptr;
	SetVisibility(ESlateVisibility::Hidden);

	if ( ProgressBar_TargetHealth )
	{
		ProgressBar_TargetHealth->SetPercent(0.0f);
	}
	ShieldGaugeRuntime = FPXTargetHealthGaugeRuntime{};
	HealthGaugeRuntime = FPXTargetHealthGaugeRuntime{};
	ApplyGaugePercents(WBP_HUD_ShieldGauge, 0.0f, 0.0f);
	ApplyGaugePercents(WBP_HUD_HealthGauge, 0.0f, 0.0f);
	if ( Text_TargetName )
	{
		Text_TargetName->SetText(FText::GetEmpty());
	}
	if ( Text_TargetHealth )
	{
		Text_TargetHealth->SetText(FText::GetEmpty());
	}
}

void UPX_TargetHealthWidget::RefreshTargetStatus()
{
	if ( !TargetStatus )
	{
		ClearTargetStatus();
		return;
	}

	const float Health = TargetStatus->GetHealth();
	const float MaxHealth = TargetStatus->GetMaxHealth();
	const float Shield = TargetStatus->GetShield();
	const float MaxShield = TargetStatus->GetMaxShield();
	const float HealthPercent = MaxHealth > KINDA_SMALL_NUMBER ? FMath::Clamp(Health / MaxHealth, 0.0f, 1.0f) : 0.0f;
	const float ShieldPercent = MaxShield > KINDA_SMALL_NUMBER ? FMath::Clamp(Shield / MaxShield, 0.0f, 1.0f) : 0.0f;

	SetVisibility(ESlateVisibility::HitTestInvisible);

	if ( ProgressBar_TargetHealth )
	{
		ProgressBar_TargetHealth->SetPercent(HealthPercent);
	}

	CacheGaugeWidgets();
	const bool bHasShieldGauge = MaxShield > KINDA_SMALL_NUMBER;
	if ( WBP_HUD_ShieldGauge )
	{
		WBP_HUD_ShieldGauge->SetVisibility(bHasShieldGauge ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Hidden);
		UpdateGaugeRuntime(ShieldGaugeRuntime, WBP_HUD_ShieldGauge, ShieldPercent);
	}
	if ( WBP_HUD_HealthGauge )
	{
		WBP_HUD_HealthGauge->SetVisibility(ESlateVisibility::HitTestInvisible);
		UpdateGaugeRuntime(HealthGaugeRuntime, WBP_HUD_HealthGauge, HealthPercent);
	}
	if ( Text_TargetName )
	{
		Text_TargetName->SetText(TargetStatus->GetDisplayName());
	}
	if ( Text_TargetHealth )
	{
		Text_TargetHealth->SetText(FText::FromString(FString::Printf(TEXT("%.0f / %.0f"), Health, MaxHealth)));
	}
}

FVector2D UPX_TargetHealthWidget::GetTargetHealthDrawSize()
{
	return FVector2D(GetTargetHealthFullWidth(), GetTargetHealthHeight());
}

void UPX_TargetHealthWidget::UpdateGaugeRuntime(FPXTargetHealthGaugeRuntime& Runtime, UUserWidget* GaugeWidget, float NewPercent)
{
	const float ClampedPercent = FMath::Clamp(NewPercent, 0.0f, 1.0f);

	if ( !Runtime.bInitialized )
	{
		Runtime.CurrentPercent = ClampedPercent;
		Runtime.LostPercent = ClampedPercent;
		Runtime.LostStartPercent = ClampedPercent;
		Runtime.LostTargetPercent = ClampedPercent;
		Runtime.LostFollowElapsed = 0.0f;
		Runtime.bLostFollowing = false;
		Runtime.bInitialized = true;
		ApplyGaugePercents(GaugeWidget, Runtime.CurrentPercent, Runtime.LostPercent);
		return;
	}

	if ( FMath::IsNearlyEqual(Runtime.CurrentPercent, ClampedPercent, KINDA_SMALL_NUMBER) )
	{
		ApplyGaugePercents(GaugeWidget, Runtime.CurrentPercent, Runtime.LostPercent);
		return;
	}

	Runtime.CurrentPercent = ClampedPercent;

	if ( ClampedPercent < Runtime.LostPercent )
	{
		Runtime.LostStartPercent = Runtime.LostPercent;
		Runtime.LostTargetPercent = ClampedPercent;
		Runtime.LostFollowElapsed = 0.0f;
		Runtime.bLostFollowing = true;
	}
	else
	{
		Runtime.LostPercent = ClampedPercent;
		Runtime.LostStartPercent = ClampedPercent;
		Runtime.LostTargetPercent = ClampedPercent;
		Runtime.LostFollowElapsed = 0.0f;
		Runtime.bLostFollowing = false;
	}

	ApplyGaugePercents(GaugeWidget, Runtime.CurrentPercent, Runtime.LostPercent);
}

void UPX_TargetHealthWidget::TickGaugeRuntime(FPXTargetHealthGaugeRuntime& Runtime, UUserWidget* GaugeWidget, float DeltaTime)
{
	if ( !Runtime.bInitialized || !Runtime.bLostFollowing )
	{
		return;
	}

	Runtime.LostFollowElapsed += DeltaTime;
	const float Alpha = LostFollowDuration > 0.0f ? FMath::Clamp(Runtime.LostFollowElapsed / LostFollowDuration, 0.0f, 1.0f) : 1.0f;
	Runtime.LostPercent = FMath::Lerp(Runtime.LostStartPercent, Runtime.LostTargetPercent, Alpha);

	if ( Alpha >= 1.0f )
	{
		Runtime.LostPercent = Runtime.LostTargetPercent;
		Runtime.LostFollowElapsed = 0.0f;
		Runtime.bLostFollowing = false;
	}

	ApplyGaugePercents(GaugeWidget, Runtime.CurrentPercent, Runtime.LostPercent);
}

void UPX_TargetHealthWidget::CacheGaugeWidgets()
{
	if ( !SizeBox_TargetHealth )
	{
		SizeBox_TargetHealth = Cast<USizeBox>(GetWidgetFromName(TEXT("SizeBox_TargetHealth")));
	}
	if ( !SizeBox_TargetHealth && WidgetTree )
	{
		SizeBox_TargetHealth = Cast<USizeBox>(WidgetTree->FindWidget(TEXT("SizeBox_TargetHealth")));
	}
	if ( !SizeBox_ShieldGauge )
	{
		SizeBox_ShieldGauge = Cast<USizeBox>(GetWidgetFromName(TEXT("SizeBox_ShieldGauge")));
	}
	if ( !SizeBox_HealthGauge )
	{
		SizeBox_HealthGauge = Cast<USizeBox>(GetWidgetFromName(TEXT("SizeBox_HealthGauge")));
	}
	if ( !SizeBox_ShieldGauge && WidgetTree )
	{
		SizeBox_ShieldGauge = Cast<USizeBox>(WidgetTree->FindWidget(TEXT("SizeBox_ShieldGauge")));
	}
	if ( !SizeBox_HealthGauge && WidgetTree )
	{
		SizeBox_HealthGauge = Cast<USizeBox>(WidgetTree->FindWidget(TEXT("SizeBox_HealthGauge")));
	}

	if ( !WBP_HUD_ShieldGauge )
	{
		WBP_HUD_ShieldGauge = Cast<UUserWidget>(GetWidgetFromName(TEXT("WBP_HUD_ShieldGauge")));
	}
	if ( !WBP_HUD_HealthGauge )
	{
		WBP_HUD_HealthGauge = Cast<UUserWidget>(GetWidgetFromName(TEXT("WBP_HUD_HealthGauge")));
	}
	if ( !WBP_HUD_ShieldGauge )
	{
		WBP_HUD_ShieldGauge = Cast<UUserWidget>(GetWidgetFromName(TEXT("WBP_HUD_Gauge_1")));
	}
	if ( !WBP_HUD_HealthGauge )
	{
		WBP_HUD_HealthGauge = Cast<UUserWidget>(GetWidgetFromName(TEXT("WBP_HUD_Gauge_2")));
	}
	if ( !WBP_HUD_ShieldGauge )
	{
		WBP_HUD_ShieldGauge = Cast<UUserWidget>(GetWidgetFromName(TEXT("WBP_HUD_Gague_1")));
	}
	if ( !WBP_HUD_HealthGauge )
	{
		WBP_HUD_HealthGauge = Cast<UUserWidget>(GetWidgetFromName(TEXT("WBP_HUD_Gague_2")));
	}
	if ( !WBP_HUD_HealthGauge )
	{
		WBP_HUD_HealthGauge = WBP_HUD_Gauge;
	}
}

void UPX_TargetHealthWidget::ApplyGaugePercents(UUserWidget* GaugeWidget, float ForegroundPercent, float LostPercent)
{
	if ( !GaugeWidget )
	{
		return;
	}

	const float FullWidth = GetGaugeFullWidth(GaugeWidget);
	ApplyOuterGaugeWidth(GaugeWidget, FullWidth);
	ApplyGaugeFullWidth(GaugeWidget, FullWidth);
	const float ForegroundWidth = FullWidth * FMath::Clamp(ForegroundPercent, 0.0f, 1.0f);
	const float LostWidth = FullWidth * FMath::Max(FMath::Clamp(LostPercent, 0.0f, 1.0f), FMath::Clamp(ForegroundPercent, 0.0f, 1.0f));

	if ( USizeBox* GaugeForegroundSizeBox = Cast<USizeBox>(FindGaugeWidgetByName(GaugeWidget, TEXT("SizeBox_Gauge_FG"))) )
	{
		GaugeForegroundSizeBox->SetWidthOverride(ForegroundWidth);
	}

	if ( USizeBox* GaugeLostSizeBox = Cast<USizeBox>(FindGaugeWidgetByName(GaugeWidget, TEXT("SizeBox_Gauge_Lost"))) )
	{
		GaugeLostSizeBox->SetWidthOverride(LostWidth);
	}
}

void UPX_TargetHealthWidget::ApplyGaugeFullWidth(UUserWidget* GaugeWidget, float FullWidth)
{
	if ( !GaugeWidget || FullWidth <= KINDA_SMALL_NUMBER )
	{
		return;
	}

	if ( USizeBox* GaugeContainerSizeBox = Cast<USizeBox>(FindGaugeWidgetByName(GaugeWidget, TEXT("SizeBox_Gauge"))) )
	{
		GaugeContainerSizeBox->SetWidthOverride(FullWidth);
	}

	if ( UWidget* GaugeBackgroundImage = FindGaugeWidgetByName(GaugeWidget, TEXT("Image_Gauge_BG")) )
	{
		if ( UImage* BackgroundImage = Cast<UImage>(GaugeBackgroundImage) )
		{
			const FVector2D CurrentDesiredSize = BackgroundImage->GetDesiredSize();
			const FSlateBrush& BackgroundBrush = BackgroundImage->GetBrush();
			const float DesiredHeight = CurrentDesiredSize.Y > KINDA_SMALL_NUMBER ? CurrentDesiredSize.Y : BackgroundBrush.ImageSize.Y;
			BackgroundImage->SetDesiredSizeOverride(FVector2D(FullWidth, DesiredHeight));
		}

		if ( UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(GaugeBackgroundImage->Slot) )
		{
			const FVector2D CurrentSize = CanvasSlot->GetSize();
			CanvasSlot->SetSize(FVector2D(FullWidth, CurrentSize.Y));
		}
		else if ( UOverlaySlot* OverlaySlot = Cast<UOverlaySlot>(GaugeBackgroundImage->Slot) )
		{
			OverlaySlot->SetHorizontalAlignment(HAlign_Fill);
		}
	}
}

void UPX_TargetHealthWidget::ApplyOuterGaugeWidth(UUserWidget* GaugeWidget, float FullWidth)
{
	if ( !GaugeWidget || FullWidth <= KINDA_SMALL_NUMBER )
	{
		return;
	}

	if ( GaugeWidget == WBP_HUD_ShieldGauge && SizeBox_ShieldGauge )
	{
		SizeBox_ShieldGauge->SetWidthOverride(FullWidth);
	}
	else if ( GaugeWidget == WBP_HUD_HealthGauge && SizeBox_HealthGauge )
	{
		SizeBox_HealthGauge->SetWidthOverride(FullWidth);
	}
}

float UPX_TargetHealthWidget::GetGaugeFullWidth(UUserWidget* GaugeWidget)
{
	if ( !GaugeWidget )
	{
		return GaugeFullWidth;
	}

	const float TargetHealthFullWidth = GetTargetHealthFullWidth();
	if ( TargetHealthFullWidth > KINDA_SMALL_NUMBER )
	{
		return TargetHealthFullWidth;
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

	if ( USizeBox* GaugeContainerSizeBox = Cast<USizeBox>(FindGaugeWidgetByName(GaugeWidget, TEXT("SizeBox_Gauge"))) )
	{
		const float ContainerWidth = GaugeContainerSizeBox->GetDesiredSize().X;
		if ( ContainerWidth > KINDA_SMALL_NUMBER )
		{
			return ContainerWidth;
		}

		const float CachedContainerWidth = GaugeContainerSizeBox->GetCachedGeometry().GetLocalSize().X;
		if ( CachedContainerWidth > KINDA_SMALL_NUMBER )
		{
			return CachedContainerWidth;
		}
	}

	return GaugeFullWidth;
}

float UPX_TargetHealthWidget::GetTargetHealthFullWidth()
{
	CacheGaugeWidgets();

	if ( !SizeBox_TargetHealth )
	{
		return 0.0f;
	}

	if ( SizeBox_TargetHealth->IsWidthOverride() )
	{
		return SizeBox_TargetHealth->GetWidthOverride();
	}

	const float DesiredWidth = SizeBox_TargetHealth->GetDesiredSize().X;
	if ( DesiredWidth > KINDA_SMALL_NUMBER )
	{
		return DesiredWidth;
	}

	const float CachedWidth = SizeBox_TargetHealth->GetCachedGeometry().GetLocalSize().X;
	if ( CachedWidth > KINDA_SMALL_NUMBER )
	{
		return CachedWidth;
	}

	return 0.0f;
}

float UPX_TargetHealthWidget::GetTargetHealthHeight() const
{
	if ( SizeBox_TargetHealth )
	{
		const float DesiredHeight = SizeBox_TargetHealth->GetDesiredSize().Y;
		if ( DesiredHeight > KINDA_SMALL_NUMBER )
		{
			return DesiredHeight;
		}

		const float CachedHeight = SizeBox_TargetHealth->GetCachedGeometry().GetLocalSize().Y;
		if ( CachedHeight > KINDA_SMALL_NUMBER )
		{
			return CachedHeight;
		}
	}

	const float DesiredHeight = GetDesiredSize().Y;
	if ( DesiredHeight > KINDA_SMALL_NUMBER )
	{
		return DesiredHeight;
	}

	const float CachedHeight = GetCachedGeometry().GetLocalSize().Y;
	if ( CachedHeight > KINDA_SMALL_NUMBER )
	{
		return CachedHeight;
	}

	return 60.0f;
}

UWidget* UPX_TargetHealthWidget::FindGaugeWidgetByName(UUserWidget* GaugeWidget, const FName& WidgetName) const
{
	if ( !GaugeWidget )
	{
		return nullptr;
	}

	if ( UWidget* DirectWidget = GaugeWidget->GetWidgetFromName(WidgetName) )
	{
		return DirectWidget;
	}

	return GaugeWidget->WidgetTree ? GaugeWidget->WidgetTree->FindWidget(WidgetName) : nullptr;
}
