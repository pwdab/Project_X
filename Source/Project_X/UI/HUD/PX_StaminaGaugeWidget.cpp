// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/HUD/PX_StaminaGaugeWidget.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/Attributes/PX_ResourceAttributeSet.h"
#include "Components/Image.h"
#include "GameFramework/Pawn.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Settings/PX_GameplayPredictionSettings.h"
#include "TimerManager.h"

void UPX_StaminaGaugeWidget::NativeConstruct()
{
	Super::NativeConstruct();

	InitializeMaterialInstances();

	CurrentRatio = 1.0f;
	LostRatio = 1.0f;
	LostStartRatio = 1.0f;
	LostTargetRatio = 1.0f;
	LostFollowElapsed = 0.0f;
	bLostFollowing = false;
	ServerStamina = 1.0f;
	ServerMaxStamina = 1.0f;
	PredictedStamina = 1.0f;
	PredictionElapsed = 0.0f;
	bShowingPredictedStamina = false;

	SetMaterialPercent(BG_MID, 1.0f);
	SetMaterialPercent(FG_MID, 1.0f);
	SetMaterialPercent(Lost_MID, 1.0f);

	if ( BG_MID )
	{
		BG_MID->SetVectorParameterValue(ColorParameterName, GaugeBackgroundColor);
	}
	if ( Image_Stamina_BG )
	{
		Image_Stamina_BG->SetColorAndOpacity(GaugeBackgroundColor);
	}
	if ( Lost_MID )
	{
		Lost_MID->SetVectorParameterValue(ColorParameterName, GaugeLostColor);
	}
	if ( Image_Stamina_Lost )
	{
		Image_Stamina_Lost->SetColorAndOpacity(GaugeLostColor);
	}
	if ( FG_MID )
	{
		FG_MID->SetVectorParameterValue(ColorParameterName, GaugeForegroundColor);
	}
	if ( Image_Stamina_FG )
	{
		Image_Stamina_FG->SetColorAndOpacity(GaugeForegroundColor);
	}
	ApplyGaugeVisibility(CurrentRatio);

	TryBindToOwningPawnAbilitySystem();
}

void UPX_StaminaGaugeWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	const bool bShouldUsePredictedStamina = UPX_GameplayPredictionSettings::IsClientPredictedStaminaUIEnabled() && bUseClientPredictedStamina;
	if ( bShouldUsePredictedStamina && bShowingPredictedStamina )
	{
		PredictionElapsed += InDeltaTime;
		if ( PredictionElapsed >= PredictionMaxHoldTime )
		{
			ClearPredictedStamina();
		}
	}
	else if ( !bShouldUsePredictedStamina && bShowingPredictedStamina )
	{
		ClearPredictedStamina();
	}

	if ( !bLostFollowing )
	{
		return;
	}

	LostFollowElapsed += InDeltaTime;

	const float Alpha = LostFollowDuration > 0.0f ? FMath::Clamp(LostFollowElapsed / LostFollowDuration, 0.0f, 1.0f) : 1.0f;

	LostRatio = FMath::Lerp(LostStartRatio, LostTargetRatio, Alpha);
	SetMaterialPercent(Lost_MID, LostRatio);

	if ( Alpha >= 1.0f )
	{
		LostRatio = LostTargetRatio;
		SetMaterialPercent(Lost_MID, LostRatio);

		bLostFollowing = false;
		LostFollowElapsed = 0.0f;
	}
}

void UPX_StaminaGaugeWidget::NativeDestruct()
{
	if ( UWorld* World = GetWorld() )
	{
		World->GetTimerManager().ClearTimer(BindRetryTimerHandle);
	}

	UnbindFromAbilitySystemComponent();

	Super::NativeDestruct();
}

