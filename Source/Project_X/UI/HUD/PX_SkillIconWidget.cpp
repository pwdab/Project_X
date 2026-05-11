// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/HUD/PX_SkillIconWidget.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/Abilities/PX_GA_WeaponStatusImbue.h"
#include "AbilitySystem/Component/PX_AbilitySystemComponent.h"
#include "AbilitySystem/Tags/PX_GameplayTags.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "GameFramework/Pawn.h"
#include "GameplayEffectTypes.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "TimerManager.h"
#include "UI/HUD/PX_KeyIconWidget.h"

void UPX_SkillIconWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if ( !CooldownTag.IsValid() )
	{
		CooldownTag = PX_GameplayTags::Cooldown_Weapon_ImbueBurn;
	}
	if ( !SkillAbilityClass )
	{
		SkillAbilityClass = UPX_GA_WeaponStatusImbueBase::StaticClass();
	}
	if ( BlockedStateTags.IsEmpty() )
	{
		BlockedStateTags = {
			PX_GameplayTags::State_Combat_Equipping,
			PX_GameplayTags::State_Combat_Unequipping,
			PX_GameplayTags::State_Combat_Reloading,
			PX_GameplayTags::State_Condition_Dead,
			PX_GameplayTags::State_Condition_Down,
			PX_GameplayTags::State_Condition_Disabled,
			PX_GameplayTags::State_Condition_Stunned
		};
	}

	InitializeCooldownMaterial();
	if ( !WBP_HUD_KeyIcon )
	{
		WBP_HUD_KeyIcon = Cast<UPX_KeyIconWidget>(GetWidgetFromName(TEXT("WBP_HUD_KeyIcon")));
		if ( !WBP_HUD_KeyIcon )
		{
			WBP_HUD_KeyIcon = Cast<UPX_KeyIconWidget>(GetWidgetFromName(TEXT("KeyIcon")));
		}
	}
	TryBindToOwningPawnAbilitySystem();
	RefreshAbilityAvailability();
	ApplyCooldown(0.0f, 0.0f);
}

void UPX_SkillIconWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if ( bSkillAbilityAvailable )
	{
		RefreshCooldown();
	}
	else
	{
		RefreshBlockedState();
	}
}

void UPX_SkillIconWidget::NativeDestruct()
{
	UnbindFromAbilitySystemComponent();

	Super::NativeDestruct();
}

void UPX_SkillIconWidget::BindToAbilitySystemComponent(UAbilitySystemComponent* InASC)
{
	if ( BoundASC == InASC )
	{
		RefreshAbilityAvailability();
		RefreshCooldown();
		return;
	}

	if ( UPX_AbilitySystemComponent* PXASC = Cast<UPX_AbilitySystemComponent>(BoundASC) )
	{
		PXASC->OnAbilityListChanged.RemoveAll(this);
	}
	UnbindBlockedTagEvents();

	BoundASC = InASC;
	if ( UPX_AbilitySystemComponent* PXASC = Cast<UPX_AbilitySystemComponent>(BoundASC) )
	{
		PXASC->OnAbilityListChanged.AddUObject(this, &UPX_SkillIconWidget::RefreshAbilityAvailability);
	}
	BindBlockedTagEvents();
	if ( UWorld* World = GetWorld() )
	{
		World->GetTimerManager().ClearTimer(BindRetryTimerHandle);
	}

	RefreshAbilityAvailability();
	RefreshCooldown();
}

void UPX_SkillIconWidget::UnbindFromAbilitySystemComponent()
{
	if ( UWorld* World = GetWorld() )
	{
		World->GetTimerManager().ClearTimer(BindRetryTimerHandle);
	}

	if ( UPX_AbilitySystemComponent* PXASC = Cast<UPX_AbilitySystemComponent>(BoundASC) )
	{
		PXASC->OnAbilityListChanged.RemoveAll(this);
	}
	UnbindBlockedTagEvents();

	BoundASC = nullptr;
	ActiveCooldownTag = FGameplayTag();
	bSkillAbilityAvailable = false;
	bBlockedByStateTag = false;
	bBlockedByExternalState = false;
	bCoolingDown = false;
	SetVisibility(ESlateVisibility::Hidden);
	ClearSkillData();
	ApplyCooldown(0.0f, 0.0f);
	RefreshBlockedState();
}

void UPX_SkillIconWidget::SetCooldownTag(FGameplayTag InCooldownTag)
{
	CooldownTag = InCooldownTag;
	if ( !ActiveCooldownTag.IsValid() )
	{
		ActiveCooldownTag = CooldownTag;
	}
	RefreshCooldown();
}

void UPX_SkillIconWidget::SetExternalBlocked(bool bBlocked)
{
	bBlockedByExternalState = bBlocked;
	RefreshBlockedState();
}

void UPX_SkillIconWidget::SetKeyIconWidget(UPX_KeyIconWidget* InKeyIconWidget)
{
	WBP_HUD_KeyIcon = InKeyIconWidget;
	if ( BoundASC )
	{
		RefreshAbilityAvailability();
	}
	else
	{
		ClearSkillData();
	}
}

