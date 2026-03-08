// Fill out your copyright notice in the Description page of Project Settings.


#include "Entity/PX_Weapon.h"
#include "Net/UnrealNetwork.h"						// Replication
#include "Components/SkeletalMeshComponent.h"		// USkeletalMeshComponent
#include "Component/Weapon/PX_WeaponDataAsset.h"	// WeaponDataAsset

DEFINE_LOG_CATEGORY(PX_Weapon);

// Sets default values
APX_Weapon::APX_Weapon()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	//PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Weapon Mesh"));
	SetRootComponent(Mesh);
	//Mesh->SetupAttachment(Root);
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Mesh->SetGenerateOverlapEvents(false);

	//WeaponData = CreateDefaultSubobject<UPX_WeaponDataAsset>(TEXT("Weapon Dta"));
	//Mesh->SetupAttachment(Mesh);
}

// Called when the game starts or when spawned
void APX_Weapon::BeginPlay()
{
	Super::BeginPlay();

	/*
	if ( !WeaponData ) return;
	InitializeFromData();
	*/
}

void APX_Weapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	//DOREPLIFETIME(APX_Weapon, SlotIndex);
	DOREPLIFETIME(APX_Weapon, WeaponData);
}

// Called every frame
void APX_Weapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

/*
void APX_Weapon::InitializeFromData()
{
	// 클라이언트 Init
	if ( HasAuthority() )	return;
	if ( !WeaponData ) return;
	if ( SlotIndex == INDEX_NONE ) return;
	if ( !Mesh ) return;

	if ( WeaponData->WeaponMesh )
	{
		Mesh->SetSkeletalMesh(WeaponData->WeaponMesh);
	}

	if ( WeaponData->WeaponAnimClass )
	{
		Mesh->SetAnimInstanceClass(WeaponData->WeaponAnimClass);
	}
}
*/

//void APX_Weapon::InitializeFromData(UPX_WeaponDataAsset* InWeaponData, int32 InSlotIndex)
void APX_Weapon::InitializeFromData(UPX_WeaponDataAsset* InWeaponData)
{
	if ( !HasAuthority() )	return;
	if ( !InWeaponData ) return;
	//if ( InSlotIndex == INDEX_NONE ) return;
	//PX_LOG(Log, TEXT("InSlotIndex : %d"), InSlotIndex);

	WeaponData = InWeaponData;
	//SlotIndex = InSlotIndex;

	//InitializeFromData();

	//PX_LOG(Log, TEXT("Initialize Server Side From Data Complete."));

	OnRep_WeaponDataUpdated();
}

void APX_Weapon::OnRep_WeaponDataUpdated()
{
	if ( !Mesh || !WeaponData ) return;

	if ( WeaponData->WeaponMesh ) Mesh->SetSkeletalMesh(WeaponData->WeaponMesh);
	if ( WeaponData->WeaponAnimClass ) Mesh->SetAnimInstanceClass(WeaponData->WeaponAnimClass);

	//PX_LOG(Log, TEXT("Weapon's Mesh and Anim Setup Complete."));
}

/*
void APX_Weapon::OnRep_SlotIndexUpdated()
{
	if ( !WeaponSlots.IsValidIndex(SlotIndex) )
	{
		WeaponData = nullptr;
		return;
	}

	WeaponData = WeaponSlots[SlotIndex];

	// 필요하면 WeaponActor도 동기화
	if ( Weapons.IsValidIndex(SlotIndex) )
	{
		APX_Weapon* NewWeapon = Weapons[SlotIndex];

		// Attach 처리
		if ( Character && NewWeapon )
		{
			NewWeapon->AttachToComponent(
				Character->GetMesh(),
				FAttachmentTransformRules::SnapToTargetNotIncludingScale,
				CurrentWeaponData->AttachSocketName
			);
		}
	}

	// ActionHandler 교체 필요하면 여기서
	if ( CurrentWeaponData )
	{
		SetActionHandler(CurrentWeaponData->ActionHandlerClass);
	}

	InitializeFromData(); // 클라에서 Mesh/AnimClass 세팅
}
*/

/*
void APX_Weapon::ServerSwitchFireMode()
{
	if ( !WeaponData || WeaponData->SupportedAttackModes.Num() == 0 ) return;

	const int32 CurrentIndex = WeaponData->SupportedAttackModes.IndexOfByKey(AttackMode);
	const int32 NextIndex = (CurrentIndex == INDEX_NONE) ? 0 : (CurrentIndex + 1) % WeaponData->SupportedAttackModes.Num();

	ServerSetFireMode(WeaponData->SupportedAttackModes[NextIndex]);
}

void APX_Weapon::ServerSetFireMode(EPXWeaponAttackMode NewMode)
{
	if ( !HasAuthority() )	return;
	if ( AttackMode == NewMode ) return;
	if ( !WeaponData || !WeaponData->SupportedAttackModes.Contains(NewMode) ) return;
	PX_LOG(Log, TEXT("SetFireMode %s -> %s"), WeaponAttackModeToString(AttackMode), WeaponAttackModeToString(NewMode));

	AttackMode = NewMode;
}



void APX_Weapon::ServerSetAmmo(int32 Amount)
{
	if ( !HasAuthority() )	return;

	AmmoInMag = Amount;
}

void APX_Weapon::ServerConsumeAmmo(int32 Amount)
{
	if ( !HasAuthority() )	return;

	AmmoInMag = FMath::Max(0, AmmoInMag - Amount);
}

void APX_Weapon::OnRep_AmmoUpdated()
{

}

void APX_Weapon::OnRep_AttackModeUpdated()
{

}
*/