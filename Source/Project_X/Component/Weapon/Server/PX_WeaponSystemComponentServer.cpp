// Fill out your copyright notice in the Description page of Project Settings.

#include "../PX_WeaponSystemComponent.h"

#include "../PX_WeaponDataAsset.h"
#include "Entity/PX_Weapon.h"
#include "GameFramework/Character.h"
#include "Component/Inventory/PX_WeaponItemInstance.h"
#include "Component/Inventory/PX_ItemDataAsset.h"
#include "Component/Inventory/PX_InventoryComponent.h"
#include "Entity/PX_Character.h"

/*
void UPX_WeaponSystemComponent::Server_TickComponent(float DeltaSeconds, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{

}
*/

/*
bool UPX_WeaponSystemComponent::ServerEquipWeapon_Validate(APX_Weapon* InWeapon)
{
    return true;
}

void UPX_WeaponSystemComponent::ServerEquipWeapon_Implementation(APX_Weapon* InWeapon)
{
    //PX_LOG(Log, TEXT(""));
    if ( !Character || !Character->HasAuthority() || Character->IsLocallyControlled() ) return;
    if ( !InWeapon || !InWeapon->GetWeaponData() ) return;

    // Set Replicate Variables
    Weapon = InWeapon;
    WeaponData = InWeapon->GetWeaponData();

    // Attach to character
    if ( USkeletalMeshComponent* CharMesh = Character->GetMesh() )
    {
        bool rst = InWeapon->AttachToComponent(CharMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, WeaponData->AttachSocketName);
    }

    //WeaponSystemComponent->ClientSubmitAction(FPXWeaponActionContext::Builder(EPXWeaponActionType::BeginEquip, Value.Get<float>() - 1));
}
*/

bool UPX_WeaponSystemComponent::ServerEquipBySlot_Validate(int32 SlotIndex)
{
    return true;
}

