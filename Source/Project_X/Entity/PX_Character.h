// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Project_X.h"
#include "GameFramework/Character.h"
#include "GameplayTagContainer.h"
#include "InputActionValue.h"
#include "AbilitySystemInterface.h"
#include "PX_Character.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
class UAnimInstance;
class UCurveFloat;
class UTimelineComponent;
class UPX_CharacterAnimInstance;
class UPX_CharacterLayerAnimInstance;
class FLifetimeProperty;
//class UPX_WeaponComponent;
class UPX_WeaponSystemComponent;
class UPX_InventoryComponent;
class UPX_TargetStatusComponent;
class UPX_AbilitySystemComponent;
class UPX_ResourceAttributeSet;
class UPX_CombatAttributeSet;
class UPX_MovementAttributeSet;
struct FInputActionValue;
struct FInputActionInstance;
class UPX_InputConfigDataAsset;
class UAbilitySystemComponent;

UENUM(BlueprintType)
enum class EMoveDirection : uint8
{
	None = 0			UMETA(DisplayName = "Idle"),
	Front = 1 << 0		UMETA(DisplayName = "Front"),
	Back = 1 << 1		UMETA(DisplayName = "Back"),
	Right = 1 << 2		UMETA(DisplayName = "Right"),
	Left = 1 << 3		UMETA(DisplayName = "Left"),
};
ENUM_CLASS_FLAGS(EMoveDirection);

/* PX_GA_Aim으로 이동
UENUM(BlueprintType)
enum class EAimState : uint8
{
	Idle = 0			UMETA(DisplayName = "Idle"),
	HipFire = 1 << 1		UMETA(DisplayName = "Aim"),
	OTS = 1 << 2		UMETA(DisplayName = "OTS"),
	ADS = 1 << 3		UMETA(DisplayName = "ADS"),
};

USTRUCT(BlueprintType)
struct FAimBitSetState
{
	GENERATED_BODY()

private:
	uint8 prvBits = 0b0000;
	uint8 curBits = 0b0000;

public:
	void AddState(EAimState NewState) { prvBits = curBits;  curBits |= (static_cast<uint8>(NewState) & 0b1111);}
	void RemoveState(EAimState StateToRemove) { prvBits = curBits;  curBits &= ~(static_cast<uint8>(StateToRemove) & 0b1111); }
	//void SetState(EAimState NewState) { Bits = static_cast<uint8>(NewState) & 0b1111; }
	// return MSB
	EAimState GetState() const
	{
		uint8 Masked = curBits & 0b1111;

		if ( Masked & 0b1000 ) return EAimState::ADS;
		if ( Masked & 0b0100 ) return EAimState::OTS;
		if ( Masked & 0b0010 ) return EAimState::HipFire;
		return EAimState::Idle;
	}
	bool Has(EAimState State) const	{ return (curBits & static_cast<uint8>(State)) != 0; }

	// 최상위 비트(ADS = 1000b) 마스킹 체크
	bool IsADS() const { return (curBits & 0b1000) != 0; }
	bool IsOTS() const { return (curBits & 0b0100) != 0; }
	bool IsAiming() const { return GetState() >= EAimState::HipFire; }
	bool IsIdle() const { return curBits == 0; }
	bool ShouldBeginAim() const {
		//UE_LOG(LogTemp, Log, TEXT("Should Begin Aim ? prvBits is IDLE ?  %s, IsAiming() ? %s"), prvBits == 0b0000 ? TEXT("true") : TEXT("false"), IsAiming() ? TEXT("true") : TEXT("false"));
		//UE_LOG(LogTemp, Log, TEXT("Return %s"), prvBits == 0b0000 && IsAiming() ? TEXT("true") : TEXT("false"));
		return prvBits == 0b0000 && IsAiming(); }
	bool ShouldEndAim() const {
		//UE_LOG(LogTemp, Log, TEXT("Should End Aim ? prvBits is Aiming ?  %s, IsAiming() ? %s"), prvBits >= 0b0010 ? TEXT("true") : TEXT("false"), IsAiming() ? TEXT("true") : TEXT("false")); 
		//UE_LOG(LogTemp, Log, TEXT("Return %s"), prvBits >= 0b0010 && !IsAiming() ? TEXT("true") : TEXT("false"));
		return prvBits >= 0b0010 && !IsAiming(); }
};
*/

