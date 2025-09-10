// Fill out your copyright notice in the Description page of Project Settings.


#include "Entity/PX_Weapon.h"
#include "Components/SkeletalMeshComponent.h"  // USkeletalMeshComponent
#include "Engine/SkeletalMesh.h"               // USkeletalMesh
#include "UObject/ConstructorHelpers.h"        // FObjectFinder

// Sets default values
APX_Weapon::APX_Weapon()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Setup Skeletal Mesh
	Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	RootComponent = Mesh;

	static ConstructorHelpers::FObjectFinder<USkeletalMesh> SkeletalMeshAsset(TEXT("/Game/Project_X/Character/Weapon/SM_Bow.SM_Bow"));
	if (SkeletalMeshAsset.Succeeded())
	{
		Mesh->SetSkeletalMesh(SkeletalMeshAsset.Object);
	}

	// Setup AnimInstance Class
	static ConstructorHelpers::FClassFinder<UAnimInstance> AnimClass(TEXT("/Game/Project_X/Character/Weapon/Animation/BP_PXBow.BP_PXBow_C"));
	if (AnimClass.Class)
	{
		GetMesh()->SetAnimInstanceClass(AnimClass.Class);
	}
}

// Called when the game starts or when spawned
void APX_Weapon::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void APX_Weapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