void UPX_WeaponSystemComponent::ServerEquipBySlot_Implementation(int32 SlotIndex)
{
    if ( !Character || !Character->HasAuthority() || Character->IsLocallyControlled() ) return;

    APX_Character* PX_Character = Cast<APX_Character>(Character);
    if ( !PX_Character ) return;

    Inventory = PX_Character->GetInventoryComponent();
    if ( !Inventory || !Inventory->IsInventoryReady() ) return;

    //if ( !CurrentWeapon || !CurrentWeaponItemInstance ) return;
    //if ( SpawnedWeapons.IsEmpty() ) return;
    if ( SlotIndex == INDEX_NONE ) return;

    UPX_WeaponItemInstance* NewWeaponInstance = Cast<UPX_WeaponItemInstance>(Inventory->GetWeaponInstanceBySlot(SlotIndex));
    
    if ( !NewWeaponInstance ) return;

    // 같은 슬롯 재장착 방지
    if ( CurrentWeaponSlotIndex == SlotIndex && CurrentWeaponItemInstance == NewWeaponInstance ) return;

    // 기존 무기 숨김 처리
    if ( CurrentWeapon && CurrentWeaponItemInstance && CurrentWeaponItemInstance != NewWeaponInstance )
    {
        // 캐릭터로부터 분리
        CurrentWeapon->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
        // 숨김 처리
        CurrentWeapon->SetActorHiddenInGame(true);
        // 델리게이트 해제
    }

    APX_Weapon* NewWeapon = nullptr;
    // 이미 스폰된 PX_Weapon이면 SpawnedWeapons에서 찾음
    if ( TObjectPtr<APX_Weapon>* Found = SpawnedWeapons.Find(SlotIndex) )
    {
        NewWeapon = Found->Get();
        //PX_LOG(Log, TEXT("Weapon is already Spawned. Using spawned weapon."));
    }

    // SpawnedWeapons에 없으면 새로 스폰하고 SpawnedWeapons에 추가
    if ( !NewWeapon )
    {
        UPX_WeaponDataAsset* WeaponDataAsset = NewWeaponInstance->GetItemDataAsset()->WeaponData;
        if ( !WeaponDataAsset ) return;

        UWorld* World = GetWorld();
        if ( !World ) return;

        // 나중에 PX_Weapon을 확장하면 바꿔야 함
        //TSubclassOf<APX_Weapon> WeaponClass = WeaponDataAsset->WeaponClass;
        TSubclassOf<APX_Weapon> WeaponClass = APX_Weapon::StaticClass();
        if ( !WeaponClass ) return;

        // 새로운 무기 스폰
        FActorSpawnParameters Params;
        Params.Owner = Character;
        Params.Instigator = Character->GetInstigator();
        Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

        NewWeapon = World->SpawnActor<APX_Weapon>(WeaponClass, FTransform::Identity, Params);
        //PX_LOG(Log, TEXT("New Weapon Spawned"));
        if ( !NewWeapon ) return;
        NewWeapon->InitializeFromData(WeaponDataAsset);

        SpawnedWeapons.Add(SlotIndex, NewWeapon);
    }

    // 무기 관련 변수 갱신
    // Replicated
    CurrentWeaponSlotIndex = SlotIndex;
    CurrentWeaponInstanceId = NewWeaponInstance->GetInstanceId();
    CurrentWeapon = NewWeapon;
    // Non-Replicated
    CurrentWeaponItemInstance = NewWeaponInstance;

    // 무기 부착
    if ( USkeletalMeshComponent* CharacterMesh = Character->GetMesh() )
    {
        const UPX_WeaponDataAsset* WeaponDataAsset = NewWeaponInstance->GetItemDataAsset()->WeaponData;
        const FName SocketName = WeaponDataAsset ? WeaponDataAsset->AttachSocketName : NAME_None;

        CurrentWeapon->AttachToComponent(CharacterMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, SocketName);
    }

    CurrentWeapon->SetActorHiddenInGame(false);

    //PX_LOG(Log, TEXT("Equip Weapon[%d] Complete"), SlotIndex);

    /*
    APX_Weapon* NewWeapon = Weapons[SlotIndex];
    if ( !NewWeapon ) return;
    if ( Weapon == NewWeapon ) return;

    if ( Weapon && Weapon != NewWeapon )
    {
        // 캐릭터로부터 분리
        Weapon->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

        // (선택) 숨김 처리
        Weapon->SetActorHiddenInGame(true);
    }

    // Set Replicate Variables
    Weapon = NewWeapon;
    Weapon->SetActorHiddenInGame(false);

    // Attach to character
    if ( USkeletalMeshComponent* CharMesh = Character->GetMesh() )
    {
        bool rst = Weapon->AttachToComponent(CharMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, WeaponItemInstance->GetWeaponData()->AttachSocketName);
    }
    */

    //ServerSubmitAction();
}

/*
bool UPX_WeaponSystemComponent::ServerEquipWeaponWithData_Validate(UPX_WeaponDataAsset* NewWeaponData)
{
    return true;
}

void UPX_WeaponSystemComponent::ServerEquipWeaponWithData_Implementation(UPX_WeaponDataAsset* NewWeaponData)
{
    ServerDestroyWeapon(Weapon);

    //PX_LOG(Log, TEXT(""));
    if ( !Character || !Character->HasAuthority() || Character->IsLocallyControlled() ) return;
    if ( !NewWeaponData ) return;

    // Spawn Weapon
    TSubclassOf<APX_Weapon> SpawnClass = APX_Weapon::StaticClass();
    if ( !SpawnClass ) return;

    FActorSpawnParameters Params;
    Params.Owner = Character;
    Params.Instigator = Character;
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    APX_Weapon* NewWeapon = Character->GetWorld()->SpawnActor<APX_Weapon>(SpawnClass, Params);
    if ( !NewWeapon ) return;

    NewWeapon->InitializeFromData(NewWeaponData);

    ServerEquipWeapon(NewWeapon);
}
*/

bool UPX_WeaponSystemComponent::ServerDestroyWeapon_Validate(APX_Weapon* InWeapon)
{
    return true;
}

void UPX_WeaponSystemComponent::ServerDestroyWeapon_Implementation(APX_Weapon* InWeapon)
{
    //PX_LOG(Log, TEXT(""));

    if ( InWeapon )
    {
        InWeapon->Destroy();
        InWeapon = nullptr;
    }
}

bool UPX_WeaponSystemComponent::ServerSubmitAction_Validate(const FPXWeaponActionContext& ActionContext)
{
    return true;
}