UCLASS()
class PROJECT_X_API APX_Character : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	APX_Character();

	void SetLayerAnimInstanceByClass(TSubclassOf<UAnimInstance> InAnimInstanceClass);
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
		

	// --- Tick-based Draw State ---------------------------------------------------------------
	bool  bWantsDraw = false;          // 입력: 지금 드로우를 "유지"하고 싶은가?
	bool  bFireOnDrawRelease = false;  // 뗄 때 Attack 실행 여부
	float DrawInputStartTime = 0.f;    // 눌린 시점 (WorldSeconds)

	/*
	// --- UserWidget -----------------------------------------------------
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	//TSubclassOf<UUserWidget> CrosshairWidgetClass;
	TSubclassOf<UUserWidget> HudRootWidgetClass;

	UPROPERTY()
	//UUserWidget* CrosshairWidget;
	UUserWidget* HudRootWidget;
	*/

	/** Replicated so we can see where remote clients are looking. */
	UPROPERTY(Replicated)
	uint8 RemoteViewYaw = 0;

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	void Server_Tick(float DeltaTime);
	void Client_Tick(float DeltatTime);
	virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PossessedBy(AController* NewController) override;

#if !UE_SERVER
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	void AbilityInputPressed(FGameplayTag InputTag);
	void AbilityInputReleased(FGameplayTag InputTag);

	// --- Input Handlers -----------------------------------------------------
	/** Move (WASD / Axis2D) */
	void BeginMove(const FInputActionValue& Value);
	void EndMove();
	EMoveDirection ResolveMoveDirection(const FVector2D& MovementVector, float DeadZone = 0.1f) const;
	/** Look (Mouse XY) */
	void Look(const FInputActionValue& Value);
	/** Jump (Space) */
	void BeginJump();
	void EndJump();
	/** Walk (LCtrl Hold) */
	void BeginWalk();
	void EndWalk();
	/** Sprint (LShift Hold) */
	void BeginSprint();
	void EndSprint();
	/** Crouch (F) */
	void BeginCrouch();
	void EndCrouch();
	/** Interact (E) */
	void Interact();
	/** Aim */
	//void BeginAim(const FInputActionValue& Value);
	//void EndAim(const FInputActionValue& Value);
	/* PX_GA_Aim으로 이동
	void BeginAim(const EAimState NewAimState);
	void EndAim(const EAimState NewAimState);
	*/
	/** OTS (RMB) */
	void BeginOTSAim(const FInputActionValue& Value);
	void EndOTSAim(const FInputActionValue& Value);
	/** ADS (RMB Hold) */
	void BeginADS(const FInputActionValue& Value);
	void EndADS(const FInputActionValue& Value);
	/** Draw (LMB Hold) */
	void BeginDraw(const FInputActionInstance& Instance);
	void EndDraw(const FInputActionInstance& Instance);
	/** Fire (LMB) */
	void FirePressed();
	void FireReleased();
	/** Reload (R) */
	void Reload();
	/** Equip Slot Weapon (X, 1, 2, 3, 4) */
	void EquipSlot(const FInputActionValue& Value);
	void EquipSlotByIndex(int32 SlotIndex);
	/** Switch Fire Mode (B) */
	void SwitchFireMode();
	/** Toggle Inventory (Tab) */
	void ToggleInventory();

private:
	// --- Server RPC Throttling Variables -----------------------------------------------------
	float LastDrawProgressSentTime = 0.f;
	float LastDrawProgressSentValue = 0.f;
