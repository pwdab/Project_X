// Fill out your copyright notice in the Description page of Project Settings.

#include "../PX_WeaponSystemComponent.h"

#include "../PX_WeaponDataAsset.h"
#include "Entity/PX_Weapon.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/Controller.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "DrawDebugHelpers.h"
#include "Component/Inventory/PX_WeaponItemInstance.h"
#include "Component/Inventory/PX_ItemDataAsset.h"
#include "Component/Inventory/PX_InventoryComponent.h"
#include "Entity/PX_Character.h"
#include "Entity/PX_Projectile.h"

#include "AbilitySystem/Tags/PX_GameplayTags.h"
#include "AbilitySystem/Abilities/PX_GameplayAbility_Equip.h"

/*
void UPX_WeaponSystemComponent::Server_TickComponent(float DeltaSeconds, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{

}
*/

void UPX_WeaponSystemComponent::ServerEquipBySlot_Implementation(int32 SlotIndex)
{
    Authoritative_EquipBySlot(SlotIndex);
}

bool UPX_WeaponSystemComponent::Authoritative_EquipBySlot(int32 SlotIndex)
{
    PX_LOG(Log, TEXT("SlotIndex: %d"), SlotIndex);
    if ( !Character || !Character->HasAuthority() )
    {
        PX_LOG(Warning, TEXT("Invalid Character State. Character: %s, HasAuthority: %d, IsLocallyControlled: %d"), *GetNameSafe(Character), Character ? Character->HasAuthority() : -1, Character ? Character->IsLocallyControlled() : -1);
		return false;
    }

    APX_Character* PX_Character = Cast<APX_Character>(Character);
    if ( !PX_Character )
    {
        PX_LOG(Warning, TEXT("Invalid PX_Character"));
        return false;
    }

    UAbilitySystemComponent* ASC = PX_Character->GetAbilitySystemComponent();
    if ( !ASC )
    {
        PX_LOG(Warning, TEXT("Invalid ASC. Continue equip without ability tag updates."));
    }

    if ( !Inventory || !Inventory->IsInventoryReady() )
    {
        PX_LOG(Warning, TEXT("Inventory is not ready. Inventory: %s, IsInventoryReady: %d"), *GetNameSafe(Inventory), Inventory ? Inventory->IsInventoryReady() : -1);
		return false;
    }

    if ( SlotIndex == INDEX_NONE )
    {
		PX_LOG(Warning, TEXT("Invalid SlotIndex: %d"), SlotIndex);
		return false;
    }

    // Setup Server Equipping Weapon Info
    int32 ServerEquippingWeaponSlotIndex = SlotIndex;
    TObjectPtr<UPX_WeaponItemInstance> ServerEquippingWeaponItemInstance = Cast<UPX_WeaponItemInstance>(Inventory->GetWeaponItemInstanceBySlot(ServerEquippingWeaponSlotIndex));
    if ( !ServerEquippingWeaponItemInstance )
    {
        PX_LOG(Warning, TEXT("No ServerEquippingWeaponItemInstance found in SlotIndex: %d"), SlotIndex);
        return false;
    }

    FGuid ServerEquippingWeaponItemInstanceId = ServerEquippingWeaponItemInstance->GetInstanceId();
    if ( !ServerEquippingWeaponItemInstanceId.IsValid() )
    {
        PX_LOG(Warning, TEXT("Invalid ServerEquippingWeaponItemInstanceId "));
        return false;
    }

    // 같은 무기 재장착 방지
    if ( ServerEquippingWeaponItemInstanceId == CurrentWeaponItemInstanceId )
    {
        PX_LOG(Warning, TEXT("Attempting to equip the same weapon instance. ServerEquippingWeaponItemInstanceId: %s, CurrentWeaponInstanceId: %s"), *ServerEquippingWeaponItemInstanceId.ToString(), *CurrentWeaponItemInstanceId.ToString());
        return false;
    }

    UPX_WeaponDataAsset* ServerEquippingWeaponDataAsset = ServerEquippingWeaponItemInstance->GetWeaponDataAsset();
    if ( !ServerEquippingWeaponDataAsset )
    {
		PX_LOG(Warning, TEXT("No WeaponDataAsset found for ServerEquippingWeaponItemInstance in SlotIndex: %d"), SlotIndex);
		return false;
    }

    // Equip 몽타주 데이터 가져오기
    UAnimMontage* EquipMontage = ServerEquippingWeaponDataAsset->EquipAction.CharacterMontage;
    if ( !EquipMontage )
    {
		PX_LOG(Warning, TEXT("No EquipMontage found in ServerEquippingWeaponDataAsset for SlotIndex: %d"), SlotIndex);
		return false;
    }

	UWorld* World = GetWorld();
    if ( !World )
    {
		PX_LOG(Warning, TEXT("Invalid World"));
		return false;
    }

    // 기존 무기 숨김 처리
    if ( CurrentWeapon )
    {
        PX_LOG(Log, TEXT("Hide Current Weapon: %s"), *CurrentWeapon->GetName());
        CurrentWeapon->SetActorHiddenInGame(true);
    }

    // 기존 무기의 Ability를 제거
    if ( CurrentWeaponItemInstance && ASC )
    {
        CurrentWeaponItemInstance->RemoveTags(ASC);
        CurrentWeaponItemInstance->RemoveAbilities(ASC);
    }

    TObjectPtr<APX_Weapon> ServerEquippingWeapon = nullptr;
    // 이미 스폰된 PX_Weapon이면 SpawnedWeapons에서 찾음
    if ( TObjectPtr<APX_Weapon>* Found = ServerSpawnedWeapons.Find(ServerEquippingWeaponItemInstanceId) )
    {
        ServerEquippingWeapon = Found->Get();
        PX_LOG(Log, TEXT("Found Server Equipping Weapon: %s in SpawnedWeapons for SlotIndex: %d"), *ServerEquippingWeapon->GetName(), SlotIndex);
    }

    // SpawnedWeapons에 없으면 새로 스폰하고 SpawnedWeapons에 추가
    //if ( !ServerEquippingWeapon )
    else
    {
        //UPX_WeaponDataAsset* WeaponDataAsset = ServerEquippingWeaponDataAsset;
        if ( !ServerEquippingWeaponDataAsset )
        {
			PX_LOG(Warning, TEXT("Invalid ServerEquippingWeaponDataAsset for the new weapon instance in SlotIndex: %d"), SlotIndex);
			return false;
        }

        // 나중에 PX_Weapon을 확장하면 바꿔야 함
        //TSubclassOf<APX_Weapon> WeaponClass = WeaponDataAsset->WeaponClass;
        TSubclassOf<APX_Weapon> WeaponClass = APX_Weapon::StaticClass();
        if ( !WeaponClass )
        {
			PX_LOG(Warning, TEXT("Invalid WeaponClass in WeaponDataAsset for SlotIndex: %d"), SlotIndex);
			return false;
        }

        // 새로운 무기 스폰
        FActorSpawnParameters Params;
        Params.Owner = Character;
        Params.Instigator = Character->GetInstigator();
        Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        /*
        FString WeaponName = TEXT("Server_") + ServerEquippingWeaponDataAsset->ItemName.ToString();
        ULevel* LevelOuter = World ? World->PersistentLevel : nullptr;
        FName BaseName(*WeaponName);
        Params.Name = MakeUniqueObjectName(LevelOuter, APX_Weapon::StaticClass(), BaseName);
        */

        ServerEquippingWeapon = World->SpawnActor<APX_Weapon>(WeaponClass, FTransform::Identity, Params);
        PX_LOG(Log, TEXT("New Weapon Spawned: %s"), *ServerEquippingWeapon->GetName());
        //PX_LOG(Log, TEXT("New Weapon Spawned"));
        if ( !ServerEquippingWeapon )
        {
			PX_LOG(Warning, TEXT("Failed to spawn new weapon for SlotIndex: %d"), SlotIndex);
			return false;
        }
        ServerEquippingWeapon->InitializeFromData(ServerEquippingWeaponDataAsset);
        ServerSpawnedWeapons.Add(ServerEquippingWeaponItemInstanceId, ServerEquippingWeapon);
    }

    // 무기 부착
    if ( USkeletalMeshComponent* CharacterMesh = Character->GetMesh() )
    {
        const FName SocketName = ServerEquippingWeaponDataAsset ? ServerEquippingWeaponDataAsset->AttachSocketName : NAME_None;

        // 서버의 무기는 Commit 할 때 보이도록 처리
        //ServerEquippingWeapon->SetActorHiddenInGame(false);
        ServerEquippingWeapon->AttachToComponent(CharacterMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, SocketName);
    }

    // Setup Server Current Weapon Info
    CurrentWeaponSlotIndex = ServerEquippingWeaponSlotIndex;
    CurrentWeaponItemInstance = ServerEquippingWeaponItemInstance;
    CurrentWeaponItemInstanceId = ServerEquippingWeaponItemInstanceId;
    CurrentWeapon = ServerEquippingWeapon;
    PX_Character->SetHasEquippedWeapon(CurrentWeaponSlotIndex != 4);

    // 현재 무기의 Ability를 부여
    if ( CurrentWeaponItemInstance && ASC )
    {
        CurrentWeaponItemInstance->AddTags(ASC);
        CurrentWeaponItemInstance->GiveAbilities(ASC);
    }

    /*
    // Setup Server Equipping Weapon Info
    ServerEquippingWeaponSlotIndex = INDEX_NONE;
    ServerEquippingWeaponItemInstance = nullptr;
    ServerEquippingWeaponItemInstanceId = FGuid();
    ServerEquippingWeapon = nullptr;
    */

    return true;
}

