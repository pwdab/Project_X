// Fill out your copyright notice in the Description page of Project Settings.


#include "Entity/PX_Item.h"
#include "AbilitySystem/Tags/PX_GameplayTags.h"
#include "Net/UnrealNetwork.h"
#include "Engine/ActorChannel.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Component/Inventory/PX_ItemInstance.h"
#include "Component/Inventory/PX_InventoryComponent.h"
#include "Component/Weapon/PX_WeaponDataAsset.h"

DEFINE_LOG_CATEGORY(PX_Item);

// Sets default values
APX_Item::APX_Item()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
    bReplicates = true;
    SetReplicateMovement(true);

    Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
    SetRootComponent(Mesh);
    Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    Sphere = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere"));
    Sphere->SetupAttachment(Mesh);
    Sphere->InitSphereRadius(60.f);
    Sphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    Sphere->SetCollisionResponseToAllChannels(ECR_Ignore);
    Sphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    Sphere->OnComponentBeginOverlap.AddDynamic(this, &APX_Item::OnItemBeginOverlap);
}

// Called when the game starts or when spawned
void APX_Item::BeginPlay()
{
	Super::BeginPlay();
	
    /*
    if ( HasAuthority() && !ItemInstance && ItemDataAsset )
    {
        ItemInstance = NewObject<UPX_ItemInstance>(this);
        ItemInstance->ServerInitializeFromData(ItemDataAsset, 1);
    }
    */

    if ( HasAuthority() )
    {
        NormalizeDropDataIfNeeded();
    }
}

// Called every frame
void APX_Item::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void APX_Item::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(APX_Item, ItemData);
}

/*
bool APX_Item::ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
    bool bDirty = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);

    if ( ItemInstance )
    {
        bDirty |= Channel->ReplicateSubobject(ItemInstance.Get(), *Bunch, *RepFlags);
    }

    return bDirty;
}
*/

void APX_Item::InitFromDropData(const FPX_ItemData& InData)
{
    if ( !HasAuthority() ) return;

    ItemData = InData;

    NormalizeDropDataIfNeeded();
}

void APX_Item::OnItemBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if ( !OtherActor ) return;
    if ( HasAuthority() )
    {
        PickUp(OtherActor);
    }

    // 지금은 자동 줍기 (수동으로 바꾸려면 여기서 UI 띄우고 E키로 ServerPickup 호출하도록 바꿔야 함)
}

void APX_Item::PickUp(AActor* PickerActor)
{
    //PX_LOG(Log, TEXT(""));
    ServerPickUp(PickerActor);
}

void APX_Item::NormalizeDropDataIfNeeded()
{
    if ( !HasAuthority() ) return;
    if ( !ItemData.ItemDataAsset ) return;


    if ( !ItemData.InstanceId.IsValid() )
    {
        ItemData.InstanceId = FGuid::NewGuid();
    }


    if ( ItemData.ItemDataAsset->IsWeaponItem() ) return;

    const UPX_WeaponDataAsset* WeaponDataAsset = Cast<UPX_WeaponDataAsset>(ItemData.ItemDataAsset);

    if ( !WeaponDataAsset ) return;

    if ( !ItemData.AttackModeTag.IsValid() )
    {
        ItemData.AttackModeTag = WeaponDataAsset->DefaultAttackModeTag;
    }
}

bool APX_Item::ServerPickUp_Validate(AActor* PickerActor)
{
    return true;
}

void APX_Item::ServerPickUp_Implementation(AActor* PickerActor)
{
    if ( !HasAuthority() ) return;
    if ( !PickerActor ) return;
    //if ( !ItemInstance ) return;

    //PX_LOG(Log, TEXT(""));

    UPX_InventoryComponent* Inventory = PickerActor->FindComponentByClass<UPX_InventoryComponent>();
    if ( !Inventory ) return;

    //Inventory->ServerAddItemInstance(ItemInstance);
    Inventory->ServerAddItemFromData(ItemData);

    Destroy();
}

