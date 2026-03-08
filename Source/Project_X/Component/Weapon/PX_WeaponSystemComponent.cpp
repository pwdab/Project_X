// Fill out your copyright notice in the Description page of Project Settings.

#include "PX_WeaponSystemComponent.h"
#include "Net/UnrealNetwork.h"
#include "Component/Weapon/PX_WeaponDataAsset.h"
#include "GameFramework/Character.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerState.h"
#include "Subsystem/PX_WeaponCommitSubsystem.h"
#include "Component/Inventory/PX_InventoryComponent.h"
#include "Entity/PX_Character.h"
#include "Entity/PX_Weapon.h"

DEFINE_LOG_CATEGORY(PX_WeaponSystemComponent);

UPX_WeaponSystemComponent::UPX_WeaponSystemComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    //PrimaryComponentTick.bStartWithTickEnabled = false;
    SetIsReplicatedByDefault(true);

    /*
    WeaponSlots.SetNum(5);
    static ConstructorHelpers::FObjectFinder<UPX_WeaponDataAsset> BardHandDataAsset(TEXT("/Game/Project_X/Character/Weapon/BareHand/Data/DA_PX_Weapon_BareHand.DA_PX_Weapon_BareHand"));
    if ( BardHandDataAsset.Succeeded() )
    {
        //WeaponSlots.Push(BardHandDataAsset.Object);
        WeaponSlots[0] = BardHandDataAsset.Object;
    }
    static ConstructorHelpers::FObjectFinder<UPX_WeaponDataAsset> PistolDataAsset(TEXT("/Game/Project_X/Character/Weapon/Lyra/Pistol/Data/DA_PX_Weapon_Pistol.DA_PX_Weapon_Pistol"));
    if ( PistolDataAsset.Succeeded() )
    {
        //WeaponSlots.Push(PistolDataAsset.Object);
        WeaponSlots[1] = PistolDataAsset.Object;
    }
    static ConstructorHelpers::FObjectFinder<UPX_WeaponDataAsset> RifleDataAsset(TEXT("/Game/Project_X/Character/Weapon/Lyra/Rifle/Data/DA_PX_Weapon_Rifle.DA_PX_Weapon_Rifle"));
    if ( RifleDataAsset.Succeeded() )
    {
        //WeaponSlots.Push(RifleDataAsset.Object);
        WeaponSlots[2] = RifleDataAsset.Object;
    }
    static ConstructorHelpers::FObjectFinder<UPX_WeaponDataAsset> ShotgunDataAsset(TEXT("/Game/Project_X/Character/Weapon/Lyra/Shotgun/Data/DA_PX_Weapon_Shotgun.DA_PX_Weapon_Shotgun"));
    if ( ShotgunDataAsset.Succeeded() )
    {
        //WeaponSlots.Push(ShotgunDataAsset.Object);
        WeaponSlots[3] = ShotgunDataAsset.Object;
    }
    static ConstructorHelpers::FObjectFinder<UPX_WeaponDataAsset> BowDataAsset(TEXT("/Game/Project_X/Character/Weapon/Bow/Data/DA_PX_Weapon_Bow.DA_PX_Weapon_Bow"));
    if ( BowDataAsset.Succeeded() )
    {
        //WeaponSlots.Push(BowDataAsset.Object);
        WeaponSlots[4] = BowDataAsset.Object;
    }
    */
}

void UPX_WeaponSystemComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    //DOREPLIFETIME(UPX_WeaponSystemComponent, WeaponItemInstance);
    DOREPLIFETIME(UPX_WeaponSystemComponent, CurrentWeaponSlotIndex);
    DOREPLIFETIME(UPX_WeaponSystemComponent, CurrentWeaponInstanceId);
    DOREPLIFETIME(UPX_WeaponSystemComponent, CurrentWeapon);
    //DOREPLIFETIME(UPX_WeaponSystemComponent, WeaponData);
    //DOREPLIFETIME(UPX_WeaponSystemComponent, Weapons);
}

