// Fill out your copyright notice in the Description page of Project Settings.

#include "../PX_WeaponSystemComponent.h"
#include "../PX_WeaponDataAsset.h"

#include "Handler/PX_WeaponActionHandler.h"
#include "Entity/PX_Character.h"
#include "Entity/PX_Weapon.h"                   // EquipWeapon

#include "Handler/PX_WeaponActionHandler.h"
#include "Driver/PX_WeaponAnimDriver.h"

#include "GameFramework/PlayerState.h"          // ClientSubmitAction

#include "Component/Inventory/PX_WeaponItemInstance.h"
#include "Component/Inventory/PX_ItemDataAsset.h"
#include "Component/Inventory//PX_InventoryComponent.h"


/*
void UPX_WeaponSystemComponent::Client_TickComponent(float DeltaSeconds, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
}
*/

/*
void UPX_WeaponSystemComponent::EquipWeapon(APX_Weapon* InWeapon)
{
    //PX_LOG(Log, TEXT(""));
    if ( !Character || Character->HasAuthority() || !Character->IsLocallyControlled() ) return;
    ServerEquipWeapon(InWeapon);
}

void UPX_WeaponSystemComponent::EquipWeapon(UPX_WeaponDataAsset* NewWeaponData)
{
    //PX_LOG(Log, TEXT(""));
    if ( !Character || Character->HasAuthority() || !Character->IsLocallyControlled() ) return;
    ServerEquipWeaponWithData(NewWeaponData);
}
*/

// PX_InventoryComponent의 OnInventoryReady 델리게이트 Handler
void UPX_WeaponSystemComponent::HandleInventoryReady()
{
    PX_LOG(Log, TEXT(""));
    if ( !Character ) return;

    APX_Character* PX_Character = Cast<APX_Character>(Character);
    if ( !PX_Character ) return;
    
    Inventory = PX_Character->GetInventoryComponent();
    if ( !Inventory ) return;

    // 구독 해제
    PX_Character->GetInventoryComponent()->OnInventoryReady.RemoveAll(this);

    EquipWeaponBySlot(4);
}

// PX_WeaponItemInstance의 OnAmmoUpdated 델리게이트 Handler
void UPX_WeaponSystemComponent::HandleAmmoUpdated(int32 InAmmoInMag, int32 InReserved)
{
    if ( CurrentWeaponSlotIndex == 4 ) return;  // BareHand
    OnCurrentAmmoUpdated.Broadcast(InAmmoInMag, InReserved);
}

// PX_WeaponItemInstance의 OnAttackModeUpdated 델리게이트 Handler
void UPX_WeaponSystemComponent::HandleAttackModeUpdated(EPXWeaponAttackMode InAttackMode)
{
    //PX_LOG(Log, TEXT("AttackMode: %s"), WeaponAttackModeToString(InAttackMode));
    OnAttackModeUpdated.Broadcast(InAttackMode);
}

void UPX_WeaponSystemComponent::EquipWeaponBySlot(int32 SlotIndex)
{
    //PX_LOG(Log, TEXT(""));
    if ( !Character || Character->HasAuthority() || !Character->IsLocallyControlled() ) return;
    ServerEquipBySlot(SlotIndex);

    //if ( !WeaponSlots.IsValidIndex(SlotIndex) ) return;

    //EquipWeapon(WeaponSlots[SlotIndex]);
    //EquipWeapon(Weapons[SlotIndex]);
}