#endif

	// --- Server RPCs -----------------------------------------------------
	UFUNCTION(Reliable, Server)
	void ServerBeginMove(const bool bMoved, const float Inspeed, const EMoveDirection InMoveDirection);
	UFUNCTION(Reliable, Server)
	void ServerEndMove(const bool bMoved);
	UFUNCTION(Reliable, Server)
	void ServerClearGameplayInputStateForUI();
	UFUNCTION(Reliable, Server)
	void ServerBeginJump();
	UFUNCTION(Reliable, Server)
	void ServerEndJump();
	UFUNCTION(Reliable, Server)
	void ServerBeginWalk();
	UFUNCTION(Reliable, Server)
	void ServerEndWalk();
	UFUNCTION(Reliable, Server)
	void ServerBeginSprint();
	UFUNCTION(Reliable, Server)
	void ServerEndSprint();
	UFUNCTION(Reliable, Server)
	void ServerBeginCrouch();
	UFUNCTION(Reliable, Server)
	void ServerEndCrouch();
	UFUNCTION(Reliable, Server, WithValidation)
	void ServerBeginAim(const bool bPressed);
	UFUNCTION(Reliable, Server, WithValidation)
	void ServerEndAim(const bool bPressed);
	UFUNCTION(Reliable, Server, WithValidation)
	void ServerBeginDraw(const bool bPressed);
	UFUNCTION(Reliable, Server, WithValidation)
	void ServerEndDraw(const bool bPressed);
	UFUNCTION(Unreliable, Server, WithValidation)
	void ServerUpdateDrawProgress(float Progress);
	UFUNCTION(Unreliable, Server)
	void ServerAddControllerYawInput(uint8 CompressedYaw);
	UFUNCTION(Server, Reliable)
	void ServerPlayTurnInPlace(bool bTurn180, bool bTurnRight);
	UFUNCTION(Server, Reliable)
	void ServerTurnEndSnap();

	/*
	// --- Client RPCs -----------------------------------------------------
	UFUNCTION(Reliable, NetMulticast)
	void ClientBeginWalk();
	UFUNCTION(Reliable, NetMulticast)
	void ClientEndWalk();
	UFUNCTION(Reliable, NetMulticast)
	void ClientBeginSprint();
	UFUNCTION(Reliable, NetMulticast)
	void ClientEndSprint();
	*/
	UFUNCTION(NetMulticast, Reliable)
	void MulticastPlayTurnInPlace(bool bTurn180, bool bTurnRight);

	// --- Replicated Variables -----------------------------------------------------
	UPROPERTY(ReplicatedUsing = OnRep_Jump)
	bool bIsJumping = false;
	UPROPERTY(ReplicatedUsing = OnRep_Crouch)
	bool bIsCrouching = false;
	UPROPERTY(EditAnywhere, ReplicatedUsing = OnRep_AimState)
	bool bIsAiming = false;
	UPROPERTY(ReplicatedUsing = OnRep_DrawState)
	bool bIsDrawing = false;
	UPROPERTY(ReplicatedUsing = OnRep_DrawProgress)
	float DrawProgress = 0.f;
	UPROPERTY(Replicated)
	bool bHasMoveInput = false;
	UPROPERTY(Replicated)
	float LastMoveSpeed = 0.0f;
	/** Last Direction */
	UPROPERTY(Replicated)
	EMoveDirection LastMoveDirection = EMoveDirection::None;
	UPROPERTY(Replicated)
	bool bForceDemoAimOffset = false;
	UPROPERTY(Replicated)
	float ForcedDemoAimYaw = 0.0f;
	UPROPERTY(Replicated)
	float ForcedDemoAimPitch = 0.0f;
	bool bGameplayInputBlockedForUI = false;

	// --- OnRep Functions -----------------------------------------------------
	UFUNCTION()
	void OnRep_Jump();
	UFUNCTION()
	void OnRep_Crouch();
	UFUNCTION()
	void OnRep_AimState();
	UFUNCTION()
	void OnRep_DrawState();
	UFUNCTION()
	void OnRep_DrawProgress();
	virtual void OnRep_PlayerState() override;

	// --- Timeline Callback Functions -----------------------------------------------------
	UFUNCTION()
	void AimProgressUpdate(float Alpha);

