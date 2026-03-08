// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Project_X.h"
#include "GameFramework/Actor.h"
//#include "Component/Weapon/PX_WeaponDataAsset.h"
#include "PX_Weapon.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(PX_Weapon, Log, All);

class UPX_WeaponDataAsset;
//enum class EPXWeaponAttackMode;
class USkeletalMeshComponent;
class UAnimInstance;

UCLASS()
class PROJECT_X_API APX_Weapon : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APX_Weapon();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// --- Server RPCs -----------------------------------------------------

	// ---  Functions -------------------------------------------------
	// Initialize this weapon's from data asset.
	//void InitializeFromData();
	//void InitializeFromData(UPX_WeaponDataAsset* InWeaponData, int32 InSlotIndex);			// PX_WeaponSystemComponent::ServerEquipBySlot
	void InitializeFromData(UPX_WeaponDataAsset* InWeaponData);

	/*
	// --- Server Functions -----------------------------------------------------
	// Set equipped weapon's fire mode to next mode.
	void ServerSwitchFireMode();
	// Set equipped weapon's fire mode to certain new mode.
	void ServerSetFireMode(EPXWeaponAttackMode NewMode);
	void ServerSetAmmo(int32 Amount);
	void ServerConsumeAmmo(int32 Amount);
	*/

	// --- Getters -------------------------------------------------
	FORCEINLINE USkeletalMeshComponent* GetMesh() const { return Mesh; }
	FORCEINLINE UAnimInstance* GetAnimInstance() const { return Mesh ? Mesh->GetAnimInstance() : nullptr; }
	//FORCEINLINE int32 GetSlotIndex() const { return SlotIndex;  }

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;



private:
	// --- Server Functions -----------------------------------------------------
	//void InitializeFromData(UPX_WeaponDataAsset* InWeaponData, int32 InSlotIndex);
	//void InitializeFromData(UPX_WeaponDataAsset* InWeaponData);

	// --- OnRep Functions -----------------------------------------------------
	/*
	UFUNCTION()
	void OnRep_SlotIndexUpdated();
	*/
	UFUNCTION()
	void OnRep_WeaponDataUpdated();

	// --- Weapon Variables ---------------------------------------------------
	// --- Replicated Variable ---------------------------------------------------
	/** Data that initialized this weapon (non-authoritative reference, mainly for debug). */
	UPROPERTY(ReplicatedUsing = OnRep_WeaponDataUpdated, VisibleInstanceOnly)
	TObjectPtr<UPX_WeaponDataAsset> WeaponData = nullptr;
	/*
	UPROPERTY(ReplicatedUsing = OnRep_SlotIndexUpdated, VisibleInstanceOnly)
	int32 SlotIndex = INDEX_NONE;
	*/
	

	// --- Weapon Variables ---------------------------------------------------
	/** The main skeletal mesh associated with this Actor */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Weapon, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USkeletalMeshComponent> Mesh;
};