void UPX_WeaponSystemComponent::ClientSubmitAction(FPXWeaponActionContext ActionContext)
{
    //PX_LOG(Log, TEXT(""));
    if ( !Character || Character->HasAuthority() || !Character->IsLocallyControlled() ) return;
    if ( ActionContext.Type == EPXWeaponActionType::None ) return;

    //PX_LOG(Log, TEXT(""));
    ActionContext.InstigatorDebugName = FString::Printf(TEXT("%s(%d)"), *GetOwner()->GetName(), Character->GetPlayerState()->GetPlayerId());

    // 클라이언트 상태 갱신
    //if ( !ClientUpdateState(ActionContext) ) return;
    
    // 
    // Debug Name
    if ( APlayerState* PlayerState = Character->GetPlayerState() )
    {
        ActionContext.InstigatorDebugName = PlayerState->GetPlayerName();
        ActionContext.InstigatorId = PlayerState->GetPlayerId();
    }
    else
    {
        ActionContext.InstigatorDebugName = Character->GetName();
        ActionContext.InstigatorId = 0;
    }
    ActionContext.ClientSequence = ClientSequence++;

    PX_LOG(Log, TEXT("ClientSeq : # %d, ActionContext : %s"), ClientSequence, *ActionContext.ToString());

    ServerSubmitAction(ActionContext);
}

bool UPX_WeaponSystemComponent::ClientUpdateState(const FPXWeaponActionContext& ActionContext)
{
    //PX_LOG(Log, TEXT(""));
    if ( !Character || Character->HasAuthority() ) return false;
//    if ( !Weapon || !WeaponData ) return false;
    if ( !CurrentWeapon ) return false;
    //PX_LOG(Log, TEXT("Weapon is not null"));
    if ( !CurrentWeaponItemInstance ) return false;
    //PX_LOG(Log, TEXT("WeaponData is not null"));
    //if ( !ActionContext ) return;
    //PX_LOG(Log, TEXT(""));

    // 현재 무기의 공격 모드
    //EPXWeaponAttackMode AttackMode = Weapon ? (Weapon->GetFireMode()) : (WeaponData ? WeaponData->DefaultAttackMode : EPXWeaponAttackMode::Single);
    EPXWeaponAttackMode AttackMode = CurrentWeaponItemInstance->GetAttackMode();
    //PX_LOG(Log, TEXT("Attack Mode : %s"), WeaponAttackModeToString(AttackMode));
    switch ( ActionContext.Type )
    {
    case EPXWeaponActionType::AttackPressed:
    {
        //PX_LOG(Log, TEXT("Client Attack Pressed. bIsReloading_Local %s -> %s"), bIsReloading_Local ? TEXT("true") : TEXT("false"), !bIsReloading_Local ? TEXT("true") : TEXT("false"));
        if ( bIsReloading_Local ) return false;
        if ( AttackMode != EPXWeaponAttackMode::Burst || BurstRemaining_Local == 0 )
        {
            bAttackPressed_Local = true;
        }

        // Set BurstRemaining if Attack Mode is Burst.
        if ( AttackMode == EPXWeaponAttackMode::Burst && BurstRemaining_Local == 0 )
        {
            // 나중에 BurstCount와 인벤토리 내 남은 탄 개수를 비교해야 함
            BurstRemaining_Local = FMath::Max(1, CurrentWeaponItemInstance->GetItemDataAsset()->WeaponData->BurstCount);
        }
        break;
    }

    case EPXWeaponActionType::AttackReleased:
    {
        // Burst는 BurestRemaining이 0이 될때까지 AttackReleased 하지 않음
        if ( AttackMode != EPXWeaponAttackMode::Burst )
        {
            //PX_LOG(Log, TEXT("bAttackPressed_Server %s -> %s"), bAttackPressed_Server ? TEXT("true") : TEXT("false"), !bAttackPressed_Server ? TEXT("true") : TEXT("false"));
            bAttackPressed_Local = false;
        }
        break;
    }

    case EPXWeaponActionType::BeginReload:
    {
        if ( bIsReloading_Local ) return false;
        if ( CurrentWeaponItemInstance->GetAmmo() >= CurrentWeaponItemInstance->GetItemDataAsset()->WeaponData->MagSize ) return false;

        UWorld* World = GetWorld();
        if ( !World ) return false;

        // 점사 중일 때 재장전을 누르면 발사를 취소하고 바로 재장전
        //PX_LOG(Log, TEXT("Client Begin Reload. bIsReloading_Local %s -> %s"), bIsReloading_Local ? TEXT("true") : TEXT("false"), !bIsReloading_Local ? TEXT("true") : TEXT("false"));
        //bAttackPressed_Local = false;
        bIsReloading_Local = true;
        //BurstRemaining_Local = 0;
        break;
    }

    case EPXWeaponActionType::EndReload:
    {
        if ( !bIsReloading_Local ) return false;

        //PX_LOG(Log, TEXT("Client End Reload. bIsReloading_Local %s -> %s"), bIsReloading_Local ? TEXT("true") : TEXT("false"), !bIsReloading_Local ? TEXT("true") : TEXT("false"));
        bIsReloading_Local = false;
        break;
    }

    case EPXWeaponActionType::SetFireMode:
    {
        if ( bIsReloading_Local ) return false;

        bAttackPressed_Local = false;

        //PX_LOG(Log, TEXT("Client End Reload. bIsReloading_Local %s -> %s"), bIsReloading_Local ? TEXT("true") : TEXT("false"), !bIsReloading_Local ? TEXT("true") : TEXT("false"));
        break;
    }

    default:
        break;
    }

    return true;
}