void UPX_WeaponSystemComponent::ServerSubmitAction_Implementation(const FPXWeaponActionContext& ActionContext)
{
    if ( !Character || !Character->HasAuthority() || Character->IsLocallyControlled() ) return;
    if ( ActionContext.Type == EPXWeaponActionType::None ) return;

    /*
    // ActionContext로 Key 생성
    const UObject* Key = GetKeyFromContext(ActionContext);
    if ( !Key ) return;

    // Key에 맞는 Buffer를 가져옴
    FSubmittedBuffer& Buffer = PendingActions.FindOrAdd(Key);
    */

    // ActionContext에서 ClientSequence를 가져옴
    const uint32 CurrentSequence = ActionContext.ClientSequence;
    
    // ServerSequence 부여
    FPXWeaponActionContext ServerActionContext = ActionContext;
    ServerActionContext.ServerSequence = ServerSequence++;

    // 너무 오래되었거나 이미 처리 완료된 Action은 폐기
    if ( CurrentSequence < Buffer.ExpectedSequence ) return;
    // 중복 Action은 무시
    if ( Buffer.SubmittedBuffer.Contains(CurrentSequence) ) return;
    // 새로운 Action을 저장
    Buffer.SubmittedBuffer.Add(CurrentSequence, ServerActionContext);
    

    /*
    // (선택) 버퍼 크기 제한
    constexpr int32 MaxPendingPerKey = 256;
    if ( Buffer.SubmittedBuffer.Num() > MaxPendingPerKey )
    {
        PX_LOG(PX_WeaponSystemComponent, Warning, TEXT("ServerSubmitAction: Pending overflow. Key = %s Num = %d Expected = %u"), *GetNameSafe(Key), Buffer.SubmittedBuffer.Num(), Buffer.ExpectedSequence);

        // 정책 1) 가장 오래된 것들 정리
        // 정책 2) 입력 거부
    }
    */

    PX_LOG(Log, TEXT("Submit Action. SubmittedBuffer Size = %d. ExpectedSequence = %u, ActionContext : %s"), Buffer.SubmittedBuffer.Num(), Buffer.ExpectedSequence, *ServerActionContext.ToString());
}

void UPX_WeaponSystemComponent::ServerCommitAction(float FixedDeltaSeconds)
{
    if ( !Character || !Character->HasAuthority() || Character->IsLocallyControlled() ) return;

    /*
    for ( auto& Pair : PendingActions )
    {  
    }
    */

    // 이번 Step에서 처리한 Action 개수
    uint32 ActionProcessed = 0;
    while ( FPXWeaponActionContext* Found = Buffer.SubmittedBuffer.Find(Buffer.ExpectedSequence) )
    {
        // ActionContext 복사 후 컨테이너에서 제거
        FPXWeaponActionContext ActionContext = *Found;
        Buffer.SubmittedBuffer.Remove(Buffer.ExpectedSequence++);

        //PX_LOG(Log, TEXT("Commit Action. ActionContext : %s"), *ActionContext.ToString());


        // 액션 승인 판단
        // TODO:
        // 예: CurrentActionHandler->Commit(Context) 또는 switch(Context.Type) 처리
        // CommitAction(Context);
        //ActionContext.bActionApproved = true;
        bool bCommitted = true, bStateUpdated = false;
        switch ( ActionContext.Type )
        {
        case EPXWeaponActionType::BeginEquip:
            UPX_WeaponItemInstance* TargetInstance = Cast<UPX_WeaponItemInstance>(Inventory->GetWeaponInstanceBySlot(ActionContext.SlotIndex));

            // 장착하고자 하는 무기가 없으면 거절
            if ( !TargetInstance )
            {
                bCommitted = false;
                break;
            }

            // 현재 무기가 없으면 항상 승인
            if ( !CurrentWeaponItemInstance )
            {
                bCommitted = true;
                break;
            }

            // 현재 무기와 장착하려는 무기가 서로 달라야 승인
            const FGuid& CurrentGuid = CurrentWeaponItemInstance->GetInstanceId();
            const FGuid& TargetGuid = TargetInstance->GetInstanceId();
            bCommitted = (CurrentGuid != TargetGuid);
            break;
        }

        if ( bCommitted )
        {
            if ( ActionContext.Type == EPXWeaponActionType::BeginEquip )
            {
                ServerEquipBySlot(ActionContext.SlotIndex);
            }

            // 서버 상태 갱신
            bStateUpdated = ServerUpdateState(ActionContext);
        }

        // 승인 결과를 컨텍스트에 반영
        ActionContext.bActionApproved = bCommitted && bStateUpdated;

        PX_LOG(Log, TEXT("Commit Action. Approved : %s,  ActionContext : %s"), ActionContext.bActionApproved ? TEXT("true") : TEXT("false"), * ActionContext.ToString());
        
        if ( ActionContext.bActionApproved )
        {
            ClientCommitAction(ActionContext, ActionContext.bActionApproved);
            MulticastCommitAction(ActionContext);
        }

        if ( ++ActionProcessed >= MaxActionsPerStep ) break;
    }

    // FirePressed이면 ActionProcessed와 관계없이 이번 Step에 반드시 AttackOnce 액션을 생성하고 수행함
    if ( bAttackPressed_Server )
    {
        //PX_LOG(Log, TEXT("Commit Action. bAttackPressed_Server = %s"), bAttackPressed_Server ? TEXT("true") : TEXT("false"));
        ServerTryAttackOnce();
    }
}