bool UPX_WeaponSystemComponent::Authoritative_EquipBySlotEnd()
{
    PX_LOG(Log, TEXT("CurrentWeaponSlotIndex = %d"), CurrentWeaponSlotIndex);

    Multicast_EquipBySlotEnd();

    return true;
}

void UPX_WeaponSystemComponent::Authoritative_CancelEquipBySlot()
{
    /*
    // 장착 중인 무기 숨김 및 부착 해제
    if ( ServerEquippingWeapon )
    {
        ServerEquippingWeapon->SetActorHiddenInGame(true);
        ServerEquippingWeapon->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
    }

    // Setup Server Equipping Weapon Info
    ServerEquippingWeaponSlotIndex = INDEX_NONE;
    ServerEquippingWeaponItemInstance = nullptr;
    ServerEquippingWeaponItemInstanceId = FGuid();
    ServerEquippingWeapon = nullptr;
    */

    if ( !CurrentWeapon )
    {
        PX_LOG(Warning, TEXT("No ServerCurrentWeapon to restore"));
        return;
    }

    if (!CurrentWeaponItemInstance )
    {
        PX_LOG(Warning, TEXT("No WeaponItemInstance found in CurrentWeaponSlotIndex: %d"), CurrentWeaponSlotIndex);
        return;
	}

    // 기존 무기 부착
    if ( USkeletalMeshComponent* CharacterMesh = Character->GetMesh() )
    {
        const UPX_WeaponDataAsset* WeaponDataAsset = CurrentWeaponItemInstance->GetWeaponDataAsset();
        const FName SocketName = WeaponDataAsset ? WeaponDataAsset->AttachSocketName : NAME_None;

        CurrentWeapon->SetActorHiddenInGame(false);
        CurrentWeapon->AttachToComponent(CharacterMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, SocketName);
    }

    if ( APX_Character* PX_Character = Cast<APX_Character>(Character) )
    {
        PX_Character->SetHasEquippedWeapon(CurrentWeaponSlotIndex != 4);
    }

    PX_LOG(Log, TEXT(""));

    Multicast_CancelEquipBySlot();
}