void UPX_WeaponSystemComponent::ClientCommitAction_Implementation(const FPXWeaponActionContext& ActionContext, bool bActionApproved)
{
    //PX_LOG(Log, TEXT(""));
    //if ( !CurrentActionHandler ) return;

    PX_LOG(Log, TEXT("ActionApproved : %s, ActionContext : %s"), bActionApproved ? TEXT("true") : TEXT("false"), *ActionContext.ToString());
    //PX_LOG(Log, TEXT("ActionContext : %s, bShouldDryFire : %s"), *ActionContext.ToString(), ActionContext.bShouldDryFire ? TEXT("true") : TEXT("false"));
    //PX_LOG(Log, TEXT("bShouldDryFire : %s"), ActionContext.bShouldDryFire ? TEXT("true") : TEXT("false"));
    

    // 클라이언트 상태 갱신
    ClientUpdateState(ActionContext);

    // BeginEquip은 ClientBuildCosmetics 완료 후 재생
    if ( ActionContext.Type == EPXWeaponActionType::BeginEquip )
    {
        const bool bIsCosmeticReady = CurrentActionHandler && CurrentAnimDriver && (CurrentWeaponSlotIndex == ActionContext.SlotIndex);

        /*
        if ( !bIsCosmeticReady )
        {
            PX_LOG(Log, TEXT("Client Cosmetics is not ready yet. Pending Action."));
        }
        else
        {
            PX_LOG(Log, TEXT("ActionContext : %s"), *ActionContext.ToString());
        }
        */

        if ( !bIsCosmeticReady )
        {
            //PX_LOG(Log, TEXT("Client Cosmetics is not ready yet. Pending Action."));
            PendingCosmeticAction = ActionContext;
            return;
        }
    }

    CurrentActionHandler->CommitCosmetic(ActionContext, bActionApproved);
}

void UPX_WeaponSystemComponent::MulticastCommitAction_Implementation(const FPXWeaponActionContext& ActionContext)
{
    //PX_LOG(Log, TEXT(""));
    if ( !Character || Character->HasAuthority() || Character->IsLocallyControlled()) return;
    //if ( !CurrentActionHandler ) return;

    //PX_LOG(Log, TEXT("ActionContext : %s"), *ActionContext.ToString());

    // 클라이언트 상태 갱신
    ClientUpdateState(ActionContext);

    // BeginEquip은 ClientBuildCosmetics 완료 후 재생
    if ( ActionContext.Type == EPXWeaponActionType::BeginEquip )
    {
        const bool bIsCosmeticReady = CurrentActionHandler && CurrentAnimDriver && (CurrentWeaponSlotIndex == ActionContext.SlotIndex);

        /*
        if ( !bIsCosmeticReady )
        {
            PX_LOG(Log, TEXT("Client Cosmetics is not ready yet. Pending Action."));
        }
        else
        {
            PX_LOG(Log, TEXT("ActionContext : %s"), *ActionContext.ToString());
        }
        */

        if ( !bIsCosmeticReady )
        {
            //PX_LOG(Log, TEXT("Client Cosmetics is not ready yet. Pending Action."));
            PendingCosmeticAction = ActionContext;
            return;
        }
    }

    CurrentActionHandler->CommitCosmetic(ActionContext, ActionContext.bActionApproved);

    //PX_LOG(Log, TEXT("bShouldDryFire : %s"), ActionContext.bShouldDryFire ? TEXT("true") : TEXT("false"));
    //PX_LOG(Log, TEXT("ActionContext : %s, bShouldDryFire : %s"), *ActionContext.ToString(), ActionContext.bShouldDryFire ? TEXT("true") : TEXT("false"));

    //PX_LOG(Log, TEXT("[MC] NetMode=%d Role=%d RemoteRole=%d Owner=%s Action=%s"), (int32)NM, (int32)GetOwnerRole(), (int32)GetOwner()->GetRemoteRole(), *GetNameSafe(GetOwner()), *ActionContext.ToString());
}

