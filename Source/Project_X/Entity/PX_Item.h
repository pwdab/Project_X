// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Project_X.h"
#include "GameplayTagContainer.h"
#include "GameFramework/Actor.h"
#include "Component/Weapon/PX_WeaponTypes.h"
#include "PX_Item.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(PX_Item, Log, All);

class USphereComponent;
class UStaticMeshComponent;
class UPX_ItemDataAsset;
class UPX_ItemInstance;

USTRUCT(BlueprintType)
struct FPX_ItemData
{
	GENERATED_BODY()

	// PX_ItemInstance
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGuid InstanceId = FGuid();

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 Quantity = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UPX_ItemDataAsset> ItemDataAsset = nullptr;

	// PX_WeaponItemInstance
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 AmmoInMag = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	//EPXWeaponAttackMode AttackMode = EPXWeaponAttackMode::None;
	FGameplayTag AttackModeTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Durability = -1.0f;
};

UCLASS()
class PROJECT_X_API APX_Item : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APX_Item();
	void InitFromDropData(const FPX_ItemData& InData);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	//virtual bool ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags) override;

private:
	// --- Server RPCs ---------------------------------------------------
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerPickUp(AActor* PickerActor);

	// --- Server Functions ---------------------------------------------------
	void NormalizeDropDataIfNeeded();

	// --- Client Functions ---------------------------------------------------
	UFUNCTION()
	void PickUp(AActor* PickerActor);
	UFUNCTION()
	void OnItemBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	

	// --- Replicated Variables ---------------------------------------------------
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadOnly, Category = Item, meta = (AllowPrivateAccess = "true"))
	FPX_ItemData ItemData;

	// --- Component Variables ---------------------------------------------------
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Item, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> Mesh;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Item, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USphereComponent> Sphere;	
};