void UPX_WeaponSystemComponent::ServerChangeEquipBySlot_Implementation(int32 SlotIndex)
{
    if ( !Character || !Character->HasAuthority() )
    {
        return;
    }

    PX_LOG(Log, TEXT("Changed SlotIndex: %d"), SlotIndex);

    if ( SlotIndex == INDEX_NONE )
    {
        PX_LOG(Warning, TEXT("Invalid SlotIndex: %d"), SlotIndex);
        return;
    }

    APX_Character* PX_Character = Cast<APX_Character>(Character);
    if ( !PX_Character )
    {
        PX_LOG(Warning, TEXT("Invalid PX_Character"));
        return;
    }

    UAbilitySystemComponent* ASC = PX_Character->GetAbilitySystemComponent();
    if ( !ASC )
    {
        PX_LOG(Warning, TEXT("Invalid ASC"));
        return;
    }

    FGameplayTagContainer TagContainer;
    TagContainer.AddTag(PX_GameplayTags::Ability_Weapon_Equip);

    TArray<FGameplayAbilitySpec*> MatchingSpecs;
    ASC->GetActivatableGameplayAbilitySpecsByAllMatchingTags(TagContainer, MatchingSpecs, false);

    for ( FGameplayAbilitySpec* Spec : MatchingSpecs )
    {
        if ( !Spec || !Spec->IsActive() ) continue;

        for ( UGameplayAbility* Instance : Spec->GetAbilityInstances() )
        {
            if ( UPX_GameplayAbility_Equip* EquipAbility = Cast<UPX_GameplayAbility_Equip>(Instance) )
            {
                if ( !EquipAbility->IsEquipCancelable() )
                {
                    PX_LOG(Log, TEXT("Equip ability is active but not cancelable yet"));
                    return;
                }

                EquipAbility->ChangeEquip(SlotIndex);
                PX_LOG(Log, TEXT("EquipChangeBySlot success"));
                return;
            }
        }
    }

    PX_LOG(Warning, TEXT("No active equip ability instance found"));
    return;
}