void UPX_SkillIconWidget::RefreshAbilityAvailability()
{
	if ( const UPX_GA_WeaponStatusImbueBase* StatusImbueAbility = FindSkillAbilityCDO() )
	{
		bSkillAbilityAvailable = true;
		ActiveCooldownTag = StatusImbueAbility->GetCooldownTag();
		ApplySkillData(StatusImbueAbility);
	}
	else
	{
		bSkillAbilityAvailable = false;
		ActiveCooldownTag = FGameplayTag();
		ClearSkillData();
	}

	SetVisibility(bSkillAbilityAvailable ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Hidden);

	if ( bSkillAbilityAvailable )
	{
		RefreshBlockedTagState();
		RefreshCooldown();
	}
	else
	{
		ApplyCooldown(0.0f, 0.0f);
		RefreshBlockedState();
	}
}

void UPX_SkillIconWidget::TryBindToOwningPawnAbilitySystem()
{
	APawn* OwningPawn = GetOwningPlayerPawn();
	if ( !OwningPawn )
	{
		if ( UWorld* World = GetWorld() )
		{
			World->GetTimerManager().SetTimer(BindRetryTimerHandle, this, &UPX_SkillIconWidget::TryBindToOwningPawnAbilitySystem, 0.25f, false);
		}
		return;
	}

	BindToAbilitySystemComponent(UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OwningPawn));

	if ( !BoundASC )
	{
		if ( UWorld* World = GetWorld() )
		{
			World->GetTimerManager().SetTimer(BindRetryTimerHandle, this, &UPX_SkillIconWidget::TryBindToOwningPawnAbilitySystem, 0.25f, false);
		}
	}
}

void UPX_SkillIconWidget::InitializeCooldownMaterial()
{
	if ( Image_SkillIcon_Cooldown )
	{
		CooldownMID = Image_SkillIcon_Cooldown->GetDynamicMaterial();
	}
}

void UPX_SkillIconWidget::BindBlockedTagEvents()
{
	UnbindBlockedTagEvents();

	if ( !BoundASC )
	{
		return;
	}

	for ( const FGameplayTag& BlockedTag : BlockedStateTags )
	{
		if ( !BlockedTag.IsValid() )
		{
			continue;
		}

		FDelegateHandle Handle = BoundASC->RegisterGameplayTagEvent(BlockedTag, EGameplayTagEventType::NewOrRemoved).AddUObject(this, &UPX_SkillIconWidget::HandleBlockedTagChanged);
		BlockedTagEventHandles.Add(BlockedTag, Handle);
	}

	RefreshBlockedTagState();
}

void UPX_SkillIconWidget::UnbindBlockedTagEvents()
{
	if ( BoundASC )
	{
		for ( const TPair<FGameplayTag, FDelegateHandle>& Pair : BlockedTagEventHandles )
		{
			if ( Pair.Key.IsValid() && Pair.Value.IsValid() )
			{
				BoundASC->RegisterGameplayTagEvent(Pair.Key, EGameplayTagEventType::NewOrRemoved).Remove(Pair.Value);
			}
		}
	}

	BlockedTagEventHandles.Reset();
}

void UPX_SkillIconWidget::HandleBlockedTagChanged(const FGameplayTag ChangedTag, int32 NewCount)
{
	RefreshBlockedTagState();
}

void UPX_SkillIconWidget::RefreshBlockedTagState()
{
	bBlockedByStateTag = false;

	if ( BoundASC )
	{
		for ( const FGameplayTag& BlockedTag : BlockedStateTags )
		{
			if ( BlockedTag.IsValid() && BoundASC->HasMatchingGameplayTag(BlockedTag) )
			{
				bBlockedByStateTag = true;
				break;
			}
		}
	}

	RefreshBlockedState();
}

void UPX_SkillIconWidget::RefreshCooldown()
{
	const FGameplayTag CooldownTagToQuery = ActiveCooldownTag.IsValid() ? ActiveCooldownTag : CooldownTag;
	if ( !bSkillAbilityAvailable || !BoundASC || !CooldownTagToQuery.IsValid() )
	{
		ApplyCooldown(0.0f, 0.0f);
		return;
	}

	FGameplayTagContainer CooldownTags;
	CooldownTags.AddTag(CooldownTagToQuery);

	const FGameplayEffectQuery CooldownQuery = FGameplayEffectQuery::MakeQuery_MatchAnyOwningTags(CooldownTags);
	const TArray<TPair<float, float>> CooldownPairs = BoundASC->GetActiveEffectsTimeRemainingAndDuration(CooldownQuery);

	float LongestRemainingTime = 0.0f;
	float LongestDuration = 0.0f;
	for ( const TPair<float, float>& CooldownPair : CooldownPairs )
	{
		if ( CooldownPair.Key > LongestRemainingTime )
		{
			LongestRemainingTime = CooldownPair.Key;
			LongestDuration = CooldownPair.Value;
		}
	}

	ApplyCooldown(LongestRemainingTime, LongestDuration);
}