/*
void UPX_WeaponSystemComponent::OnRep_WeaponUpdated()
{
    PX_LOG(Log, TEXT(""));
    if ( !Weapon || !WeaponData ) return;

    // Set Anim Layer
    if ( WeaponData->CharacterAnimClass )
    {
        if ( APX_Character* PX_Character = Cast<APX_Character>(Character) )
        {
            PX_Character->SetLayerAnimInstanceByClass(WeaponData->CharacterAnimClass);
        }
    }

    // Set Anim Driver
    if ( !CurrentAnimDriver )
    {
        CurrentAnimDriver = NewObject<UPX_WeaponAnimDriver>(this, WeaponData->AnimDriverClass);
    }
    UAnimInstance* CharacterAnim = Character ? Character->GetMesh()->GetAnimInstance() : nullptr;
    UAnimInstance* WeaponAnim = Weapon ? Weapon->GetMesh()->GetAnimInstance() : nullptr;
    CurrentAnimDriver->Initialize(CharacterAnim, WeaponAnim);

    // Set Action Handler
    if ( !CurrentActionHandler )
    {
        CurrentActionHandler = NewObject<UPX_WeaponActionHandler>(this, WeaponData->ActionHandlerClass);
    }
    CurrentActionHandler->Initialize(this, CurrentAnimDriver, WeaponData);
}

void UPX_WeaponSystemComponent::OnRep_WeaponDataUpdated()
{
    //PX_LOG(Log, TEXT(""));
    if ( !Weapon || !WeaponData ) return;
    Weapon->InitializeFromData(WeaponData);

    // Set Anim Layer
    if ( WeaponData->CharacterAnimClass )
    {
        if ( APX_Character* PX_Character = Cast<APX_Character>(Character) )
        {
            PX_Character->SetLayerAnimInstanceByClass(WeaponData->CharacterAnimClass);
        }
    }

    // Set Anim Driver
    if ( !CurrentAnimDriver )
    {
        CurrentAnimDriver = NewObject<UPX_WeaponAnimDriver>(this, WeaponData->AnimDriverClass);
    }
    UAnimInstance* CharacterAnim = Character ? Character->GetMesh()->GetAnimInstance() : nullptr;
    UAnimInstance* WeaponAnim = Weapon ? Weapon->GetMesh()->GetAnimInstance() : nullptr;
    CurrentAnimDriver->Initialize(CharacterAnim, WeaponAnim);

    // Set Action Handler
    if ( !CurrentActionHandler )
    {
        CurrentActionHandler = NewObject<UPX_WeaponActionHandler>(this, WeaponData->ActionHandlerClass);
    }
    CurrentActionHandler->Initialize(this, CurrentAnimDriver, WeaponData);    
}
*/

void UPX_WeaponSystemComponent::OnRep_WeaponSlotUpdated()
{
    //PX_LOG(Log, TEXT(""));
}

void UPX_WeaponSystemComponent::OnRep_WeaponInstanceIdUpdated()
{
    //PX_LOG(Log, TEXT(""));
}