void UPX_WeaponSystemComponent::ServerSetIsEquipCancelable_Implementation(bool InValue)
{
    if ( !Character || !Character->HasAuthority() )
    {
        return;
    }

    PX_LOG(Log, TEXT(""));

    Local_SetIsEquipCancelable(InValue);
}

bool UPX_WeaponSystemComponent::Authoritative_Reload()
{
    if ( CurrentWeaponItemInstance->GetAmmo() >= CurrentWeaponItemInstance->GetWeaponDataAsset()->MagSize )
    {
        PX_LOG(Log, TEXT("Ammo is already Full."));
        return false;
    }
    return true;
}

bool UPX_WeaponSystemComponent::Authoritative_ReloadEnd()
{
    if ( !CurrentWeaponItemInstance )
    {
        PX_LOG(Warning, TEXT("Invalid CurrentWeaponItemInstance"));
        return false;
    }

    PX_LOG(Log, TEXT("CurrentWeaponItemInstance->GetAmmo() : % d"), CurrentWeaponItemInstance->GetWeaponDataAsset()->MagSize)

    // 나중에 MagSize와 인벤토리에 남은 탄 수를 비교해야 함
    CurrentWeaponItemInstance->SetAmmo(CurrentWeaponItemInstance->GetWeaponDataAsset()->MagSize);

    Multicast_ReloadEnd();

    return true;
}

void UPX_WeaponSystemComponent::Authoritative_CancelReload()
{
    PX_LOG(Log, TEXT(""));

    Multicast_CancelReload();
}