void UPX_StaminaGaugeWidget::BindToAbilitySystemComponent(UAbilitySystemComponent* InASC)
{
	if ( BoundASC == InASC )
	{
		RefreshStaminaFromAttributeSet();
		return;
	}

	UnbindFromAbilitySystemComponent();

	BoundASC = InASC;
	if ( !BoundASC )
	{
		ClearPredictedStamina();
		UpdateStamina(0.0f, 1.0f);
		return;
	}

	StaminaChangedHandle = BoundASC->GetGameplayAttributeValueChangeDelegate(UPX_ResourceAttributeSet::GetStaminaAttribute()).AddUObject(this, &UPX_StaminaGaugeWidget::HandleStaminaAttributeChanged);
	MaxStaminaChangedHandle = BoundASC->GetGameplayAttributeValueChangeDelegate(UPX_ResourceAttributeSet::GetMaxStaminaAttribute()).AddUObject(this, &UPX_StaminaGaugeWidget::HandleStaminaAttributeChanged);

	if ( UWorld* World = GetWorld() )
	{
		World->GetTimerManager().ClearTimer(BindRetryTimerHandle);
	}

	RefreshStaminaFromAttributeSet();
}

void UPX_StaminaGaugeWidget::UnbindFromAbilitySystemComponent()
{
	if ( !BoundASC )
	{
		StaminaChangedHandle.Reset();
		MaxStaminaChangedHandle.Reset();
		return;
	}

	if ( StaminaChangedHandle.IsValid() )
	{
		BoundASC->GetGameplayAttributeValueChangeDelegate(UPX_ResourceAttributeSet::GetStaminaAttribute()).Remove(StaminaChangedHandle);
	}

	if ( MaxStaminaChangedHandle.IsValid() )
	{
		BoundASC->GetGameplayAttributeValueChangeDelegate(UPX_ResourceAttributeSet::GetMaxStaminaAttribute()).Remove(MaxStaminaChangedHandle);
	}

	StaminaChangedHandle.Reset();
	MaxStaminaChangedHandle.Reset();
	BoundASC = nullptr;
	ClearPredictedStamina();
}

void UPX_StaminaGaugeWidget::UpdateStamina(float CurrentStamina, float MaxStamina)
{
	ServerMaxStamina = FMath::Max(MaxStamina, 0.0f);
	ServerStamina = FMath::Clamp(CurrentStamina, 0.0f, ServerMaxStamina);

	if ( !UPX_GameplayPredictionSettings::IsClientPredictedStaminaUIEnabled() || !bUseClientPredictedStamina )
	{
		ClearPredictedStamina();
		UpdateStaminaGauge(ServerStamina, ServerMaxStamina);
		return;
	}

	if ( bShowingPredictedStamina )
	{
		const bool bServerCaughtPrediction = ServerStamina <= PredictedStamina + PredictionServerTolerance;
		const bool bServerRejectedPrediction = ServerStamina >= ServerMaxStamina - PredictionServerTolerance;
		if ( bServerCaughtPrediction || bServerRejectedPrediction )
		{
			ClearPredictedStamina();
			return;
		}

		UpdateStaminaGauge(PredictedStamina, ServerMaxStamina);
		return;
	}

	UpdateStaminaGauge(ServerStamina, ServerMaxStamina);
}

void UPX_StaminaGaugeWidget::ApplyPredictedStaminaCost(float StaminaCost)
{
	if ( !UPX_GameplayPredictionSettings::IsClientPredictedStaminaUIEnabled() || !bUseClientPredictedStamina )
	{
		ClearPredictedStamina();
		return;
	}

	if ( StaminaCost <= 0.0f )
	{
		return;
	}

	if ( BoundASC )
	{
		ServerStamina = FMath::Clamp(BoundASC->GetNumericAttribute(UPX_ResourceAttributeSet::GetStaminaAttribute()), 0.0f, BoundASC->GetNumericAttribute(UPX_ResourceAttributeSet::GetMaxStaminaAttribute()));
		ServerMaxStamina = FMath::Max(BoundASC->GetNumericAttribute(UPX_ResourceAttributeSet::GetMaxStaminaAttribute()), 0.0f);
	}

	const float PredictionBase = bShowingPredictedStamina ? PredictedStamina : ServerStamina;
	PredictedStamina = FMath::Clamp(PredictionBase - StaminaCost, 0.0f, ServerMaxStamina);
	PredictionElapsed = 0.0f;
	bShowingPredictedStamina = true;

	UpdateStaminaGauge(PredictedStamina, ServerMaxStamina);
}

bool UPX_StaminaGaugeWidget::GetDisplayedStamina(float& OutCurrentStamina, float& OutMaxStamina) const
{
	OutMaxStamina = ServerMaxStamina;
	OutCurrentStamina = (UPX_GameplayPredictionSettings::IsClientPredictedStaminaUIEnabled() && bUseClientPredictedStamina && bShowingPredictedStamina) ? PredictedStamina : ServerStamina;
	return ServerMaxStamina > 0.0f;
}

