// Fill out your copyright notice in the Description page of Project Settings.

#include "Component/Demo/PX_ComparisonInputDriverComponent.h"

#include "AbilitySystem/Tags/PX_GameplayTags.h"
#include "Entity/PX_Character.h"
#include "GameFramework/Controller.h"

namespace
{
FPXComparisonInputStep MakeComparisonInputStep(EPXComparisonInputAction Action, float StartTime, float Duration, FVector2D Axis, FGameplayTag InputTag, int32 EquipSlotNumber = 1)
{
	FPXComparisonInputStep Step;
	Step.Action = Action;
	Step.StartTime = StartTime;
	Step.Duration = Duration;
	Step.Axis = Axis;
	Step.InputTag = InputTag;
	Step.EquipSlotNumber = EquipSlotNumber;
	return Step;
}
}

UPX_ComparisonInputDriverComponent::UPX_ComparisonInputDriverComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
	InitializeDefaultSequence();
}

void UPX_ComparisonInputDriverComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerCharacter = Cast<APX_Character>(GetOwner());
	ResetRuntimeState();

	if ( bAutoStart )
	{
		PendingAutoStartTime = AutoStartDelay;
	}
}

void UPX_ComparisonInputDriverComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	StopPlayback();
	Super::EndPlay(EndPlayReason);
}

void UPX_ComparisonInputDriverComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if ( PendingAutoStartTime >= 0.0f )
	{
		PendingAutoStartTime -= DeltaTime;
		if ( PendingAutoStartTime <= 0.0f )
		{
			if ( CanDriveOwner() )
			{
				PendingAutoStartTime = -1.0f;
				StartPlayback();
			}
			else
			{
				PendingAutoStartTime = 0.0f;
			}
		}
	}

	if ( PendingLoopStartTime >= 0.0f )
	{
		PendingLoopStartTime -= DeltaTime;
		if ( PendingLoopStartTime <= 0.0f )
		{
			if ( CanDriveOwner() )
			{
				PendingLoopStartTime = -1.0f;
				StartPlayback();
			}
			else
			{
				PendingLoopStartTime = 0.0f;
			}
		}
	}

	if ( !bPlaying || !CanDriveOwner() )
	{
		return;
	}

	PlaybackTime += DeltaTime;

	bool bHasActiveMove = false;
	FVector2D AccumulatedMove = FVector2D::ZeroVector;
	float LastStepEndTime = 0.0f;

	for ( int32 StepIndex = 0; StepIndex < Sequence.Num(); ++StepIndex )
	{
		const FPXComparisonInputStep& Step = Sequence[StepIndex];
		const float StepEndTime = GetStepEndTime(Step);
		LastStepEndTime = FMath::Max(LastStepEndTime, StepEndTime);

		if ( PlaybackTime >= Step.StartTime && !StartedSteps.Contains(StepIndex) )
		{
			PressStep(StepIndex);
			StartedSteps.Add(StepIndex);
		}

		if ( PlaybackTime >= Step.StartTime && PlaybackTime < StepEndTime )
		{
			if ( Step.Action == EPXComparisonInputAction::Move )
			{
				bHasActiveMove = true;
				AccumulatedMove += Step.Axis;
			}
			else if ( Step.Action == EPXComparisonInputAction::Look && OwnerCharacter && OwnerCharacter->GetController() )
			{
				OwnerCharacter->AddControllerYawInput(Step.Axis.X * DeltaTime);
				OwnerCharacter->AddControllerPitchInput(Step.Axis.Y * DeltaTime);
			}
		}

		if ( PlaybackTime >= StepEndTime && StartedSteps.Contains(StepIndex) && !ReleasedSteps.Contains(StepIndex) )
		{
			ReleaseStep(StepIndex);
			ReleasedSteps.Add(StepIndex);
		}
	}

	if ( bHasActiveMove )
	{
		if ( AccumulatedMove.SizeSquared() > 1.0f )
		{
			AccumulatedMove.Normalize();
		}

		OwnerCharacter->ComparisonBeginMoveInput(AccumulatedMove);
	}
	else
	{
		OwnerCharacter->ComparisonEndMoveInput();
	}

	if ( PlaybackTime >= LastStepEndTime )
	{
		StopPlayback();
		if ( bLoop )
		{
			PendingLoopStartTime = LoopDelay;
		}
	}
}

void UPX_ComparisonInputDriverComponent::StartPlayback()
{
	if ( !CanDriveOwner() )
	{
		return;
	}

	ResetRuntimeState();
	PendingLoopStartTime = -1.0f;
	bPlaying = true;
}

void UPX_ComparisonInputDriverComponent::StopPlayback()
{
	if ( OwnerCharacter )
	{
		for ( int32 StepIndex : StartedSteps )
		{
			if ( !ReleasedSteps.Contains(StepIndex) )
			{
				ReleaseStep(StepIndex);
			}
		}
		OwnerCharacter->ComparisonEndMoveInput();
	}

	bPlaying = false;
	PendingAutoStartTime = -1.0f;
	PendingLoopStartTime = -1.0f;
}

void UPX_ComparisonInputDriverComponent::RestartPlayback()
{
	StopPlayback();
	StartPlayback();
}