void UPX_WeaponSystemComponent::OnRep_WeaponUpdated()
{
    //PX_LOG(Log, TEXT(""));

    ClientBuildCosmetics();

    TryResolveAndBindCurrentWeaponInstance();

    OnCurrentWeaponUpdated.Broadcast(CurrentWeaponSlotIndex);
    HandleAmmoUpdated(CurrentWeaponItemInstance->GetAmmo(), 1000);
}

void UPX_WeaponSystemComponent::ClientBuildCosmetics()
{
    //PX_LOG(Log, TEXT(""));

    if ( !Character || Character->HasAuthority() ) return;

    APX_Character* PX_Character = Cast<APX_Character>(Character);
    if ( !PX_Character ) return;

    Inventory = PX_Character->GetInventoryComponent();
    if ( !Inventory || !Inventory->IsInventoryReady() ) return;

    if ( CurrentWeaponSlotIndex == INDEX_NONE ) return;

    //PX_LOG(Log, TEXT("CurrentWeaponSlotIndex : %d"), CurrentWeaponSlotIndex);

    // 슬롯에서 현재 무기의 인스턴스와 데이터 찾기
    CurrentWeaponItemInstance = Cast<UPX_WeaponItemInstance>(Inventory->GetWeaponInstanceBySlot(CurrentWeaponSlotIndex));
    if ( !CurrentWeaponItemInstance ) return;
    UPX_WeaponDataAsset* WeaponDataAsset = CurrentWeaponItemInstance->GetItemDataAsset()->WeaponData;
    if ( !WeaponDataAsset ) return;

    if ( !CurrentWeapon ) return;

    // Create AnimDriver
    if ( WeaponDataAsset->AnimDriverClass )
    {
        if ( !CurrentAnimDriver || CurrentAnimDriver->GetClass() != WeaponDataAsset->AnimDriverClass )
        {
            CurrentAnimDriver = NewObject<UPX_WeaponAnimDriver>(this, WeaponDataAsset->AnimDriverClass);
            if ( CurrentAnimDriver )
            {
                //PX_LOG(Log, TEXT("Create Current Anim Driver Complete."));
            }
        }

        // Init AnimDriver
        if ( CurrentAnimDriver )
        {
            
            UAnimInstance* CharacterAnim = Character->GetMesh() ? Character->GetMesh()->GetAnimInstance() : nullptr;
            UAnimInstance* WeaponAnim = CurrentWeapon->GetAnimInstance();

            if ( CharacterAnim )
            {
                CurrentAnimDriver->Initialize(CharacterAnim, WeaponAnim);
            }
        }
    }
    else
    {
        CurrentAnimDriver = nullptr;
        //PX_LOG(Log, TEXT("CurrentAnimDriver is null."));
    }

    // Create ActionHandler
    if ( WeaponDataAsset->ActionHandlerClass )
    {
        if ( !CurrentActionHandler || CurrentActionHandler->GetClass() != WeaponDataAsset->ActionHandlerClass )
        {
            CurrentActionHandler = NewObject<UPX_WeaponActionHandler>(this, WeaponDataAsset->ActionHandlerClass);
            if ( CurrentActionHandler )
            {
                //PX_LOG(Log, TEXT("Create Current Action Handler Complete."));
            }
        }

        if ( CurrentActionHandler )
        {
            // Init ActionHandler
            if ( CurrentActionHandler && CurrentAnimDriver )
            {
                CurrentActionHandler->Initialize(this, CurrentAnimDriver, WeaponDataAsset);
            }
        }
    }
    else
    {
        CurrentActionHandler = nullptr;
        //PX_LOG(Log, TEXT("CurrentActionHandler is null."));
    }

    // Set Layer Anim
    if ( WeaponDataAsset->CharacterAnimClass )
    {
        PX_Character->SetLayerAnimInstanceByClass(WeaponDataAsset->CharacterAnimClass);
    }

    //PX_LOG(Log, TEXT("Client Build Weapon's Cosmetics Complete."));

    //PX_LOG(Log, TEXT("Cosmetics Built. Slot=%d WeaponData=%s"), CurrentWeaponSlotIndex, WeaponDataAsset ? *WeaponDataAsset->GetPathName() : TEXT("None"));

    if ( PendingCosmeticAction.IsSet() )
    {
        const FPXWeaponActionContext& ActionContext = PendingCosmeticAction.GetValue();

        //PX_LOG(Log, TEXT("Play Pending Action : %s"), *ActionContext.ToString());

        // 슬롯이 일치하고 준비되었을 때만
        if ( ActionContext.Type == EPXWeaponActionType::BeginEquip && CurrentWeaponSlotIndex == ActionContext.SlotIndex && CurrentActionHandler && CurrentAnimDriver )
        {
            CurrentActionHandler->CommitCosmetic(ActionContext, /*bIsLocal=*/true);
            PendingCosmeticAction.Reset();
        }
    }
}