void UPX_StaminaGaugeWidget::UpdateStaminaGauge(float CurrentStamina, float MaxStamina)
{
	const float NewRatio = MaxStamina > 0.0f ? FMath::Clamp(CurrentStamina / MaxStamina, 0.0f, 1.0f) : 0.0f;

	CurrentRatio = NewRatio;
	SetMaterialPercent(FG_MID, CurrentRatio);
	ApplyGaugeVisibility(CurrentRatio);

	if ( NewRatio < LostRatio )
	{
		LostStartRatio = LostRatio;
		LostTargetRatio = NewRatio;
		LostFollowElapsed = 0.0f;
		bLostFollowing = true;
	}
	else
	{
		bLostFollowing = false;
		LostFollowElapsed = 0.0f;

		LostRatio = NewRatio;
		LostStartRatio = NewRatio;
		LostTargetRatio = NewRatio;

		SetMaterialPercent(Lost_MID, LostRatio);
	}
}

void UPX_StaminaGaugeWidget::ClearPredictedStamina()
{
	if ( !bShowingPredictedStamina )
	{
		return;
	}

	bShowingPredictedStamina = false;
	PredictionElapsed = 0.0f;
	PredictedStamina = ServerStamina;
	UpdateStaminaGauge(ServerStamina, ServerMaxStamina);
}

void UPX_StaminaGaugeWidget::ApplyGaugeVisibility(float Ratio)
{
	const bool bIsFullStamina = FMath::IsNearlyEqual(Ratio, 1.0f, FullStaminaHiddenTolerance);
	const bool bShouldShow = !bIsFullStamina;
	SetVisibility(bShouldShow ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Hidden);
}

void UPX_StaminaGaugeWidget::TryBindToOwningPawnAbilitySystem()
{
	if ( BoundASC )
	{
		return;
	}

	if ( APawn* OwningPawn = GetOwningPlayerPawn() )
	{
		BindToAbilitySystemComponent(UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OwningPawn));
	}

	if ( !BoundASC )
	{
		if ( UWorld* World = GetWorld() )
		{
			World->GetTimerManager().SetTimer(BindRetryTimerHandle, this, &UPX_StaminaGaugeWidget::TryBindToOwningPawnAbilitySystem, 0.2f, true);
		}
	}
}

void UPX_StaminaGaugeWidget::RefreshStaminaFromAttributeSet()
{
	if ( !BoundASC )
	{
		UpdateStamina(0.0f, 1.0f);
		return;
	}

	UpdateStamina(
		BoundASC->GetNumericAttribute(UPX_ResourceAttributeSet::GetStaminaAttribute()),
		BoundASC->GetNumericAttribute(UPX_ResourceAttributeSet::GetMaxStaminaAttribute()));
}

void UPX_StaminaGaugeWidget::HandleStaminaAttributeChanged(const FOnAttributeChangeData& ChangeData)
{
	RefreshStaminaFromAttributeSet();
}

void UPX_StaminaGaugeWidget::InitializeMaterialInstances()
{
	if ( Image_Stamina_FG )
	{
		FG_MID = Image_Stamina_FG->GetDynamicMaterial();
	}

	if ( Image_Stamina_Lost )
	{
		Lost_MID = Image_Stamina_Lost->GetDynamicMaterial();
	}

	if ( Image_Stamina_BG )
	{
		BG_MID = Image_Stamina_BG->GetDynamicMaterial();
	}
}

void UPX_StaminaGaugeWidget::SetMaterialPercent(UMaterialInstanceDynamic* MID, float Percent)
{
	if ( !MID )
	{
		return;
	}

	const float ClampedPercent = FMath::Clamp(Percent, 0.0f, 1.0f);
	MID->SetScalarParameterValue(PercentParameterName, ClampedPercent);

	for ( const FName& ParameterName : AdditionalPercentParameterNames )
	{
		if ( ParameterName != NAME_None && ParameterName != PercentParameterName )
		{
			MID->SetScalarParameterValue(ParameterName, ClampedPercent);
		}
	}
}