void UPX_ComparisonInputDriverComponent::InitializeDefaultSequence()
{
	if ( !Sequence.IsEmpty() )
	{
		return;
	}

	Sequence = {
		MakeComparisonInputStep(EPXComparisonInputAction::AbilityHold, 0.00f, 3.00f, FVector2D::ZeroVector, PX_GameplayTags::Input_Locomotion_Sprint),
		MakeComparisonInputStep(EPXComparisonInputAction::Move, 0.00f, 1.30f, FVector2D(0.0f, 1.0f), FGameplayTag()),
		MakeComparisonInputStep(EPXComparisonInputAction::Move, 1.50f, 1.50f, FVector2D(0.0f, -1.0f), FGameplayTag()),
		MakeComparisonInputStep(EPXComparisonInputAction::Move, 3.00f, 1.40f, FVector2D(0.0f, 1.0f), FGameplayTag()),
		MakeComparisonInputStep(EPXComparisonInputAction::AbilityTap, 3.25f, 0.0f, FVector2D::ZeroVector, PX_GameplayTags::Input_Locomotion_Jump),
		MakeComparisonInputStep(EPXComparisonInputAction::AbilityTap, 4.40f, 0.0f, FVector2D::ZeroVector, PX_GameplayTags::Input_Locomotion_Jump),
		MakeComparisonInputStep(EPXComparisonInputAction::Move, 5.50f, 1.50f, FVector2D(0.0f, 1.0f), FGameplayTag()),
		MakeComparisonInputStep(EPXComparisonInputAction::EquipSlot, 7.50f, 0.0f, FVector2D::ZeroVector, FGameplayTag(), 1),
		MakeComparisonInputStep(EPXComparisonInputAction::EquipSlot, 9.25f, 0.0f, FVector2D::ZeroVector, FGameplayTag(), 2),
		MakeComparisonInputStep(EPXComparisonInputAction::EquipSlot, 11.00f, 0.0f, FVector2D::ZeroVector, FGameplayTag(), 3),
		MakeComparisonInputStep(EPXComparisonInputAction::EquipSlot, 12.75f, 0.0f, FVector2D::ZeroVector, FGameplayTag(), 0),
		MakeComparisonInputStep(EPXComparisonInputAction::EquipSlot, 13.25f, 0.0f, FVector2D::ZeroVector, FGameplayTag(), 1),
		MakeComparisonInputStep(EPXComparisonInputAction::EquipSlot, 13.25f, 0.0f, FVector2D::ZeroVector, FGameplayTag(), 2),
		MakeComparisonInputStep(EPXComparisonInputAction::Move, 15.25f, 0.70f, FVector2D(1.0f, 1.0f), FGameplayTag()),
		MakeComparisonInputStep(EPXComparisonInputAction::Move, 15.75f, 0.50f, FVector2D(0.0f, 1.0f), FGameplayTag()),
		MakeComparisonInputStep(EPXComparisonInputAction::AbilityTap, 20.00f, 0.0f, FVector2D::ZeroVector, PX_GameplayTags::Input_Combat_Reload),
		MakeComparisonInputStep(EPXComparisonInputAction::AbilityTap, 22.00f, 0.0f, FVector2D::ZeroVector, PX_GameplayTags::Input_Combat_Attack),
		MakeComparisonInputStep(EPXComparisonInputAction::AbilityTap, 23.50f, 0.0f, FVector2D::ZeroVector, PX_GameplayTags::Input_Skill_E),
		MakeComparisonInputStep(EPXComparisonInputAction::AbilityTap, 24.50f, 0.0f, FVector2D::ZeroVector, PX_GameplayTags::Input_Combat_Attack)
	};
}

void UPX_ComparisonInputDriverComponent::ResetRuntimeState()
{
	StartedSteps.Reset();
	ReleasedSteps.Reset();
	PlaybackTime = 0.0f;
}

void UPX_ComparisonInputDriverComponent::PressStep(int32 StepIndex)
{
	if ( !OwnerCharacter || !Sequence.IsValidIndex(StepIndex) )
	{
		return;
	}

	const FPXComparisonInputStep& Step = Sequence[StepIndex];
	if ( Step.Action == EPXComparisonInputAction::AbilityHold || Step.Action == EPXComparisonInputAction::AbilityTap )
	{
		OwnerCharacter->ComparisonPressAbilityInput(Step.InputTag);
	}
	else if ( Step.Action == EPXComparisonInputAction::EquipSlot )
	{
		const int32 WeaponSlotIndex = Step.EquipSlotNumber <= 0 ? 4 : Step.EquipSlotNumber - 1;
		OwnerCharacter->ComparisonEquipSlot(WeaponSlotIndex);
	}
}

void UPX_ComparisonInputDriverComponent::ReleaseStep(int32 StepIndex)
{
	if ( !OwnerCharacter || !Sequence.IsValidIndex(StepIndex) )
	{
		return;
	}

	const FPXComparisonInputStep& Step = Sequence[StepIndex];
	if ( Step.Action == EPXComparisonInputAction::AbilityHold || Step.Action == EPXComparisonInputAction::AbilityTap )
	{
		OwnerCharacter->ComparisonReleaseAbilityInput(Step.InputTag);
	}
}

bool UPX_ComparisonInputDriverComponent::CanDriveOwner() const
{
	if ( !OwnerCharacter )
	{
		return false;
	}

	return !bRunOnlyOnLocallyControlledOwner || OwnerCharacter->IsLocallyControlled();
}

float UPX_ComparisonInputDriverComponent::GetStepEndTime(const FPXComparisonInputStep& Step) const
{
	const bool bTapLikeAction = Step.Action == EPXComparisonInputAction::AbilityTap || Step.Action == EPXComparisonInputAction::EquipSlot;
	const float EffectiveDuration = bTapLikeAction ? TapDuration : Step.Duration;
	return Step.StartTime + FMath::Max(EffectiveDuration, 0.0f);
}