void UPX_WeaponSystemComponent::TryResolveAndBindCurrentWeaponInstance()
{
    //PX_LOG(Log, TEXT(""));
    if ( !Character )
    {
        Character = Cast<ACharacter>(GetOwner());
        if ( !Character ) return;
    }
    //PX_LOG(Log, TEXT("Character is not null"));

    if ( !Inventory )
    {
        APX_Character* PXCharacter = Cast<APX_Character>(Character);
        if ( !PXCharacter ) return;

        Inventory = PXCharacter->GetInventoryComponent();
    }
    //PX_LOG(Log, TEXT("Inventory is not null"));

    if ( !Inventory || !Inventory->IsInventoryReady() ) return;
    if ( CurrentWeaponSlotIndex == INDEX_NONE ) return;
    //PX_LOG(Log, TEXT("CurrentWeaponSlotIndex : %d"), CurrentWeaponSlotIndex);
    if ( !CurrentWeaponInstanceId.IsValid() ) return;
    //PX_LOG(Log, TEXT("CurrentWeaponInstanceId : %s"), *CurrentWeaponInstanceId.ToString());


    // 슬롯에서 인스턴스 찾기
    UPX_WeaponItemInstance* NewWeaponItemInstance = Cast<UPX_WeaponItemInstance>(Inventory->GetWeaponInstanceBySlot(CurrentWeaponSlotIndex));

    if ( !NewWeaponItemInstance ) return;
    //PX_LOG(Log, TEXT("NewWeaponItemInstance is not null"));
    if ( NewWeaponItemInstance->GetInstanceId() != CurrentWeaponInstanceId ) return;
    //PX_LOG(Log, TEXT("InstanceId is same"));

    if ( AmmoUpdatedHandle.IsValid() && NewWeaponItemInstance->OnAmmoUpdated.IsBoundToObject(this) )
    {
        return;
    }

    // 이전 인스턴스 언바인딩
    UnbindFromCurrentWeaponInstance();

    // 로컬 캐시 갱신
    CurrentWeaponItemInstance = NewWeaponItemInstance;

    // 델리게이트 바인딩
    AmmoUpdatedHandle = CurrentWeaponItemInstance->OnAmmoUpdated.AddUObject(this, &UPX_WeaponSystemComponent::HandleAmmoUpdated);
    AttackModeUpdatedHandle = CurrentWeaponItemInstance->OnAttackModeUpdated.AddUObject(this, &UPX_WeaponSystemComponent::HandleAttackModeUpdated);

    //HandleAmmoUpdated(CurrentWeaponItemInstance->GetItemDataAsset()->WeaponData->MagSize, CurrentWeaponItemInstance->GetAmmo());
    //PX_LOG(Log, TEXT(""));
}

void UPX_WeaponSystemComponent::UnbindFromCurrentWeaponInstance()
{
    //PX_LOG(Log, TEXT(""));
    if ( CurrentWeaponItemInstance && AmmoUpdatedHandle.IsValid() )
    {
        CurrentWeaponItemInstance->OnAmmoUpdated.Remove(AmmoUpdatedHandle);
        CurrentWeaponItemInstance->OnAttackModeUpdated.Remove(AttackModeUpdatedHandle);
    }

    AmmoUpdatedHandle.Reset();
    AttackModeUpdatedHandle.Reset();

    CurrentWeaponItemInstance = nullptr;
}