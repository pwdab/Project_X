// Fill out your copyright notice in the Description page of Project Settings.

#include "Component/UI/PX_TargetStatusComponent.h"

#include "Project_X.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/Attributes/PX_ResourceAttributeSet.h"
#include "GameFramework/Pawn.h"
#include "UI/HUD/PX_TargetHealthWidget.h"
#include "Components/SceneComponent.h"
#include "Components/WidgetComponent.h"
#include "UObject/ConstructorHelpers.h"

UPX_TargetStatusComponent::UPX_TargetStatusComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	static ConstructorHelpers::FClassFinder<UPX_TargetHealthWidget> DefaultStatusWidgetClass(TEXT("/Game/Project_X/UMG/HUD/WBP_HUD_TargetHealth"));
	if ( DefaultStatusWidgetClass.Succeeded() )
	{
		StatusWidgetClass = DefaultStatusWidgetClass.Class;
	}
}

void UPX_TargetStatusComponent::BeginPlay()
{
	Super::BeginPlay();

	InitializeStatusWidget();
	BindAttributeDelegates();
	SetStatusWidgetVisible(false);
}

void UPX_TargetStatusComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UnbindAttributeDelegates();

	Super::EndPlay(EndPlayReason);
}

void UPX_TargetStatusComponent::NotifyCombatWith(APawn* Viewer)
{
	if ( !Viewer )
	{
		return;
	}

	FPXTargetCombatEntry* Entry = FindOrAddCombatEntry(Viewer);
	if ( !Entry )
	{
		return;
	}

	const UWorld* World = GetWorld();
	const double CurrentTime = World ? World->GetTimeSeconds() : 0.0;
	Entry->LastCombatTime = CurrentTime;
	Entry->OutOfRangeStartTime = 0.0;
	Entry->bInCombat = true;
}

bool UPX_TargetStatusComponent::IsInCombatWith(APawn* Viewer) const
{
	if ( !Viewer )
	{
		return false;
	}

	FPXTargetCombatEntry* Entry = LocalCombatEntries.Find(Viewer);
	if ( !Entry )
	{
		return false;
	}

	return IsCombatEntryActive(Viewer, *Entry);
}

float UPX_TargetStatusComponent::GetHealth() const
{
	const UAbilitySystemComponent* ASC = GetTargetAbilitySystemComponent();
	return ASC ? ASC->GetNumericAttribute(UPX_ResourceAttributeSet::GetHealthAttribute()) : 0.0f;
}

float UPX_TargetStatusComponent::GetMaxHealth() const
{
	const UAbilitySystemComponent* ASC = GetTargetAbilitySystemComponent();
	return ASC ? ASC->GetNumericAttribute(UPX_ResourceAttributeSet::GetMaxHealthAttribute()) : 0.0f;
}

float UPX_TargetStatusComponent::GetShield() const
{
	const UAbilitySystemComponent* ASC = GetTargetAbilitySystemComponent();
	return ASC ? ASC->GetNumericAttribute(UPX_ResourceAttributeSet::GetShieldAttribute()) : 0.0f;
}

float UPX_TargetStatusComponent::GetMaxShield() const
{
	const UAbilitySystemComponent* ASC = GetTargetAbilitySystemComponent();
	return ASC ? ASC->GetNumericAttribute(UPX_ResourceAttributeSet::GetMaxShieldAttribute()) : 0.0f;
}

bool UPX_TargetStatusComponent::HasTakenResourceDamage() const
{
	const float MaxHealth = GetMaxHealth();
	const float MaxShield = GetMaxShield();
	const bool bHealthDamaged = MaxHealth > KINDA_SMALL_NUMBER && GetHealth() < MaxHealth - KINDA_SMALL_NUMBER;
	const bool bShieldDamaged = MaxShield > KINDA_SMALL_NUMBER && GetShield() < MaxShield - KINDA_SMALL_NUMBER;
	return bHealthDamaged || bShieldDamaged;
}

FText UPX_TargetStatusComponent::GetDisplayName() const
{
	if ( !DisplayName.IsEmpty() )
	{
		return DisplayName;
	}

	return FText::FromString(GetNameSafe(GetOwner()));
}