public:
	void HandleEndAim();
	void RequestTurnInPlace(float DeltaYaw);
	void RequestTurnEndSnap();
	// 무기에 따라 Attack Input Tag를 수정해 반환
	FGameplayTag ResolveAttackInputTag() const;

	FORCEINLINE UInputMappingContext* GetDefaultMappingContext() { return DefaultMappingContext; }
	/* InputConfigDataAsset로 대체
	FORCEINLINE UInputAction* GetMoveAction() { return MoveAction; }
	FORCEINLINE UInputAction* GetLookAction() { return LookAction; }
	FORCEINLINE UInputAction* GetJumpAction() { return JumpAction; }
	FORCEINLINE UInputAction* GetWalkAction() { return WalkAction; }
	FORCEINLINE UInputAction* GetSprintAction() { return SprintAction; }
	FORCEINLINE UInputAction* GetCrouchAction() { return CrouchAction; }
	FORCEINLINE UInputAction* GetInteractAction() { return InteractAction; }
	FORCEINLINE UInputAction* GetOTSAimAction() { return OTSAimAction; }
	FORCEINLINE UInputAction* GetADSAction() { return ADSAction; }
	FORCEINLINE UInputAction* GetDrawAction() { return DrawAction; }
	FORCEINLINE UInputAction* GetFireAction() { return FireAction; }
	FORCEINLINE UInputAction* GetReloadAction() { return ReloadAction; }
	FORCEINLINE UInputAction* GetEquipSlotAction() { return EquipSlotAction; }
	FORCEINLINE UInputAction* GetSwitchFireModeAction() { return SwitchFireModeAction; }
	FORCEINLINE UInputAction* GetToggleInventoryAction() { return ToggleInventoryAction; }
	*/
	FORCEINLINE UPX_InputConfigDataAsset* GetInputConfigDataAsset() { return InputConfigDataAsset; }
	FORCEINLINE UInputAction* GetEquipSlotAction() { return EquipSlotAction; }

	//FORCEINLINE UPX_WeaponComponent* GetWeaponComponent() { return WeaponComponent; }
	FORCEINLINE UPX_WeaponSystemComponent* GetWeaponSystemComponent() { return WeaponSystemComponent; }
	FORCEINLINE UPX_InventoryComponent* GetInventoryComponent() { return InventoryComponent; }
	FORCEINLINE UPX_TargetStatusComponent* GetTargetStatusComponent() { return TargetStatusComponent; }
	FORCEINLINE FRotator GetAimRotation() { FRotator Rot = FRotator(GetBaseAimRotation()); Rot.Yaw = FRotator::DecompressAxisFromByte(RemoteViewYaw); Rot.Pitch = FRotator::DecompressAxisFromByte(RemoteViewPitch); return Rot; }
	FORCEINLINE UPX_CharacterAnimInstance* GetAnimInstance() { return CachedAnimInstance; }
	FORCEINLINE UPX_CharacterLayerAnimInstance* GetLayerAnimInstance() { return CachedLayerAnimInstance; }
	FORCEINLINE bool HasMoveInput() const { return bHasMoveInput; }
	FORCEINLINE bool IsGameplayInputBlockedForUI() const { return bGameplayInputBlockedForUI; }
	FORCEINLINE bool ShouldForceDemoAimOffset() const { return bForceDemoAimOffset; }
	FORCEINLINE float GetForcedDemoAimYaw() const { return ForcedDemoAimYaw; }
	FORCEINLINE float GetForcedDemoAimPitch() const { return ForcedDemoAimPitch; }
	FORCEINLINE float GetLastMoveSpeed() { return LastMoveSpeed; }
	FORCEINLINE EMoveDirection GetLastMoveDirection() { return LastMoveDirection; }
	FORCEINLINE TObjectPtr<class UTimelineComponent> GetAimProgressTimeline() { return AimProgressTimeline; }
	//FORCEINLINE FAimBitSetState GetAimState() { return AimState; }

	//UAbilitySystemComponent* GetAbilitySystemComponent() const;

	void SetIsAiming(bool bNewIsAiming);
	void ApplyAimCameraMode(bool bWantsADS, bool bWantsOTS);
	void SetLocomotionJumping(bool bNewIsJumping);
	void SetLocomotionCrouching(bool bNewIsCrouching);
	void ApplyLocomotionSpeedMode();
	void ClearGameplayInputStateForUI();
	void SetGameplayInputBlockedForUI(bool bNewBlocked);
	void SetHasEquippedWeapon(bool bNewHasEquippedWeapon);
	void SetDemoMoveInputState(bool bMoved, float InSpeed, EMoveDirection InMoveDirection);
	void SetDemoAimYaw(float InYaw);
	void SetDemoAimRotation(const FRotator& InAimRotation);
	void SetForceDemoAimOffset(bool bNewForce);
	void ComparisonBeginMoveInput(const FVector2D& MovementVector);
	void ComparisonEndMoveInput();
	void ComparisonPressAbilityInput(FGameplayTag InputTag);
	void ComparisonReleaseAbilityInput(FGameplayTag InputTag);
	void ComparisonEquipSlot(int32 SlotIndex);

