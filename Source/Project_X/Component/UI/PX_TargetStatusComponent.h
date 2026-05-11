// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayEffectTypes.h"
#include "PX_TargetStatusComponent.generated.h"

class APawn;
class UAbilitySystemComponent;
class UPX_TargetHealthWidget;
class UWidgetComponent;

UENUM(BlueprintType)
enum class EPXTargetStatusType : uint8
{
	Character,
	Monster
};

USTRUCT()
struct FPXTargetCombatEntry
{
	GENERATED_BODY()

	double LastCombatTime = 0.0;
	double OutOfRangeStartTime = 0.0;
	bool bInCombat = false;
};

UCLASS(ClassGroup = (PX), meta = (BlueprintSpawnableComponent))
class PROJECT_X_API UPX_TargetStatusComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPX_TargetStatusComponent();
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION(BlueprintCallable, Category = "Target Status")
	void NotifyCombatWith(APawn* Viewer);

	UFUNCTION(BlueprintCallable, Category = "Target Status")
	bool IsInCombatWith(APawn* Viewer) const;

	UFUNCTION(BlueprintCallable, Category = "Target Status")
	float GetHealth() const;

	UFUNCTION(BlueprintCallable, Category = "Target Status")
	float GetMaxHealth() const;

	UFUNCTION(BlueprintCallable, Category = "Target Status")
	float GetShield() const;

	UFUNCTION(BlueprintCallable, Category = "Target Status")
	float GetMaxShield() const;

	UFUNCTION(BlueprintCallable, Category = "Target Status")
	bool HasTakenResourceDamage() const;

	UFUNCTION(BlueprintCallable, Category = "Target Status")
	FText GetDisplayName() const;

	UFUNCTION(BlueprintCallable, Category = "Target Status")
	FVector GetStatusWorldLocation() const;

	UFUNCTION(BlueprintCallable, Category = "Target Status")
	UAbilitySystemComponent* GetTargetAbilitySystemComponent() const;

	UFUNCTION(BlueprintCallable, Category = "Target Status")
	void SetStatusWidgetVisible(bool bVisible);

	UFUNCTION(BlueprintCallable, Category = "Target Status")
	void RefreshStatusWidget();

private:
	FPXTargetCombatEntry* FindOrAddCombatEntry(APawn* Viewer);
	bool IsCombatEntryActive(APawn* Viewer, FPXTargetCombatEntry& Entry) const;
	void InitializeStatusWidget();
	void UpdateStatusWidgetDrawSize();
	void BindAttributeDelegates();
	void UnbindAttributeDelegates();
	void HandleStatusAttributeChanged(const FOnAttributeChangeData& ChangeData);

private:
	UPROPERTY(EditAnywhere, Category = "Target Status")
	EPXTargetStatusType TargetStatusType = EPXTargetStatusType::Character;

	UPROPERTY(EditAnywhere, Category = "Target Status")
	FText DisplayName;

	UPROPERTY(EditAnywhere, Category = "Target Status")
	float StatusLocationZOffset = 110.0f;

	UPROPERTY(EditAnywhere, Category = "Target Status|Widget")
	TSubclassOf<UPX_TargetHealthWidget> StatusWidgetClass;

	UPROPERTY(EditAnywhere, Category = "Target Status|Widget")
	FVector2D StatusWidgetDrawSize = FVector2D(220.0f, 60.0f);

	UPROPERTY(EditAnywhere, Category = "Target Status|Character")
	float CharacterCombatTimeout = 5.0f;

	UPROPERTY(EditAnywhere, Category = "Target Status|Monster")
	float MonsterCombatExitDistance = 1800.0f;

	UPROPERTY(EditAnywhere, Category = "Target Status|Monster")
	float MonsterCombatExitDelay = 3.0f;

	UPROPERTY(Transient)
	TObjectPtr<UWidgetComponent> StatusWidgetComponent;
	UPROPERTY(Transient)
	TObjectPtr<UPX_TargetHealthWidget> StatusWidget;
	UPROPERTY(Transient)
	TObjectPtr<UAbilitySystemComponent> BoundASC;
	UPROPERTY(Transient)
	bool bStatusWidgetVisible = false;

	mutable TMap<TWeakObjectPtr<APawn>, FPXTargetCombatEntry> LocalCombatEntries;

	FDelegateHandle HealthChangedHandle;
	FDelegateHandle MaxHealthChangedHandle;
	FDelegateHandle ShieldChangedHandle;
};