FVector UPX_TargetStatusComponent::GetStatusWorldLocation() const
{
	const AActor* Owner = GetOwner();
	if ( !Owner )
	{
		return FVector::ZeroVector;
	}

	FVector Origin = FVector::ZeroVector;
	FVector Extent = FVector::ZeroVector;
	Owner->GetActorBounds(true, Origin, Extent);
	return FVector(Origin.X, Origin.Y, Origin.Z + Extent.Z + StatusLocationZOffset);
}

UAbilitySystemComponent* UPX_TargetStatusComponent::GetTargetAbilitySystemComponent() const
{
	return GetOwner() ? UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner()) : nullptr;
}

void UPX_TargetStatusComponent::SetStatusWidgetVisible(bool bVisible)
{
	InitializeStatusWidget();
	if ( !StatusWidgetComponent || !StatusWidget )
	{
		PX_LOG(Warning, TEXT("Cannot set target status widget visible. Owner: %s, WidgetClass: %s, WidgetComponent: %s, Widget: %s"),
			*GetNameSafe(GetOwner()),
			*GetNameSafe(StatusWidgetClass),
			*GetNameSafe(StatusWidgetComponent),
			*GetNameSafe(StatusWidget));
		return;
	}

	const bool bWasVisible = bStatusWidgetVisible;
	bStatusWidgetVisible = bVisible;
	StatusWidgetComponent->SetVisibility(bVisible, true);
	StatusWidgetComponent->SetHiddenInGame(!bVisible);
	StatusWidget->SetVisibility(bVisible ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Hidden);

	if ( bVisible && !bWasVisible )
	{
		RefreshStatusWidget();
	}
	else if ( bVisible )
	{
		RefreshStatusWidget();
	}
}

void UPX_TargetStatusComponent::RefreshStatusWidget()
{
	InitializeStatusWidget();
	BindAttributeDelegates();
	if ( !StatusWidget )
	{
		return;
	}

	StatusWidget->SetTargetStatus(this);
	UpdateStatusWidgetDrawSize();
}

FPXTargetCombatEntry* UPX_TargetStatusComponent::FindOrAddCombatEntry(APawn* Viewer)
{
	if ( !Viewer )
	{
		return nullptr;
	}

	return &LocalCombatEntries.FindOrAdd(Viewer);
}

bool UPX_TargetStatusComponent::IsCombatEntryActive(APawn* Viewer, FPXTargetCombatEntry& Entry) const
{
	if ( !Entry.bInCombat || !Viewer )
	{
		return false;
	}

	const UWorld* World = GetWorld();
	const double CurrentTime = World ? World->GetTimeSeconds() : 0.0;

	if ( TargetStatusType == EPXTargetStatusType::Character )
	{
		if ( CurrentTime - Entry.LastCombatTime >= CharacterCombatTimeout )
		{
			Entry.bInCombat = false;
		}

		return Entry.bInCombat;
	}

	const AActor* Owner = GetOwner();
	if ( !Owner )
	{
		Entry.bInCombat = false;
		return false;
	}

	const float Distance = FVector::Dist(Viewer->GetActorLocation(), Owner->GetActorLocation());
	if ( Distance <= MonsterCombatExitDistance )
	{
		Entry.OutOfRangeStartTime = 0.0;
		return true;
	}

	if ( Entry.OutOfRangeStartTime <= 0.0 )
	{
		Entry.OutOfRangeStartTime = CurrentTime;
		return true;
	}

	if ( CurrentTime - Entry.OutOfRangeStartTime >= MonsterCombatExitDelay )
	{
		Entry.bInCombat = false;
		Entry.OutOfRangeStartTime = 0.0;
	}

	return Entry.bInCombat;
}

