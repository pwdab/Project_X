#pragma once

#include "Project_X.h"
#include "Abilities/GameplayAbilityTargetTypes.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "TimerManager.h"
#include "PX_DemoBotComponent.generated.h"

class APX_Character;
class UAbilitySystemComponent;
class UAnimInstance;
class UAnimMontage;
class UPX_InventoryComponent;
class UPX_ItemDataAsset;
class UPX_WeaponSystemComponent;

UENUM(BlueprintType)
enum class EPXDemoBotBehavior : uint8
{
	StrafeLoop UMETA(DisplayName = "Strafe Loop"),
	EquipLoop UMETA(DisplayName = "Equip Loop"),
	FireReloadLoop UMETA(DisplayName = "Fire Reload Loop"),
	RifleJumpFiveShotLoop UMETA(DisplayName = "Rifle Jump Five Shot Loop"),
};

UCLASS(ClassGroup=(PX), meta=(BlueprintSpawnableComponent))
class PROJECT_X_API UPX_DemoBotComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPX_DemoBotComponent();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category = "PX|Demo Bot")
	void StartDemo();

	UFUNCTION(BlueprintCallable, Category = "PX|Demo Bot")
	void StopDemo();

	UAnimMontage* GetBotWeaponFireMontage() const { return BotWeaponFireMontage; }
	UAnimMontage* GetBotCharacterReloadMontage() const { return BotCharacterReloadMontage; }
	UAnimMontage* GetBotWeaponReloadMontage() const { return BotWeaponReloadMontage; }
	bool ShouldForceAimingPose() const { return Behavior == EPXDemoBotBehavior::RifleJumpFiveShotLoop; }
	void EnsureCombatAnimLayer();