private:
	// --- Camera -------------------------------------------------------------
	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USpringArmComponent> CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> FollowCamera;

	/** FPS camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> FPSCamera;

	// TPS 시점에서 에임 보정을 위해 카메라를 캐릭터 쪽으로 5도 회전시킴
	const float CameraOffset = -0.f;

	// --- Animation ----------------------------------------------------------
	/** AnimInstance Class */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Animation, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UAnimInstance> AnimInstanceClass;
	/** AnimInstanceLayer Class */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Animation, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UAnimInstance> AnimInstanceLayerClass;
	/** Cached AnimInstance */
	UPROPERTY(Transient)
	UPX_CharacterAnimInstance* CachedAnimInstance = nullptr;
	/** Cached Layer AnimInstance */
	UPROPERTY(Transient)
	UPX_CharacterLayerAnimInstance* CachedLayerAnimInstance = nullptr;

	// --- Input --------------------------------------------------------------
	// MappingContext
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;
	/* InputConfigDataAsset로 대체
	// Move Input Action
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;
	// Look Input Action
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;
	// Jump Input Action
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* JumpAction;
	// Walk Input Action
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* WalkAction;
	// Sprint Input Action
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* SprintAction;
	// Crouch Input Action
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* CrouchAction;
	// Interact Input Action
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* InteractAction;
	// Over The Shoulder Aim Input Action
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* OTSAimAction;
	// Aim Down Sight Input Action
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* ADSAction;
	// Draw Input Action
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* DrawAction;
	// Fire Input Action
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* FireAction;
	// Reload Input Action
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* ReloadAction;
	// Equip Input Action
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* EquipSlotAction;
	// Switch Fire Mode Input Action
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* SwitchFireModeAction;
	// Toggle Inventory Input Action
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* ToggleInventoryAction;
	*/
	// Input Config Data Asset
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UPX_InputConfigDataAsset> InputConfigDataAsset;
	// Equip Input Action (특수 InputAction으로 따로 빼둠, 슬롯마다 다르게 매핑될 수 있기 때문)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* EquipSlotAction;
	// 현재 무기의 Attack Input Tag
	UPROPERTY(Transient)
	FGameplayTag CachedAttackInputTag;


	// --- Montage -----------------------------------------------------
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Montage", meta = (AllowPrivateAccess = "true"))
	UAnimMontage* Turn90Montage;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Montage", meta = (AllowPrivateAccess = "true"))
	UAnimMontage* Turn180Montage;

	// --- Locomotion --------------------------------------------------------------
	//UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Locomotion, meta = (AllowPrivateAccess = "true"))
	//bool bHasMoveInput = false;
	/** Last Move Speed */
	//UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Locomotion, meta = (AllowPrivateAccess = "true"))
	//float LastMoveSpeed = 0.0f;
	/** Last Direction */
	//UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Locomotion, meta = (AllowPrivateAccess = "true"))
	//EMoveDirection LastMoveDirection = EMoveDirection::None;

	// --- Timeline --------------------------------------------------------------
	/** Aim Progress Timeline */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Aim, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UTimelineComponent> AimProgressTimeline;
	/** Aim Progress Curve */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Aim, meta = (AllowPrivateAccess = "true"))
	UCurveFloat* AimProgressCurve;

	/** Aim State Bit Set */
	/* PX_GA_Aim으로 이동
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Aim, meta = (AllowPrivateAccess = "true"))
	FAimBitSetState AimState;
	*/

	// --- Weapon --------------------------------------------------------------
	/** Weapon Component */
	//UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = WeaponComponent, meta = (AllowPrivateAccess = "true"))
	//TObjectPtr<UPX_WeaponComponent> WeaponComponent;

	/** New unified weapon system component (Action pipeline). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = WeaponSystem, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UPX_WeaponSystemComponent> WeaponSystemComponent;

	// --- Inventory --------------------------------------------------------------
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Inventory, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UPX_InventoryComponent> InventoryComponent;

	// --- UI --------------------------------------------------------------
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = UI, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UPX_TargetStatusComponent> TargetStatusComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UPX_AbilitySystemComponent> CharacterAbilitySystemComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UPX_ResourceAttributeSet> CharacterResourceAttributeSet;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UPX_CombatAttributeSet> CharacterCombatAttributeSet;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UPX_MovementAttributeSet> CharacterMovementAttributeSet;

};