void UPX_SkillIconWidget::RefreshBlockedState()
{
	if ( !Image_SkillIcon_Blocked )
	{
		return;
	}

	const bool bBlocked = bSkillAbilityAvailable && (bBlockedByStateTag || bBlockedByExternalState);
	Image_SkillIcon_Blocked->SetVisibility(bBlocked ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Hidden);
}

void UPX_SkillIconWidget::ApplyCooldown(float RemainingTime, float Duration)
{
	bCoolingDown = RemainingTime > KINDA_SMALL_NUMBER && Duration > KINDA_SMALL_NUMBER;
	const float CooldownPercent = bCoolingDown ? FMath::Clamp(RemainingTime / Duration, 0.0f, 1.0f) : 0.0f;

	SetMaterialPercent(CooldownPercent);

	const ESlateVisibility CooldownVisibility = bCoolingDown ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Hidden;
	if ( Image_SkillIcon_Cooldown_BG )
	{
		Image_SkillIcon_Cooldown_BG->SetVisibility(CooldownVisibility);
	}
	if ( Image_SkillIcon_Cooldown )
	{
		Image_SkillIcon_Cooldown->SetVisibility(CooldownVisibility);
	}
	if ( TextBlock_SkillIcon_Cooldown )
	{
		TextBlock_SkillIcon_Cooldown->SetVisibility(CooldownVisibility);
		TextBlock_SkillIcon_Cooldown->SetText(bCoolingDown
			? FText::FromString(FString::Printf(TEXT("%.1f"), RemainingTime))
			: FText::GetEmpty());
	}

	RefreshBlockedState();
}

void UPX_SkillIconWidget::ApplySkillData(const UPX_GA_WeaponStatusImbueBase* StatusImbueAbility)
{
	if ( !StatusImbueAbility )
	{
		ClearSkillData();
		return;
	}

	if ( Image_SkillIcon && StatusImbueAbility->GetSkillIcon() )
	{
		Image_SkillIcon->SetBrushFromTexture(StatusImbueAbility->GetSkillIcon(), true);
	}

	if ( !WBP_HUD_KeyIcon )
	{
		WBP_HUD_KeyIcon = Cast<UPX_KeyIconWidget>(GetWidgetFromName(TEXT("WBP_HUD_KeyIcon")));
		if ( !WBP_HUD_KeyIcon )
		{
			WBP_HUD_KeyIcon = Cast<UPX_KeyIconWidget>(GetWidgetFromName(TEXT("KeyIcon")));
		}
	}
	if ( WBP_HUD_KeyIcon )
	{
		WBP_HUD_KeyIcon->UpdateText(GetKeyTextForInputTag(StatusImbueAbility->GetInputTag()));
	}
}

void UPX_SkillIconWidget::ClearSkillData()
{
	if ( !WBP_HUD_KeyIcon )
	{
		WBP_HUD_KeyIcon = Cast<UPX_KeyIconWidget>(GetWidgetFromName(TEXT("WBP_HUD_KeyIcon")));
		if ( !WBP_HUD_KeyIcon )
		{
			WBP_HUD_KeyIcon = Cast<UPX_KeyIconWidget>(GetWidgetFromName(TEXT("KeyIcon")));
		}
	}
	if ( WBP_HUD_KeyIcon )
	{
		WBP_HUD_KeyIcon->UpdateText(FText::GetEmpty());
	}
}

const UPX_GA_WeaponStatusImbueBase* UPX_SkillIconWidget::FindSkillAbilityCDO() const
{
	if ( !BoundASC )
	{
		return nullptr;
	}

	for ( const FGameplayAbilitySpec& Spec : BoundASC->GetActivatableAbilities() )
	{
		const UPX_GA_WeaponStatusImbueBase* StatusImbueAbility = Cast<UPX_GA_WeaponStatusImbueBase>(Spec.Ability);
		if ( StatusImbueAbility && Spec.DynamicAbilityTags.HasTagExact(PX_GameplayTags::Input_Skill_E) )
		{
			return StatusImbueAbility;
		}
	}

	return nullptr;
}

FText UPX_SkillIconWidget::GetKeyTextForInputTag(FGameplayTag InputTag) const
{
	if ( InputTag == PX_GameplayTags::Input_Skill_E )
	{
		return FText::FromString(TEXT("E"));
	}

	if ( InputTag.IsValid() )
	{
		FString TagString = InputTag.ToString();
		FString RightPart;
		if ( TagString.Split(TEXT("."), nullptr, &RightPart, ESearchCase::IgnoreCase, ESearchDir::FromEnd) )
		{
			return FText::FromString(RightPart);
		}
		return FText::FromString(TagString);
	}

	return FText::GetEmpty();
}

void UPX_SkillIconWidget::SetMaterialPercent(float Percent)
{
	if ( !CooldownMID )
	{
		InitializeCooldownMaterial();
	}

	if ( CooldownMID )
	{
		CooldownMID->SetScalarParameterValue(PercentParameterName, FMath::Clamp(Percent, 0.0f, 1.0f));
	}
}