private:
	bool CacheOwner();
	bool CanRunDemo() const;
	void ClearDemoTimers();
	bool EnsureDemoInventory();
	bool IsStunned() const;
	void BindStunTagEvent();
	void UnbindStunTagEvent();
	void HandleStunTagChanged(const FGameplayTag ChangedTag, int32 NewCount);
	void CancelActiveCombatAction();
	void ApplyStrafeAnimLayer();

	void StartStrafeLoop();
	void TickStrafeLoop(float DeltaTime);
	void StopStrafeLoop();

	void StartEquipLoop();
	void EquipSelectedSlot();
	void EquipBareHands();
	bool TryEquipSlot(int32 SlotIndex);
	void FinishEquipAction();

	void StartFireReloadLoop();
	void EquipCombatSlot();
	void StartFirePhase();
	void FireOnceForDemo();
	void FinishFireOnceForDemo();
	void StartReloadPhase();
	void FinishReloadPhase();
	void StartRifleJumpFiveShotLoop();
	void StartRifleJumpReloadPhase();
	void FinishRifleJumpReloadPhase();
	void StartRifleJumpAttackPhase();
	void FireRifleJumpShot();
	void FinishRifleJumpShot();
	void FinishRifleJumpAttackPhase();
	APX_Character* FindNearestTargetCharacter() const;
	bool UpdateAimAtTarget();
	bool MakeTargetedFireData(FGameplayAbilityTargetDataHandle& OutTargetData) const;
	FVector GetTargetAimPoint(const APX_Character* TargetCharacter) const;

	UPROPERTY(EditAnywhere, Category = "PX|Demo Bot")
	EPXDemoBotBehavior Behavior = EPXDemoBotBehavior::StrafeLoop;

	UPROPERTY(EditAnywhere, Category = "PX|Demo Bot")
	bool bAutoStart = true;

	UPROPERTY(EditAnywhere, Category = "PX|Demo Bot", meta = (ClampMin = "0.0"))
	float StartDelay = 0.0f;

	UPROPERTY(EditAnywhere, Category = "PX|Demo Bot|Strafe", meta = (ClampMin = "0.0"))
	float StrafeDistance = 300.0f;

	UPROPERTY(EditAnywhere, Category = "PX|Demo Bot|Strafe", meta = (ClampMin = "0.0"))
	float StrafeSpeed = 90.0f;

	UPROPERTY(EditAnywhere, Category = "PX|Demo Bot|Strafe")
	bool bFaceInitialDirectionWhileStrafing = true;

	UPROPERTY(EditAnywhere, Category = "PX|Demo Bot|Strafe")
	TSubclassOf<UAnimInstance> StrafeAnimInstanceClass;

	UPROPERTY(EditAnywhere, Category = "PX|Demo Bot|Strafe")
	TSubclassOf<UAnimInstance> StrafeBaseAnimInstanceClass;

	UPROPERTY(EditAnywhere, Category = "PX|Demo Bot|Equip", meta = (ClampMin = "0"))
	int32 EquipSlotIndex = 0;

	UPROPERTY(EditAnywhere, Category = "PX|Demo Bot|Equip", meta = (ClampMin = "0"))
	int32 BareHandSlotIndex = 4;

	UPROPERTY(EditAnywhere, Category = "PX|Demo Bot|Equip")
	TObjectPtr<UPX_ItemDataAsset> EquipItemDataAsset = nullptr;

	UPROPERTY(EditAnywhere, Category = "PX|Demo Bot|Equip", meta = (ClampMin = "0.0"))
	float EquipInterval = 2.0f;

	UPROPERTY(EditAnywhere, Category = "PX|Demo Bot|Equip", meta = (ClampMin = "0.0"))
	float UnequipInterval = 2.0f;

	UPROPERTY(EditAnywhere, Category = "PX|Demo Bot|Equip", meta = (ClampMin = "0.0"))
	float EquipActionDuration = 0.7f;

	UPROPERTY(EditAnywhere, Category = "PX|Demo Bot|Combat", meta = (ClampMin = "0"))
	int32 CombatSlotIndex = 0;

	UPROPERTY(EditAnywhere, Category = "PX|Demo Bot|Combat")
	TObjectPtr<UPX_ItemDataAsset> CombatItemDataAsset = nullptr;

	UPROPERTY(EditAnywhere, Category = "PX|Demo Bot|Combat", meta = (ClampMin = "0.0"))
	float FireDuration = 2.0f;

	UPROPERTY(EditAnywhere, Category = "PX|Demo Bot|Combat", meta = (ClampMin = "0.02"))
	float FireInterval = 0.12f;

	UPROPERTY(EditAnywhere, Category = "PX|Demo Bot|Combat", meta = (ClampMin = "0.0"))
	float ReloadDuration = 1.5f;

	UPROPERTY(EditAnywhere, Category = "PX|Demo Bot|Combat", meta = (ClampMin = "0.0"))
	float PostReloadDelay = 0.2f;

	UPROPERTY(EditAnywhere, Category = "PX|Demo Bot|Combat")
	bool bFaceInitialDirectionWhileAttacking = true;

	UPROPERTY(EditAnywhere, Category = "PX|Demo Bot|Combat")
	TObjectPtr<UAnimMontage> BotWeaponFireMontage = nullptr;

	UPROPERTY(EditAnywhere, Category = "PX|Demo Bot|Combat")
	TObjectPtr<UAnimMontage> BotCharacterReloadMontage = nullptr;

	UPROPERTY(EditAnywhere, Category = "PX|Demo Bot|Combat")
	TObjectPtr<UAnimMontage> BotWeaponReloadMontage = nullptr;

	UPROPERTY(EditAnywhere, Category = "PX|Demo Bot|Rifle Jump Attack", meta = (ClampMin = "1"))
	int32 JumpAttackShotCount = 5;

	UPROPERTY(EditAnywhere, Category = "PX|Demo Bot|Rifle Jump Attack", meta = (ClampMin = "0.02"))
	float JumpAttackFireInterval = 0.12f;

	UPROPERTY(EditAnywhere, Category = "PX|Demo Bot|Rifle Jump Attack", meta = (ClampMin = "0.0"))
	float PostJumpAttackDelay = 2.0f;

	UPROPERTY(EditAnywhere, Category = "PX|Demo Bot|Rifle Jump Attack", meta = (ClampMin = "0.0"))
	float JumpAttackJumpZVelocity = 700.0f;

	UPROPERTY(EditAnywhere, Category = "PX|Demo Bot|Rifle Jump Attack", meta = (ClampMin = "0.0", DisplayName = "Recognition Radius"))
	float TargetSearchRadius = 2500.0f;

	UPROPERTY(EditAnywhere, Category = "PX|Demo Bot|Rifle Jump Attack")
	FVector TargetAimOffset = FVector(0.0f, 0.0f, 70.0f);

	UPROPERTY(Transient)
	TObjectPtr<APX_Character> OwnerCharacter = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<APX_Character> TargetCharacter = nullptr;

	UPROPERTY(Transient)
	TSubclassOf<UAnimInstance> EnsuredCombatAnimLayerClass;

	UPROPERTY(Transient)
	TObjectPtr<UPX_WeaponSystemComponent> WeaponSystem = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UPX_InventoryComponent> Inventory = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UAbilitySystemComponent> AbilitySystem = nullptr;

	bool bRunning = false;
	bool bPausedByStun = false;
	bool bFireOnceInProgress = false;
	bool bReloadInProgress = false;
	bool bOriginalUseControllerRotationYaw = false;
	bool bOriginalOrientRotationToMovement = true;
	bool bOriginalUseControllerDesiredRotation = false;
	bool bStrafeMovementSettingsApplied = false;
	int32 StrafeDirection = 1;
	int32 RemainingJumpAttackShots = 0;
	float FirePhaseEndTime = 0.0f;
	float OriginalMaxWalkSpeed = 0.0f;
	FVector StrafeOrigin = FVector::ZeroVector;
	FVector StrafeAxis = FVector::RightVector;
	FRotator InitialActorRotation = FRotator::ZeroRotator;

	FTimerHandle StartTimerHandle;
	FTimerHandle EquipLoopTimerHandle;
	FTimerHandle EquipEndTimerHandle;
	FTimerHandle FireLoopTimerHandle;
	FTimerHandle FireEndTimerHandle;
	FTimerHandle ReloadTimerHandle;
	FDelegateHandle StunTagChangedHandle;
};