bool UPX_WeaponSystemComponent::ServerUpdateState(const FPXWeaponActionContext& ActionContext)
{
    //PX_LOG(Log, TEXT(""));
    if ( !Character || !Character->HasAuthority() ) return false;
    //PX_LOG(Log, TEXT(""));
    if ( !CurrentWeapon || !CurrentWeaponItemInstance->GetItemDataAsset()->WeaponData ) return false;
    //if ( !ActionContext ) return;

    // 현재 무기의 공격 모드
    //EPXWeaponAttackMode AttackMode = Weapon ? (Weapon->GetFireMode()) : (WeaponData ? WeaponData->DefaultAttackMode : EPXWeaponAttackMode::Single);
    EPXWeaponAttackMode AttackMode = CurrentWeaponItemInstance->GetAttackMode();
    //PX_LOG(Log, TEXT("Attack Mode : %s"), WeaponAttackModeToString(AttackMode));
    //PX_LOG(Log, TEXT("bIsEquipping_Server : %s"), bIsEquipping_Server ? TEXT("true") : TEXT("false"));
    switch ( ActionContext.Type )
    {
    case EPXWeaponActionType::BeginEquip:
    {
        //if ( bIsEquipping_Server ) return false;

        bAttackPressed_Server = false;
        BurstRemaining_Server = 0;
        bIsReloading_Server = false;
        bIsEquipping_Server = true;
        //PX_LOG(Log, TEXT("BeginEquip"));
        break;
    }
    case EPXWeaponActionType::EndEquip:
    {
        if ( !bIsEquipping_Server ) return false;

        bAttackPressed_Server = false;
        BurstRemaining_Server = 0;
        bIsReloading_Server = false;
        bIsEquipping_Server = false;
        //PX_LOG(Log, TEXT("EndEquip"));
        break;
    }
    case EPXWeaponActionType::AttackPressed:
    {
        //PX_LOG(Log, TEXT("bAttackPressed_Server %s -> %s"), bAttackPressed_Server ? TEXT("true") : TEXT("false"), !bAttackPressed_Server ? TEXT("true") : TEXT("false"));
        if ( AttackMode != EPXWeaponAttackMode::Burst || BurstRemaining_Server == 0 )
        {
            bAttackPressed_Server = true;
        }

        // Set BurstRemaining if Attack Mode is Burst.
        if ( AttackMode == EPXWeaponAttackMode::Burst && BurstRemaining_Server == 0 )
        {
            // 나중에 BurstCount와 인벤토리 내 남은 탄 개수를 비교해야 함
            BurstRemaining_Server = FMath::Max(1, CurrentWeaponItemInstance->GetItemDataAsset()->WeaponData->BurstCount);
        }
        break;
    }

    case EPXWeaponActionType::AttackReleased:
    {
        // Burst는 BurestRemaining이 0이 될때까지 AttackReleased 하지 않음
        if ( AttackMode != EPXWeaponAttackMode::Burst )
        {
            //PX_LOG(Log, TEXT("bAttackPressed_Server %s -> %s"), bAttackPressed_Server ? TEXT("true") : TEXT("false"), !bAttackPressed_Server ? TEXT("true") : TEXT("false"));
            bAttackPressed_Server = false;
        }
        break;
    }

    case EPXWeaponActionType::BeginReload:
    {
        if ( bIsEquipping_Server || bIsReloading_Server ) return false;
        if ( CurrentWeaponItemInstance->GetAmmo() >= CurrentWeaponItemInstance->GetItemDataAsset()->WeaponData->MagSize ) return false;

        UWorld* World = GetWorld();
        if ( !World ) return false;

        // 점사 중일 때 재장전을 누르면 발사를 취소하고 바로 재장전
        //PX_LOG(Log, TEXT("bAttackPressed_Server %s -> %s, bIsReloading_Server %s -> %s"), bAttackPressed_Server ? TEXT("true") : TEXT("false"), !bAttackPressed_Server ? TEXT("true") : TEXT("false"), bIsReloading_Server ? TEXT("true") : TEXT("false"), !bIsReloading_Server ? TEXT("true") : TEXT("false"));
        bAttackPressed_Server = false;
        bIsReloading_Server = true;
        BlockingActionTime_Server = World->GetTimeSeconds();
        BurstRemaining_Server = 0;
        break;
    }

    case EPXWeaponActionType::EndReload:
    {
        if ( !bIsReloading_Server ) return false;
        bIsReloading_Server = false;
        // 나중에 MagSize와 인벤토리에 남은 탄 수를 비교해야 함
        CurrentWeaponItemInstance->SetAmmo(CurrentWeaponItemInstance->GetItemDataAsset()->WeaponData->MagSize);
        break;
    }

    case EPXWeaponActionType::SetFireMode:
    {
        if ( bIsEquipping_Server || bIsReloading_Server ) return false;

        bAttackPressed_Server = false;
        CurrentWeaponItemInstance->SwitchFireMode();
        BurstRemaining_Server = 0;
        //PX_LOG(Log, TEXT("Client End Reload. bIsReloading_Local %s -> %s"), bIsReloading_Local ? TEXT("true") : TEXT("false"), !bIsReloading_Local ? TEXT("true") : TEXT("false"));
        break;
    }

    default:
        break;
    }

    return true;
}

