// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Blueprint/UserWidget.h"
#include "PX_SkillIconWidget.generated.h"

class UAbilitySystemComponent;
class UGameplayAbility;
class UImage;
class UMaterialInstanceDynamic;
class UTextBlock;
class UPX_KeyIconWidget;
class UPX_GA_WeaponStatusImbueBase;

UCLASS()
class PROJECT_X_API UPX_SkillIconWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Skill Icon")
	void BindToAbilitySystemComponent(UAbilitySystemComponent* InASC);

	UFUNCTION(BlueprintCallable, Category = "Skill Icon")
	void UnbindFromAbilitySystemComponent();

	UFUNCTION(BlueprintCallable, Category = "Skill Icon")
	void SetCooldownTag(FGameplayTag InCooldownTag);

	UFUNCTION(BlueprintCallable, Category = "Skill Icon")
	void RefreshAbilityAvailability();

	UFUNCTION(BlueprintCallable, Category = "Skill Icon")
	void SetExternalBlocked(bool bBlocked);

	UFUNCTION(BlueprintCallable, Category = "Skill Icon")
	void SetKeyIconWidget(UPX_KeyIconWidget* InKeyIconWidget);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	virtual void NativeDestruct() override;

protected:
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> Image_SkillIcon_BG;
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> Image_SkillIcon;
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> Image_SkillIcon_Cooldown_BG;
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> Image_SkillIcon_Cooldown;
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> TextBlock_SkillIcon_Cooldown;
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> Image_SkillIcon_Blocked;
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> Image_SkillIcon_Frame;
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UPX_KeyIconWidget> WBP_HUD_KeyIcon;

private:
	void TryBindToOwningPawnAbilitySystem();
	void InitializeCooldownMaterial();
	void BindBlockedTagEvents();
	void UnbindBlockedTagEvents();
	void HandleBlockedTagChanged(const FGameplayTag ChangedTag, int32 NewCount);
	void RefreshBlockedTagState();
	void RefreshCooldown();
	void RefreshBlockedState();
	void ApplyCooldown(float RemainingTime, float Duration);
	void ApplySkillData(const UPX_GA_WeaponStatusImbueBase* StatusImbueAbility);
	void ClearSkillData();
	void SetMaterialPercent(float Percent);
	const UPX_GA_WeaponStatusImbueBase* FindSkillAbilityCDO() const;
	FText GetKeyTextForInputTag(FGameplayTag InputTag) const;

private:
	UPROPERTY(Transient)
	TObjectPtr<UAbilitySystemComponent> BoundASC;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> CooldownMID;

	UPROPERTY(EditAnywhere, Category = "Skill Icon|Cooldown", meta = (Categories = "Cooldown"))
	FGameplayTag CooldownTag;

	UPROPERTY(EditAnywhere, Category = "Skill Icon|Cooldown")
	FName PercentParameterName = TEXT("Percent");

	UPROPERTY(EditAnywhere, Category = "Skill Icon|Ability")
	TSubclassOf<UGameplayAbility> SkillAbilityClass;

	UPROPERTY(EditAnywhere, Category = "Skill Icon|Blocked", meta = (Categories = "State"))
	TArray<FGameplayTag> BlockedStateTags;

	FGameplayTag ActiveCooldownTag;

	bool bSkillAbilityAvailable = false;
	bool bBlockedByStateTag = false;
	bool bBlockedByExternalState = false;
	bool bCoolingDown = false;

	TMap<FGameplayTag, FDelegateHandle> BlockedTagEventHandles;

	FTimerHandle BindRetryTimerHandle;
};
