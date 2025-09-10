// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Project_X.h"
#include "GameFramework/Character.h"
#include "PX_Character.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
class UAnimInstance;
class UCurveFloat;
class UTimelineComponent;
class UPX_CharacterAnimInstance;
class FLifetimeProperty;
class UPX_WeaponComponent;
struct FInputActionValue;
struct FInputActionInstance;

UCLASS()
class PROJECT_X_API APX_Character : public ACharacter
{
	GENERATED_BODY()

public:
	APX_Character();

	// --- UserWidget -----------------------------------------------------
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> CrosshairWidgetClass;

	UPROPERTY()
	UUserWidget* CrosshairWidget;

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

#if !UE_SERVER
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// --- Input Handlers -----------------------------------------------------
	/** Move (WASD / Axis2D) */
	void Move(const FInputActionValue& Value);
	/** Look (Mouse XY) */
	void Look(const FInputActionValue& Value);
	/** Aim (RMB) */
	void BeginAim(const FInputActionValue& Value);
	void EndAim(const FInputActionValue& Value);
	/** Draw (LMB Hold) */
	void BeginDraw(const FInputActionInstance& Instance);
	void EndDraw(const FInputActionInstance& Instance);

private:
	// --- Server RPC Throttling Variables -----------------------------------------------------
	float LastDrawProgressSentTime = 0.f;
	float LastDrawProgressSentValue = 0.f;
#endif

	// --- Server RPCs -----------------------------------------------------
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

	// --- Replicated Variables -----------------------------------------------------
	UPROPERTY(ReplicatedUsing = OnRep_AimState)
	bool bIsAiming = false;
	UPROPERTY(ReplicatedUsing = OnRep_DrawState)
	bool bIsDrawing = false;
	UPROPERTY(ReplicatedUsing = OnRep_DrawProgress)
	float DrawProgress = 0.f;

	// --- OnRep Functions -----------------------------------------------------
	UFUNCTION()
	void OnRep_AimState();
	UFUNCTION()
	void OnRep_DrawState();
	UFUNCTION()
	void OnRep_DrawProgress();

	// --- Timeline Callback Functions -----------------------------------------------------
	UFUNCTION()
	void AimProgressUpdate(float Alpha);

public:
	FORCEINLINE UInputMappingContext* GetDefaultMappingContext() { return DefaultMappingContext; }
	FORCEINLINE UInputAction* GetMoveAction() { return MoveAction; }
	FORCEINLINE UInputAction* GetLookAction() { return LookAction; }
	FORCEINLINE UInputAction* GetAimAction() { return AimAction; }
	FORCEINLINE UInputAction* GetDrawAction() { return DrawAction; }
	FORCEINLINE UPX_WeaponComponent* GetWeapon() { return Weapon; }

private:
	// --- Camera -------------------------------------------------------------
	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USpringArmComponent> CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> FollowCamera;

	// --- Animation ----------------------------------------------------------
	/** AnimInstance Class */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Animation, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UAnimInstance> AnimInstanceClass;
	/** Cached AnimInstance */
	UPROPERTY(Transient)
	UPX_CharacterAnimInstance* CachedAnimInstance = nullptr;

	// --- Input --------------------------------------------------------------
	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;
	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;
	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;
	/** Aim Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* AimAction;
	/** Draw Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* DrawAction;

	// --- Timeline --------------------------------------------------------------
	/** Aim Progress Timeline */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Aim, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UTimelineComponent> AimProgressTimeline;
	/** Aim Progress Curve */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Aim, meta = (AllowPrivateAccess = "true"))
	UCurveFloat* AimProgressCurve;

	// --- Weapon --------------------------------------------------------------
	/** Weapon Component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Weapon, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UPX_WeaponComponent> Weapon;
};
