// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Project_X.h"
#include "GameFramework/Actor.h"
#include "PX_Projectile.generated.h"

class USphereComponent;
class UProjectileMovementComponent;
class UPX_WeaponComponent;					// Debug Camera
class UGameplayEffect;

UCLASS()
class PROJECT_X_API APX_Projectile : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	APX_Projectile();

	// --- Debug Camera -------------------------------------------------------------
	// Transition Camera to Target Actor
	UFUNCTION(Client, Reliable)
	void ClientCameraTransition(AActor* TargetActor);


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	UFUNCTION()
	void OnHit(UPrimitiveComponent* Hitcomponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	/** The SphereComponent being used for movement collision */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Projectile, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USphereComponent> SphereCollision;

	/** The main static mesh associated with this Actor */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Projectile, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> Mesh;

	/** Projectile Movement component used for projectile movement logic */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Projectile, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement;

	// --- Debug Camera -------------------------------------------------------------
	/** Camera boom positioning the camera behind the Actor */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Projectile, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom = nullptr;

	/** Follow camera */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Projectile, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera = nullptr;

	// --- Replicated Variables -----------------------------------------------------
	UPROPERTY(ReplicatedUsing = OnRep_DisableProjectile)
	bool bDisableProjectile = false;

	// --- OnRep Functions -----------------------------------------------------
	UFUNCTION()
	void OnRep_DisableProjectile();

	void DisableProjectile();

	bool ApplyDamageGameplayEffect(AActor* TargetActor);
	void ApplyStatusGameplayEffects(AActor* TargetActor);

public:
	FORCEINLINE USphereComponent* GetSphereCollision() const { return SphereCollision; }
	FORCEINLINE UStaticMeshComponent* GetMesh() const { return Mesh; }
	FORCEINLINE UProjectileMovementComponent* GetProjectileMovement() const { return ProjectileMovement; }
	void SetStatusGameplayEffectClasses(const TArray<TSubclassOf<UGameplayEffect>>& InStatusGameplayEffectClasses);

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile|Damage")
	float DamageAmount = 20.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile|Damage")
	TSubclassOf<UGameplayEffect> DamageGameplayEffectClass;

	UPROPERTY(Transient)
	TArray<TSubclassOf<UGameplayEffect>> StatusGameplayEffectClasses;

};