bool UPX_WeaponSystemComponent::Authoritative_FireOnce(const FGameplayAbilityTargetDataHandle* FireTargetData)
{
    if ( !CurrentWeaponItemInstance )
    {
        PX_LOG(Warning, TEXT("Invalid CurrentWeaponItemInstance"));
        return false;
    }

    int32 AmmoInMag = CurrentWeaponItemInstance->GetAmmo();
    PX_LOG(Log, TEXT("AmmoInMag: %d"), AmmoInMag);
    UPX_WeaponDataAsset* WeaponDataAsset = CurrentWeaponItemInstance->GetWeaponDataAsset();
    if ( !WeaponDataAsset )
    {
        PX_LOG(Warning, TEXT("Invalid WeaponDataAsset"));
        return false;
    }

    if ( WeaponDataAsset->MagSize < AmmoInMag || AmmoInMag < 0 )
    {
        PX_LOG(Warning, TEXT("Invalid Server Ammo In Mag"));
        return false;
    }

    if ( AmmoInMag <= 0 )
    {
        PX_LOG(Log, TEXT("DryFire. Skip ammo consume and projectile spawn"));
        return true;
    }

    PX_LOG(Log, TEXT("Before SetAmmo: %d"), CurrentWeaponItemInstance->GetAmmo());
    CurrentWeaponItemInstance->SetAmmo(--AmmoInMag);
    PX_LOG(Log, TEXT("After SetAmmo: %d"), CurrentWeaponItemInstance->GetAmmo());

    // Spawn Projectile
    if ( WeaponDataAsset->ProjectileClass && CurrentWeapon )
    {
        UWorld* World = GetWorld();
        APX_Character* PX_Character = Cast<APX_Character>(Character);
        USkeletalMeshComponent* WeaponMesh = CurrentWeapon->GetMesh();
        if ( World && PX_Character && WeaponMesh )
        {
            static const FName MuzzleSocketName(TEXT("Muzzle"));
            const FTransform MuzzleTransform = WeaponMesh->DoesSocketExist(MuzzleSocketName)
                ? WeaponMesh->GetSocketTransform(MuzzleSocketName, RTS_World)
                : CurrentWeapon->GetActorTransform();
            FVector MuzzleLocation = MuzzleTransform.GetLocation();
            const FVector ServerMuzzleLocation = MuzzleLocation;

            FVector CameraLocation = MuzzleLocation;
            FRotator CameraRotation = PX_Character->GetBaseAimRotation();
            if ( AController* Controller = PX_Character->GetController() )
            {
                Controller->GetPlayerViewPoint(CameraLocation, CameraRotation);
            }

            FVector CameraDirection = CameraRotation.Vector().GetSafeNormal();
            if ( CameraDirection.IsNearlyZero() )
            {
                CameraDirection = MuzzleTransform.GetRotation().GetForwardVector();
            }

            const float TraceRange = 15000.0f;
            const float DebugDuration = 1.0f;
            bool bUseClientFireTrace = false;
            FVector CameraTargetPoint = CameraLocation + CameraDirection * TraceRange;
            //DrawDebugSphere(World, ServerMuzzleLocation, 2.0f, 12, FColor::Red, false, DebugDuration);

            if ( FireTargetData && FireTargetData->Num() >= 2 )
            {
                const FGameplayAbilityTargetData* RawAimTargetData = FireTargetData->Get(0);
                const FGameplayAbilityTargetData* RawCameraTargetData = FireTargetData->Get(1);
                const FGameplayAbilityTargetData_LocationInfo* AimTargetData = RawAimTargetData && RawAimTargetData->GetScriptStruct() == FGameplayAbilityTargetData_LocationInfo::StaticStruct()
                    ? static_cast<const FGameplayAbilityTargetData_LocationInfo*>(RawAimTargetData)
                    : nullptr;
                const FGameplayAbilityTargetData_LocationInfo* CameraTargetData = RawCameraTargetData && RawCameraTargetData->GetScriptStruct() == FGameplayAbilityTargetData_LocationInfo::StaticStruct()
                    ? static_cast<const FGameplayAbilityTargetData_LocationInfo*>(RawCameraTargetData)
                    : nullptr;
                const FVector ClientMuzzleLocation = AimTargetData ? AimTargetData->SourceLocation.GetTargetingTransform().GetLocation() : FVector::ZeroVector;
                const FVector ClientCameraTargetPoint = AimTargetData ? AimTargetData->TargetLocation.GetTargetingTransform().GetLocation() : FVector::ZeroVector;
                const FTransform ClientCameraTransform = CameraTargetData ? CameraTargetData->SourceLocation.GetTargetingTransform() : FTransform::Identity;
                FVector ClientCameraDirection = ClientCameraTransform.GetRotation().GetForwardVector().GetSafeNormal();

                if ( ClientCameraDirection.IsNearlyZero() )
                {
                    ClientCameraDirection = CameraDirection;
                }

                const FVector ClientCameraLocation = ClientCameraTransform.GetLocation();
                const bool bClientMuzzleReasonable = FVector::DistSquared(ClientMuzzleLocation, PX_Character->GetActorLocation()) <= FMath::Square(500.0f);
                //DrawDebugSphere(World, ClientMuzzleLocation, 2.0f, 12, FColor::Cyan, false, DebugDuration);

                if ( bClientMuzzleReasonable )
                {
                    bUseClientFireTrace = true;
                    MuzzleLocation = ClientMuzzleLocation;
                    CameraLocation = ClientCameraLocation;
                    CameraDirection = ClientCameraDirection;
                    CameraTargetPoint = ClientCameraTargetPoint;
                }
            }

            FCollisionQueryParams TraceParams(SCENE_QUERY_STAT(PX_ServerProjectileAimTrace), true);
            TraceParams.AddIgnoredActor(PX_Character);
            TraceParams.AddIgnoredActor(CurrentWeapon);

            if ( !bUseClientFireTrace )
            {
                FHitResult AimHit;
                const FVector CameraTraceEnd = CameraLocation + CameraDirection * TraceRange;
                const bool bHit = World->LineTraceSingleByChannel(AimHit, CameraLocation, CameraTraceEnd, ECC_PX_CombatTrace, TraceParams);
                CameraTargetPoint = bHit ? AimHit.ImpactPoint : CameraTraceEnd;
            }

            const float MuzzlePlaneDistance = FVector::DotProduct(MuzzleLocation - CameraLocation, CameraDirection);
            const float TargetDistance = FVector::DotProduct(CameraTargetPoint - CameraLocation, CameraDirection);
            const bool bTargetIsInFrontOfMuzzlePlane = TargetDistance > MuzzlePlaneDistance + KINDA_SMALL_NUMBER;

            const FVector TargetPoint = bTargetIsInFrontOfMuzzlePlane
                ? CameraTargetPoint
                : MuzzleLocation + CameraDirection * TraceRange;

            FVector FireDirection = (TargetPoint - MuzzleLocation).GetSafeNormal();
            if ( FireDirection.IsNearlyZero() )
            {
                FireDirection = CameraDirection;
            }

            //DrawDebugLine(World, CameraLocation, CameraTargetPoint, FColor::Red, false, DebugDuration, 0, 1.0f);
            //DrawDebugSphere(World, CameraTargetPoint, 2.0f, 12, FColor::Red, false, DebugDuration);

            const FVector ProjectileLineEnd = MuzzleLocation + FireDirection * TraceRange;
            const FColor ProjectileDebugColor = bTargetIsInFrontOfMuzzlePlane ? FColor::Cyan : FColor::Yellow;
            //DrawDebugLine(World, MuzzleLocation, ProjectileLineEnd, ProjectileDebugColor, false, DebugDuration, 0, 1.0f);

            FActorSpawnParameters SpawnParams;
            SpawnParams.Owner = PX_Character;
            SpawnParams.Instigator = PX_Character;
            SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

            APX_Projectile* Projectile = World->SpawnActor<APX_Projectile>(
                WeaponDataAsset->ProjectileClass,
                MuzzleLocation,
                FireDirection.Rotation(),
                SpawnParams);

            if ( Projectile && Projectile->GetProjectileMovement() )
            {
                Projectile->SetStatusGameplayEffectClasses(ConsumeNextAttackStatusEffects());
                Projectile->GetProjectileMovement()->Velocity = FireDirection * Projectile->GetProjectileMovement()->InitialSpeed;
            }
        }
    }

    return true;
}