void UPX_WeaponSystemComponent::BeginPlay()
{
    Super::BeginPlay();

    Character = Cast<ACharacter>(GetOwner());
    if ( !Character ) return;

    if ( Character->HasAuthority() )
    {
        //SpawnWeapons();
    }

    if ( Character->HasAuthority() )
    {
        if ( GetWorld() )
        {
            if ( UPX_WeaponCommitSubsystem* Subsys = GetWorld()->GetSubsystem<UPX_WeaponCommitSubsystem>() )
            {
                Subsys->Register(this);
            }
        }
    }

    APX_Character* PX_Character = Cast<APX_Character>(Character);
    if ( !PX_Character ) return;

    Inventory = PX_Character->GetInventoryComponent();

    if ( !Character->HasAuthority() && Character->IsLocallyControlled() )
    {
        if ( !Inventory ) return;

        if ( Inventory->IsInventoryReady() )
        {
            PX_LOG(Log, TEXT("Inventory is Already Ready. Not using delegate."));
            EquipWeaponBySlot(4);
        }
        else
        {
            PX_LOG(Log, TEXT("Inventory is not Ready. Using delegate."));
            Inventory->OnInventoryReady.AddUObject(this, &UPX_WeaponSystemComponent::HandleInventoryReady);
        }
    }
}

void UPX_WeaponSystemComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if ( GetWorld() && Character && Character->HasAuthority() )
    {
        if ( UPX_WeaponCommitSubsystem* Subsys = GetWorld()->GetSubsystem<UPX_WeaponCommitSubsystem>() )
        {
            Subsys->Unregister(this);
        }
    }

    Super::EndPlay(EndPlayReason);
}

void UPX_WeaponSystemComponent::TickComponent(float DeltaSeconds, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaSeconds, TickType, ThisTickFunction);

    if ( Character->HasAuthority() )
    {
        //Server_TickComponent(DeltaSeconds, TickType, ThisTickFunction);
    }

    if ( Character->IsLocallyControlled() )
    {
        //Client_TickComponent(DeltaSeconds, TickType, ThisTickFunction);
    }
}

/*
void UPX_WeaponSystemComponent::SpawnWeapons()
{
    // Spawn and SAve
    if ( !Character || !Character->HasAuthority() ) return;   // ✅ 서버에서만 스폰
    UWorld* World = GetWorld();
    if ( !World ) return;

    //Weapons.SetNum(WeaponSlots.Num());
    Weapons.SetNum(5);

    for ( int32 i = 0; i < WeaponSlots.Num(); ++i )
    {
        UPX_WeaponDataAsset* DA = WeaponSlots[i].Get();
        if ( !DA ) continue;

        PX_LOG(Log, TEXT("WeaponSlots.Num() : %d, Current Index : %d"), WeaponSlots.Num(), i);

        // ✅ DataAsset에 무기 클래스가 있어야 함 (없으면 아래 참고)
        //if ( !DA->WeaponClass ) continue; // TSubclassOf<APX_Weapon> WeaponClass;

        FActorSpawnParameters Params;
        Params.Owner = GetOwner(); // 중요: 오너 지정
        Params.Instigator = Cast<APawn>(GetOwner());
        Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

        APX_Weapon* Wpn = World->SpawnActor<APX_Weapon>(APX_Weapon::StaticClass(), FTransform::Identity, Params);
        if ( !Wpn ) continue;

        // 데이터 주입
        Wpn->InitializeFromData(DA);

        // (선택) 캐릭터/무기 소켓에 부착
        // Wpn->AttachToComponent(CharacterMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("WeaponSocket"));

        Weapons[i] = Wpn;
        PX_LOG(Log, TEXT("Weapon Spawn Complete."));
    }
}
*/

/*
const UObject* UPX_WeaponSystemComponent::GetKeyFromContext(const FPXWeaponActionContext& ActionContext) const
{
    const APawn* Pawn = Cast<APawn>(ActionContext.Owner.Get());
    if ( !Pawn ) return nullptr;

    // Player
    if ( const APlayerState* PlayerState = Pawn->GetPlayerState() )
    {
        return Cast<const UObject>(PlayerState);
    }
    // AI
    return Cast<const UObject>(Pawn);
}
*/