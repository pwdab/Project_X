// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Project_X.h"
#include "GameFramework/Actor.h"
#include "PX_Weapon.generated.h"

class USkeletalMeshComponent;

UCLASS()
class PROJECT_X_API APX_Weapon : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APX_Weapon();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	/** The main skeletal mesh associated with this Actor */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Weapon, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USkeletalMeshComponent> Mesh;
	
public:
	FORCEINLINE USkeletalMeshComponent* GetMesh() const { return Mesh; }
};
