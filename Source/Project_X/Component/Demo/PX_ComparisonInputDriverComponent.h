// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "PX_ComparisonInputDriverComponent.generated.h"

class APX_Character;

UENUM(BlueprintType)
enum class EPXComparisonInputAction : uint8
{
	Move UMETA(DisplayName = "Move"),
	AbilityHold UMETA(DisplayName = "Ability Hold"),
	AbilityTap UMETA(DisplayName = "Ability Tap"),
	EquipSlot UMETA(DisplayName = "Equip Slot"),
	Look UMETA(DisplayName = "Look")
};

USTRUCT(BlueprintType)
struct FPXComparisonInputStep
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Comparison")
	EPXComparisonInputAction Action = EPXComparisonInputAction::Move;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Comparison", meta = (ClampMin = "0.0"))
	float StartTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Comparison", meta = (ClampMin = "0.0"))
	float Duration = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Comparison", meta = (EditCondition = "Action == EPXComparisonInputAction::Move || Action == EPXComparisonInputAction::Look", EditConditionHides))
	FVector2D Axis = FVector2D::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Comparison", meta = (EditCondition = "Action == EPXComparisonInputAction::AbilityHold || Action == EPXComparisonInputAction::AbilityTap", EditConditionHides))
	FGameplayTag InputTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Comparison", meta = (ClampMin = "0", ClampMax = "5", EditCondition = "Action == EPXComparisonInputAction::EquipSlot", EditConditionHides))
	int32 EquipSlotNumber = 1;
};

UCLASS(ClassGroup = (PX), meta = (BlueprintSpawnableComponent))
class PROJECT_X_API UPX_ComparisonInputDriverComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPX_ComparisonInputDriverComponent();

	UFUNCTION(BlueprintCallable, Category = "PX|Comparison")
	void StartPlayback();

	UFUNCTION(BlueprintCallable, Category = "PX|Comparison")
	void StopPlayback();

	UFUNCTION(BlueprintCallable, Category = "PX|Comparison")
	void RestartPlayback();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	void InitializeDefaultSequence();
	void ResetRuntimeState();
	void PressStep(int32 StepIndex);
	void ReleaseStep(int32 StepIndex);
	bool CanDriveOwner() const;
	float GetStepEndTime(const FPXComparisonInputStep& Step) const;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PX|Comparison", meta = (AllowPrivateAccess = "true"))
	bool bAutoStart = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PX|Comparison", meta = (AllowPrivateAccess = "true"))
	bool bRunOnlyOnLocallyControlledOwner = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PX|Comparison", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float AutoStartDelay = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PX|Comparison", meta = (AllowPrivateAccess = "true"))
	bool bLoop = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PX|Comparison", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float LoopDelay = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PX|Comparison", meta = (AllowPrivateAccess = "true", ClampMin = "0.01"))
	float TapDuration = 0.08f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PX|Comparison", meta = (AllowPrivateAccess = "true"))
	TArray<FPXComparisonInputStep> Sequence;

	UPROPERTY(Transient)
	TObjectPtr<APX_Character> OwnerCharacter;

	TSet<int32> StartedSteps;
	TSet<int32> ReleasedSteps;
	bool bPlaying = false;
	float PlaybackTime = 0.0f;
	float PendingAutoStartTime = -1.0f;
	float PendingLoopStartTime = -1.0f;
};