bool UPX_WeaponSystemComponent::Authoritative_FireOnceEnd()
{
    PX_LOG(Log, TEXT(""));

    Multicast_FireOnceEnd();

    return true;
}

void UPX_WeaponSystemComponent::Authoritative_CancelFireOnce()
{
    PX_LOG(Log, TEXT(""));

    //Client_CancelFireOnce();

    Multicast_CancelFireOnce();
}

bool UPX_WeaponSystemComponent::Authoritative_SwitchAttackMode()
{
    if ( !CurrentWeaponItemInstance )
    {
        PX_LOG(Warning, TEXT("Invalid CurrentWeaponItemInstance"));
        return false;
    }

    PX_LOG(Log, TEXT(""));

    if ( !CurrentWeaponItemInstance->SwitchAttackMode() )
    {
        PX_LOG(Warning, TEXT("Failed to Authoritative Switch Attack Mode"));
        return false;
    }

    return true;
}

bool UPX_WeaponSystemComponent::Authoritative_SwitchAttackModeEnd()
{
    PX_LOG(Log, TEXT(""));

    Multicast_SwitchAttackModeEnd();

    return true;
}

void UPX_WeaponSystemComponent::Authoritative_CancelSwitchAttackMode()
{
    PX_LOG(Log, TEXT(""));

    Multicast_CancelSwitchAttackMode();
}

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
