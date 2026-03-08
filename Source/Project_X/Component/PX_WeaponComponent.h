// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Project_X.h"
#include "Components/SceneComponent.h"
#include "PX_WeaponComponent.generated.h"

class APX_Weapon;
class APX_Projectile;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECT_X_API UPX_WeaponComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UPX_WeaponComponent();

	UFUNCTION(BlueprintCallable, Category = WeaponComponent)
	void SetProjectileClass(TSubclassOf<APX_Projectile> NewProjectileClass);

	void Attack();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	// --- Server RPCs -----------------------------------------------------
	UFUNCTION(Reliable, Server, WithValidation)
	void ServerAttack(const FVector_NetQuantize ClientMuzzleLoc, const FVector_NetQuantize ClientCamLoc, const FVector_NetQuantize ClientTargetPoint);

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	FORCEINLINE APX_Weapon* GetWeapon() { return Weapon; }

private:
	/** Weapon */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Weapon, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<APX_Weapon> Weapon;

	/** The Projectile Class being used for Weapon */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = WeaponComponent, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<APX_Projectile> ProjectileClass;
};