void UPX_WeaponSystemComponent::ServerTryAttackOnce()
{
    if ( !Character || !Character->HasAuthority() ) return;

    UPX_WeaponItemInstance* WeaponItemInstance = CurrentWeaponItemInstance.Get();
    UPX_WeaponDataAsset* WeaponData = WeaponItemInstance->GetItemDataAsset()->WeaponData.Get();

    if ( !CurrentWeapon || !WeaponItemInstance || !WeaponData ) return;

    UWorld* World = GetWorld();
    if ( !World ) return;
    const float CurrentTime = World->GetTimeSeconds();

    // Check Attack Cool Time.
    if ( CurrentTime < NextAttackTime_Server ) return;

    // 이번 Step에 AttackOnce 액션을 생성할지 결정
    bool bShouldAttackOnce = false;
    EPXWeaponAttackMode AttackMode = CurrentWeaponItemInstance->GetAttackMode();
    switch ( AttackMode )
    {
    case EPXWeaponAttackMode::Single:
    case EPXWeaponAttackMode::Auto:
        bShouldAttackOnce = bAttackPressed_Server;
        break;

    case EPXWeaponAttackMode::Burst:
        bShouldAttackOnce = (BurstRemaining_Server > 0);
        break;

    default:
        break;
    }
    if ( !bShouldAttackOnce ) return;

    const bool bUsesAmmo = (WeaponData->ProjectileClass != nullptr) && (WeaponData->MagSize > 0);
    const int32 AmmoBefore = WeaponItemInstance->GetAmmo();

    // 서버가 AttackOnce 컨텍스트를 생성
    FPXWeaponActionContext AttackOnceContext;
    //AttackOnceContext.InstigatorDebugName = ActionContext->InstigatorDebugName;
    //AttackOnceContext.InstigatorId = ActionContext->InstigatorId;
    AttackOnceContext.ClientSequence = INDEX_NONE;
    AttackOnceContext.ServerSequence = ServerSequence++;
    AttackOnceContext.Type = EPXWeaponActionType::AttackOnce;
    //AttackOnceContext.SlotIndex = ActionContext->SlotIndex;
    AttackOnceContext.bShouldDryFire = (bUsesAmmo && AmmoBefore <= 0);
    //AttackOnceContext.bShouldDryFire = false;
    AttackOnceContext.bActionApproved = ServerAttackOnceApproved(&AttackOnceContext.ActionDenyReason);

    //PX_LOG(Log, TEXT("Try Attack Once. AttackMode : %s, bShouldDryFire = %s, bActionApproved = %s"), WeaponAttackModeToString(AttackMode), AttackOnceContext.bShouldDryFire ? TEXT("true") : TEXT("false"), AttackOnceContext.bActionApproved ? TEXT("true") : TEXT("false"));


    if ( !AttackOnceContext.bActionApproved ) return;

    ClientCommitAction(AttackOnceContext, AttackOnceContext.bActionApproved);
    
    

    if ( !AttackOnceContext.bShouldDryFire )
    {
        WeaponItemInstance->ConsumeAmmo(1);
        // TODO : 탄 소모
        // Spawn Projectile
    }
    else
    {
        // Handle DryFire
        bAttackPressed_Server = false;

        if ( AttackMode == EPXWeaponAttackMode::Burst )
        {
            BurstRemaining_Server = 0;
        }
    }

    MulticastCommitAction(AttackOnceContext);

    // Update Next Attack Time
    NextAttackTime_Server = CurrentTime + FMath::Max(0.f, WeaponData->FireInterval);

    // Handle Burst Mode
    if ( AttackMode == EPXWeaponAttackMode::Burst )
    {
        //PX_LOG(Log, TEXT("Try Attack Once. AttackMode : Burst. BurstRemaining_Server = %d"), BurstRemaining_Server);
        BurstRemaining_Server = FMath::Max(0, BurstRemaining_Server - 1);
        // End Burst & Update Server State
        if ( BurstRemaining_Server == 0 )
        {
            bAttackPressed_Server = false;
            //PX_LOG(Log, TEXT("Try Attack Once. AttackMode : Burst. Burst End. bAttackPressed_Server is false"));
        }
    }
    // Handle Single Modes
    else if ( AttackMode == EPXWeaponAttackMode::Single )
    {
        // 단발은 1발 발사 후 즉시 입력 해제
        bAttackPressed_Server = false;
        //PX_LOG(Log, TEXT("Try Attack Once. AttackMode : Single. bAttackPressed_Server is false"));
    }

        
    if ( AttackMode == EPXWeaponAttackMode::Burst )
    {
        PX_LOG(Log, TEXT("Try Attack Once. Ammo : %d / %d. BurstRemaining_Server = %d, bShouldDryFire = %s, ActionContext : %s"), WeaponItemInstance->GetAmmo(), WeaponData->MagSize, BurstRemaining_Server, AttackOnceContext.bShouldDryFire ? TEXT("true") : TEXT("false"), *AttackOnceContext.ToString());
    }
    else
    {
        PX_LOG(Log, TEXT("Try Attack Once. Ammo : %d / %d, bShouldDryFire = %s, ActionContext : %s"), WeaponItemInstance->GetAmmo(), WeaponData->MagSize, AttackOnceContext.bShouldDryFire ? TEXT("true") : TEXT("false"), *AttackOnceContext.ToString());
    }

    /*
    if ( bAttackPressed_Server && WeaponItemInstance->GetAmmo() <= 0 )
    {
        bAttackPressed_Server = false;
        if ( AttackMode == EPXWeaponAttackMode::Burst )
        {
            BurstRemaining_Server = 0;
        }
    }
    */
}

bool UPX_WeaponSystemComponent::ServerAttackOnceApproved(FString* OutReason) const
{
    if ( !Character || !Character->HasAuthority() ) { if ( OutReason ) *OutReason = TEXT("No Authority"); return false; }
    //if ( bActionLocked ) { if ( OutReason ) *OutReason = TEXT("Action Locked"); return false; }
    if ( bIsReloading_Server ) { if ( OutReason ) *OutReason = TEXT("Reloading"); return false; }
    if ( bIsEquipping_Server ) { if ( OutReason ) *OutReason = TEXT("Equipping"); return false; }

    // 필요하면 여기 추가:
    // - Weapon null
    // - 죽음/기절 상태
    // - 탄 변경 중
    return true;
}