void UPX_TargetStatusComponent::InitializeStatusWidget()
{
	if ( StatusWidgetComponent || !StatusWidgetClass || !GetOwner() )
	{
		if ( !StatusWidgetClass )
		{
			PX_LOG(Warning, TEXT("Missing StatusWidgetClass. Owner: %s"), *GetNameSafe(GetOwner()));
		}
		return;
	}

	StatusWidgetComponent = NewObject<UWidgetComponent>(GetOwner(), TEXT("TargetStatusWidget"));
	if ( !StatusWidgetComponent )
	{
		return;
	}

	GetOwner()->AddInstanceComponent(StatusWidgetComponent);
	StatusWidgetComponent->SetupAttachment(GetOwner()->GetRootComponent());
	StatusWidgetComponent->SetWidgetClass(StatusWidgetClass);
	StatusWidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);
	StatusWidgetComponent->SetDrawSize(StatusWidgetDrawSize);
	StatusWidgetComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	StatusWidgetComponent->SetRelativeLocation(FVector(0.0f, 0.0f, StatusLocationZOffset));
	StatusWidgetComponent->RegisterComponent();
	StatusWidgetComponent->InitWidget();

	StatusWidget = Cast<UPX_TargetHealthWidget>(StatusWidgetComponent->GetWidget());
	if ( StatusWidget )
	{
		StatusWidget->SetTargetStatus(this);
		UpdateStatusWidgetDrawSize();
	}
	else
	{
		PX_LOG(Warning, TEXT("StatusWidget is not PX_TargetHealthWidget. Owner: %s, Widget: %s"),
			*GetNameSafe(GetOwner()),
			*GetNameSafe(StatusWidgetComponent->GetWidget()));
	}
}

void UPX_TargetStatusComponent::UpdateStatusWidgetDrawSize()
{
	if ( !StatusWidgetComponent || !StatusWidget )
	{
		return;
	}

	const FVector2D DrawSize = StatusWidget->GetTargetHealthDrawSize();
	if ( DrawSize.X > KINDA_SMALL_NUMBER && DrawSize.Y > KINDA_SMALL_NUMBER )
	{
		StatusWidgetComponent->SetDrawSize(DrawSize);
	}
}

void UPX_TargetStatusComponent::BindAttributeDelegates()
{
	if ( BoundASC )
	{
		return;
	}

	BoundASC = GetTargetAbilitySystemComponent();
	if ( !BoundASC )
	{
		return;
	}

	HealthChangedHandle = BoundASC->GetGameplayAttributeValueChangeDelegate(UPX_ResourceAttributeSet::GetHealthAttribute()).AddUObject(this, &UPX_TargetStatusComponent::HandleStatusAttributeChanged);
	MaxHealthChangedHandle = BoundASC->GetGameplayAttributeValueChangeDelegate(UPX_ResourceAttributeSet::GetMaxHealthAttribute()).AddUObject(this, &UPX_TargetStatusComponent::HandleStatusAttributeChanged);
	ShieldChangedHandle = BoundASC->GetGameplayAttributeValueChangeDelegate(UPX_ResourceAttributeSet::GetShieldAttribute()).AddUObject(this, &UPX_TargetStatusComponent::HandleStatusAttributeChanged);
}

void UPX_TargetStatusComponent::UnbindAttributeDelegates()
{
	if ( !BoundASC )
	{
		return;
	}

	if ( HealthChangedHandle.IsValid() )
	{
		BoundASC->GetGameplayAttributeValueChangeDelegate(UPX_ResourceAttributeSet::GetHealthAttribute()).Remove(HealthChangedHandle);
	}
	if ( MaxHealthChangedHandle.IsValid() )
	{
		BoundASC->GetGameplayAttributeValueChangeDelegate(UPX_ResourceAttributeSet::GetMaxHealthAttribute()).Remove(MaxHealthChangedHandle);
	}
	if ( ShieldChangedHandle.IsValid() )
	{
		BoundASC->GetGameplayAttributeValueChangeDelegate(UPX_ResourceAttributeSet::GetShieldAttribute()).Remove(ShieldChangedHandle);
	}

	HealthChangedHandle.Reset();
	MaxHealthChangedHandle.Reset();
	ShieldChangedHandle.Reset();
	BoundASC = nullptr;
}

void UPX_TargetStatusComponent::HandleStatusAttributeChanged(const FOnAttributeChangeData& ChangeData)
{
	RefreshStatusWidget();
}
