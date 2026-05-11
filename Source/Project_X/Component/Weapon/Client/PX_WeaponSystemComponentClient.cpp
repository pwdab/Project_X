// Fill out your copyright notice in the Description page of Project Settings.

#include "Component/Weapon/PX_WeaponSystemComponent.h"
#include "../PX_WeaponDataAsset.h"

#include "Handler/PX_WeaponActionHandler.h"
#include "Component/Demo/PX_DemoBotComponent.h"
#include "Entity/PX_Character.h"
#include "Entity/PX_Weapon.h"                   // EquipWeapon

#include "Component/Inventory/PX_WeaponItemInstance.h"
#include "Component/Inventory/PX_ItemDataAsset.h"
#include "Component/Inventory//PX_InventoryComponent.h"

#include "GameplayTagContainer.h"
#include "Camera/PlayerCameraManager.h"
#include "Components/SkeletalMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/PlayerController.h"
//#include "Engine/World.h"

#include "AbilitySystem/Tags/PX_GameplayTags.h"
#include "AbilitySystem/Abilities/PX_GameplayAbility_Equip.h"


/*
void UPX_WeaponSystemComponent::Client_TickComponent(float DeltaSeconds, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
}
*/

bool UPX_WeaponSystemComponent::Predict_EquipBySlot(int32 SlotIndex)
{
    PX_LOG(Log, TEXT("SlotIndex: %d"), SlotIndex);
    if ( !Character || Character->HasAuthority() || !Character->IsLocallyControlled() )
    {
		PX_LOG(Warning, TEXT("Invalid Character State. Character: %s, HasAuthority: %d, IsLocallyControlled: %d"), *GetNameSafe(Character), Character ? Character->HasAuthority() : -1, Character ? Character->IsLocallyControlled() : -1);
        return false;
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

    // Setup Predicted Current Weapon Info
    PredictedCurrentWeaponSlotIndex = (PredictedCurrentWeaponSlotIndex == INDEX_NONE) ? CurrentWeaponSlotIndex : PredictedCurrentWeaponSlotIndex;
    PredictedCurrentWeaponItemInstance = Cast<UPX_WeaponItemInstance>(Inventory->GetWeaponItemInstanceBySlot(PredictedCurrentWeaponSlotIndex));
    PredictedCurrentWeaponItemInstanceId = PredictedCurrentWeaponItemInstance ? PredictedCurrentWeaponItemInstance->GetInstanceId() : FGuid();
    // 기존 무기 숨김
    if ( PredictedCurrentWeapon )
    {
		PX_LOG(Log, TEXT("Hide Predicted Current Weapon: %s"), *PredictedCurrentWeapon->GetName());
        PredictedCurrentWeapon->SetActorHiddenInGame(true);
        PredictedCurrentWeapon->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
        //PredictedCurrentWeapon->Destroy();
    }

	// Setup Predicted Equipping Weapon Info
    int32 PredictedEquippingWeaponSlotIndex = SlotIndex;
    TObjectPtr<UPX_WeaponItemInstance> PredictedEquippingWeaponItemInstance = Cast<UPX_WeaponItemInstance>(Inventory->GetWeaponItemInstanceBySlot(PredictedEquippingWeaponSlotIndex));
    if ( !PredictedEquippingWeaponItemInstance )
    {
		PX_LOG(Warning, TEXT("No PredictedEquippingWeaponItemInstance found in SlotIndex: %d"), SlotIndex);
        return false;
    }

    FGuid PredictedEquippingWeaponItemInstanceId = PredictedEquippingWeaponItemInstance->GetInstanceId();
    if ( !PredictedEquippingWeaponItemInstanceId.IsValid() )
    {
        PX_LOG(Warning, TEXT("Invalid PredictedEquippingWeaponItemInstanceId"));
        return false;
    }

    // 같은 무기 재장착 방지
    if ( PredictedEquippingWeaponItemInstanceId == PredictedCurrentWeaponItemInstanceId )
    {
        PX_LOG(Warning, TEXT("Attempting to equip the same weapon instance. PredictedEquippingWeaponItemInstanceId: %s, PredictedCurrentWeaponInstanceId: %s"), *PredictedEquippingWeaponItemInstanceId.ToString(), *PredictedCurrentWeaponItemInstanceId.ToString());
		return false;
    }

    TObjectPtr<APX_Weapon> PredictedEquippingWeapon = nullptr;
    // 이미 스폰된 PX_Weapon이면 SpawnedWeapons에서 찾아 가져옴
    if ( TObjectPtr<APX_Weapon>* Found = PredictedSpawnedWeapons.Find(PredictedEquippingWeaponItemInstanceId) )
    {
        PredictedEquippingWeapon = Found->Get();
		PX_LOG(Log, TEXT("Found Predicted Equipping Weapon: %s in SpawnedWeapons for SlotIndex: %d"), *PredictedEquippingWeapon->GetName(), SlotIndex);
    }
    // SpawnedWeapons에 없으면 새로 스폰
    //if ( !PredictedEquippingWeapon )
    else
    {
        UPX_WeaponDataAsset* PredictedEquippingWeaponDataAsset = PredictedEquippingWeaponItemInstance->GetWeaponDataAsset();
        if ( !PredictedEquippingWeaponDataAsset )
        {
            PX_LOG(Warning, TEXT("Invalid PredictedEquippingWeaponDataAsset for PredictedEquippingWeaponItemInstance in SlotIndex: %d"), SlotIndex);
			return false;
        }

        UWorld* World = GetWorld();
        if ( !World )
        {
			PX_LOG(Warning, TEXT("Unable to get World context"));
			return false;
        }

        // 나중에 PX_Weapon을 확장하면 바꿔야 함
        //TSubclassOf<APX_Weapon> WeaponClass = WeaponDataAsset->WeaponClass;
        TSubclassOf<APX_Weapon> WeaponClass = APX_Weapon::StaticClass();
        if ( !WeaponClass )
        {
			PX_LOG(Warning, TEXT("Invalid WeaponClass in WeaponDataAsset for the new weapon instance in SlotIndex: %d"), SlotIndex);
			return false;
        }

        // 새로운 무기 스폰
        FActorSpawnParameters Params;
        Params.Owner = Character;
        Params.Instigator = Character->GetInstigator();
        Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        /*
        FString WeaponName = TEXT("Local_") + WeaponDataAsset->ItemName.ToString();
        ULevel* LevelOuter = World ? World->PersistentLevel : nullptr;
        FName BaseName(*WeaponName);
        Params.Name = MakeUniqueObjectName(LevelOuter, APX_Weapon::StaticClass(), BaseName);
        */
        PredictedEquippingWeapon = World->SpawnActor<APX_Weapon>(WeaponClass, FTransform::Identity, Params);
        PX_LOG(Log, TEXT("New PredictedEquippingWeapon Spawned: %s"), *PredictedEquippingWeapon->GetName());
        if ( !PredictedEquippingWeapon )
        {
			PX_LOG(Warning, TEXT("Failed to spawn new PredictedEquippingWeapon for SlotIndex: %d"), SlotIndex);
			return false;
        }
        PredictedEquippingWeapon->InitializeFromData(PredictedEquippingWeaponDataAsset);
        PredictedSpawnedWeapons.Add(PredictedEquippingWeaponItemInstanceId, PredictedEquippingWeapon);
    }

    // 새로운 무기 부착
    if ( USkeletalMeshComponent* CharacterMesh = Character->GetMesh() )
    {
        const UPX_WeaponDataAsset* PredictedEquippingWeaponDataAsset = PredictedEquippingWeaponItemInstance->GetWeaponDataAsset();
        const FName SocketName = PredictedEquippingWeaponDataAsset ? PredictedEquippingWeaponDataAsset->AttachSocketName : NAME_None;

        PredictedEquippingWeapon->SetActorHiddenInGame(false);
        PredictedEquippingWeapon->AttachToComponent(CharacterMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, SocketName);
    }

    // Setup Predicted Current Weapon Info
    PredictedCurrentWeaponSlotIndex = PredictedEquippingWeaponSlotIndex;
    PredictedCurrentWeaponItemInstance = PredictedEquippingWeaponItemInstance;
    PredictedCurrentWeaponItemInstanceId = PredictedEquippingWeaponItemInstanceId;
    PredictedCurrentWeapon = PredictedEquippingWeapon;
    PredictedCurrentAmmoInMag = PredictedCurrentWeaponItemInstance->GetAmmo();
    bHasPredictedCurrentAmmo = true;
    PredictedFireAmmoRollback = INDEX_NONE;
    bHasPredictedFireAmmoRollback = false;
    PredictedCurrentAttackModeTag = PredictedCurrentWeaponItemInstance->GetAttackMode();
    bHasPredictedCurrentAttackMode = true;
    PredictedAttackModeRollback = FGameplayTag();
    bHasPredictedAttackModeRollback = false;

    if ( APX_Character* PX_Character = Cast<APX_Character>(Character) )
    {
        PX_Character->SetHasEquippedWeapon(PredictedCurrentWeaponSlotIndex != 4);
    }

    /*
    // Setup Predicted Equipping Weapon Info
    PredictedEquippingWeaponSlotIndex = INDEX_NONE;
    PredictedEquippingWeaponItemInstance = nullptr;
    PredictedEquippingWeaponItemInstanceId = FGuid();
    PredictedEquippingWeapon = nullptr;
    */

    return true;
}

bool UPX_WeaponSystemComponent::Predict_EquipBySlotEnd()
{
    PX_LOG(Log, TEXT("PredictedCurrentWeaponSlotIndex = %d"), PredictedCurrentWeaponSlotIndex);

    if ( !PredictedCurrentWeapon )
    {
        PX_LOG(Warning, TEXT("No PredictedCurrentWeapon"));
        return false;
    }

    if ( PredictedCurrentWeaponItemInstance )
    {
        // UI 업데이트
        HandleAmmoUpdated(GetCurrentAmmoInMag(), 1000);
    }

    /*
    // 이미 authoritative weapon으로 바뀐 경우 return
    if ( PredictedCurrentWeaponItemInstanceId == CurrentWeaponItemInstanceId )
    {
        // Predicted Current Weapon 숨김
        PX_LOG(Log, TEXT("Predicted weapon already authoritative. Hide Predicted weapon: %s"), *PredictedCurrentWeapon->GetName());
        PredictedCurrentWeapon->SetActorHiddenInGame(true);
        PredictedCurrentWeapon->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
        //PredictedCurrentWeapon->Destroy();
        PredictedCurrentWeapon = CurrentWeapon;
        return true;
    }
    

    // Predicted Current Weapon 유지
    PX_LOG(Log, TEXT("Predicted weapon: %s is not yet authoritative"), *PredictedCurrentWeapon->GetName());
    */

    return true;
}

void UPX_WeaponSystemComponent::Predict_CancelEquipBySlot()
{
    PX_LOG(Log, TEXT(""));

    if ( !Character ) return;

    /*
    if ( PredictedEquippingWeaponSlotIndex != INDEX_NONE )
    {
        Local_StopMontage(PredictedEquippingWeaponSlotIndex);
    }

    // 장착 중인 무기 숨김 및 부착 해제
    if ( PredictedEquippingWeapon )
    {
        PredictedEquippingWeapon->SetActorHiddenInGame(true);
        PredictedEquippingWeapon->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
    }
    
    // Setup Predicted Equipping Weapon Info
    PredictedEquippingWeaponSlotIndex = INDEX_NONE;
    PredictedEquippingWeaponItemInstance = nullptr;
    PredictedEquippingWeaponItemInstanceId = FGuid();
    PredictedEquippingWeapon = nullptr;
    */

    // Equip 몽타주 데이터 가져오기
    UPX_WeaponItemInstance* WeaponInstance = Cast<UPX_WeaponItemInstance>(Inventory->GetWeaponItemInstanceBySlot(CurrentWeaponSlotIndex));
    if ( !WeaponInstance ) return;
    
    UPX_WeaponDataAsset* WeaponDataAsset = WeaponInstance->GetWeaponDataAsset();
    if ( !WeaponDataAsset ) return;
    
    UAnimMontage* EquipMontage = WeaponDataAsset->EquipAction.CharacterMontage;
    if ( !EquipMontage ) return;
    
    USkeletalMeshComponent* MeshComp = Character->GetMesh();
    if ( !MeshComp ) return;
    
    UAnimInstance* AnimInstance = MeshComp->GetAnimInstance();
    if ( !AnimInstance ) return;
    
    APX_Character* PX_Character = Cast<APX_Character>(Character);
    if ( !PX_Character ) return;

    Local_StopCharacterMontage(EquipMontage);

    if ( !PredictedCurrentWeapon )
    {
        PX_LOG(Warning, TEXT("No PredictedCurrentWeapon to restore"));
		return;
    }

    if ( !PredictedCurrentWeaponItemInstance )
    {
        PX_LOG(Warning, TEXT("No WeaponItemInstance found in PredictedCurrentWeaponItemInstance: %d"), PredictedCurrentWeaponSlotIndex);
        return;
    }

    // 기존 무기 부착
    if ( USkeletalMeshComponent* CharacterMesh = Character->GetMesh() )
    {
        //const UPX_WeaponDataAsset* WeaponDataAsset = PredictedCurrentWeaponItemInstance->GetWeaponDataAsset();
        const FName SocketName = WeaponDataAsset ? WeaponDataAsset->AttachSocketName : NAME_None;

        PredictedCurrentWeapon->SetActorHiddenInGame(false);
        PredictedCurrentWeapon->AttachToComponent(CharacterMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, SocketName);
    }
    
	// 기존 무기로 UI 업데이트
    SyncPredictedAmmoFromCurrent();
    SyncPredictedAttackModeFromCurrent();
    HandleAmmoUpdated(GetCurrentAmmoInMag(), 1000);

    PX_Character->SetHasEquippedWeapon(CurrentWeaponSlotIndex != 4);
}

bool UPX_WeaponSystemComponent::Local_ChangeEquipBySlot(int32 SlotIndex)
{
    if ( !Character || Character->HasAuthority() || !Character->IsLocallyControlled() )
    {
        return false;
    }

    PX_LOG(Log, TEXT("Changed SlotIndex: %d"), SlotIndex);

    if ( SlotIndex == INDEX_NONE )
    {
        PX_LOG(Warning, TEXT("Invalid SlotIndex: %d"), SlotIndex);
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
        PX_LOG(Warning, TEXT("Invalid ASC"));
        return false;
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
                    return false;
                }

                EquipAbility->ChangeEquip(SlotIndex);
                PX_LOG(Log, TEXT("EquipChangeBySlot success"));

                ServerChangeEquipBySlot(SlotIndex);
                return true;
            }
        }
    }

    PX_LOG(Warning, TEXT("No active equip ability instance found"));
    return false;
}

void UPX_WeaponSystemComponent::SetIsEquipCancelable(bool InValue)
{
    PX_LOG(Log, TEXT(""));

    Local_SetIsEquipCancelable(InValue);

    if ( Character && !Character->HasAuthority() && Character->IsLocallyControlled() )
    {
        ServerSetIsEquipCancelable(InValue);
    }
}

int32 UPX_WeaponSystemComponent::GetCurrentAmmoInMag() const
{
    if ( bHasPredictedCurrentAmmo && PredictedCurrentAmmoInMag != INDEX_NONE )
    {
        return FMath::Max(PredictedCurrentAmmoInMag, 0);
    }

    if ( CurrentWeaponItemInstance )
    {
        return CurrentWeaponItemInstance->GetAmmo();
    }

    if ( PredictedCurrentWeaponItemInstance )
    {
        return PredictedCurrentWeaponItemInstance->GetAmmo();
    }

    return 0;
}

void UPX_WeaponSystemComponent::SyncPredictedAmmoFromCurrent()
{
    UPX_WeaponItemInstance* SourceWeaponItemInstance = CurrentWeaponItemInstance ? CurrentWeaponItemInstance.Get() : PredictedCurrentWeaponItemInstance.Get();
    if ( SourceWeaponItemInstance )
    {
        PredictedCurrentAmmoInMag = SourceWeaponItemInstance->GetAmmo();
        bHasPredictedCurrentAmmo = true;
    }
    else
    {
        PredictedCurrentAmmoInMag = INDEX_NONE;
        bHasPredictedCurrentAmmo = false;
    }

    PredictedFireAmmoRollback = INDEX_NONE;
    bHasPredictedFireAmmoRollback = false;
}

FGameplayTag UPX_WeaponSystemComponent::GetCurrentAttackModeTag() const
{
    const int32 EffectiveWeaponSlotIndex = (PredictedCurrentWeaponSlotIndex != INDEX_NONE) ? PredictedCurrentWeaponSlotIndex : CurrentWeaponSlotIndex;
    if ( EffectiveWeaponSlotIndex == 4 )
    {
        return FGameplayTag();
    }

    if ( bHasPredictedCurrentAttackMode && PredictedCurrentAttackModeTag.IsValid() )
    {
        return PredictedCurrentAttackModeTag;
    }

    if ( CurrentWeaponItemInstance )
    {
        return CurrentWeaponItemInstance->GetAttackMode();
    }

    if ( PredictedCurrentWeaponItemInstance )
    {
        return PredictedCurrentWeaponItemInstance->GetAttackMode();
    }

    return FGameplayTag();
}

void UPX_WeaponSystemComponent::SyncPredictedAttackModeFromCurrent()
{
    UPX_WeaponItemInstance* SourceWeaponItemInstance = CurrentWeaponItemInstance ? CurrentWeaponItemInstance.Get() : PredictedCurrentWeaponItemInstance.Get();
    if ( SourceWeaponItemInstance )
    {
        PredictedCurrentAttackModeTag = SourceWeaponItemInstance->GetAttackMode();
        bHasPredictedCurrentAttackMode = true;
    }
    else
    {
        PredictedCurrentAttackModeTag = FGameplayTag();
        bHasPredictedCurrentAttackMode = false;
    }

    PredictedAttackModeRollback = FGameplayTag();
    bHasPredictedAttackModeRollback = false;
}

bool UPX_WeaponSystemComponent::Predict_Reload()
{
    if ( !PredictedCurrentWeaponItemInstance )
    {
        PX_LOG(Warning, TEXT("Invalid PredictedCurrentWeaponItemInstance"));
        return false;
    }

    UPX_WeaponDataAsset* WeaponDataAsset = PredictedCurrentWeaponItemInstance->GetWeaponDataAsset();
    if ( !WeaponDataAsset )
    {
        PX_LOG(Warning, TEXT("Invalid WeaponDataAsset"));
        return false;
    }

    if ( GetCurrentAmmoInMag() >= WeaponDataAsset->MagSize )
    {
        PX_LOG(Log, TEXT("Ammo is already Full."));
        return false;
    }

    return true;
}

bool UPX_WeaponSystemComponent::Predict_ReloadEnd()
{
    if ( !PredictedCurrentWeaponItemInstance )
    {
        PX_LOG(Warning, TEXT("Invalid PredictedCurrentWeaponItemInstance"));
        return false;
    }

    UPX_WeaponDataAsset* WeaponDataAsset = PredictedCurrentWeaponItemInstance->GetWeaponDataAsset();
    if ( !WeaponDataAsset )
    {
        PX_LOG(Warning, TEXT("Invalid WeaponDataAsset"));
        return false;
    }

    PredictedCurrentAmmoInMag = WeaponDataAsset->MagSize;
    bHasPredictedCurrentAmmo = true;

    PX_LOG(Log, TEXT("PredictedCurrentAmmoInMag : %d"), PredictedCurrentAmmoInMag)

    // UI 업데이트
    HandleAmmoUpdated(PredictedCurrentAmmoInMag, 1000);

    return true;
}

void UPX_WeaponSystemComponent::Predict_CancelReload()
{
    if ( !PredictedCurrentWeaponItemInstance )
    {
        PX_LOG(Warning, TEXT("Invalid PredictedCurrentWeaponItemInstance"));
        return;
    }

    SyncPredictedAmmoFromCurrent();
    HandleAmmoUpdated(GetCurrentAmmoInMag(), 1000);
}

bool UPX_WeaponSystemComponent::MakePredictedProjectileFireTargetData(FGameplayAbilityTargetDataHandle& OutTargetData) const
{
    if ( !Character || Character->HasAuthority() || !Character->IsLocallyControlled() )
    {
        return false;
    }

    UPX_WeaponItemInstance* WeaponItemInstance = PredictedCurrentWeaponItemInstance ? PredictedCurrentWeaponItemInstance.Get() : CurrentWeaponItemInstance.Get();
    if ( !WeaponItemInstance )
    {
        return false;
    }

    UPX_WeaponDataAsset* WeaponDataAsset = WeaponItemInstance->GetWeaponDataAsset();
    if ( !WeaponDataAsset || !WeaponDataAsset->ProjectileClass )
    {
        return false;
    }

    APX_Character* PX_Character = Cast<APX_Character>(Character);
    APX_Weapon* SourceWeapon = PredictedCurrentWeapon ? PredictedCurrentWeapon.Get() : CurrentWeapon.Get();
    USkeletalMeshComponent* WeaponMesh = SourceWeapon ? SourceWeapon->GetMesh() : nullptr;
    APlayerController* PlayerController = PX_Character ? Cast<APlayerController>(PX_Character->GetController()) : nullptr;
    APlayerCameraManager* CameraManager = PlayerController ? PlayerController->PlayerCameraManager : nullptr;
    if ( !PX_Character || !SourceWeapon || !WeaponMesh || !CameraManager )
    {
        return false;
    }

    static const FName MuzzleSocketName(TEXT("Muzzle"));
    const FTransform MuzzleTransform = WeaponMesh->DoesSocketExist(MuzzleSocketName)
        ? WeaponMesh->GetSocketTransform(MuzzleSocketName, RTS_World)
        : SourceWeapon->GetActorTransform();
    const FVector MuzzleLocation = MuzzleTransform.GetLocation();
    if ( UWorld* World = GetWorld() )
    {
        //DrawDebugSphere(World, MuzzleLocation, 2.0f, 12, FColor::Cyan, false, 1.0f);
    }

    const FVector CameraLocation = CameraManager->GetCameraLocation();
    FVector CameraDirection = CameraManager->GetCameraRotation().Vector().GetSafeNormal();
    if ( CameraDirection.IsNearlyZero() )
    {
        CameraDirection = MuzzleTransform.GetRotation().GetForwardVector();
    }

    FCollisionQueryParams TraceParams(SCENE_QUERY_STAT(PX_ClientProjectileAimTrace), true);
    TraceParams.AddIgnoredActor(PX_Character);
    TraceParams.AddIgnoredActor(SourceWeapon);

    const float TraceRange = 15000.0f;
    FHitResult AimHit;
    const FVector CameraTraceEnd = CameraLocation + CameraDirection * TraceRange;
    const bool bHit = GetWorld() && GetWorld()->LineTraceSingleByChannel(AimHit, CameraLocation, CameraTraceEnd, ECC_PX_CombatTrace, TraceParams);
    const FVector CameraTargetPoint = bHit ? AimHit.ImpactPoint : CameraTraceEnd;

    FGameplayAbilityTargetData_LocationInfo* AimTargetData = new FGameplayAbilityTargetData_LocationInfo();
    AimTargetData->SourceLocation.LocationType = EGameplayAbilityTargetingLocationType::LiteralTransform;
    AimTargetData->SourceLocation.LiteralTransform = FTransform(MuzzleTransform.GetRotation(), MuzzleLocation);
    AimTargetData->TargetLocation.LocationType = EGameplayAbilityTargetingLocationType::LiteralTransform;
    AimTargetData->TargetLocation.LiteralTransform = FTransform(CameraDirection.Rotation(), CameraTargetPoint);
    OutTargetData.Add(AimTargetData);

    FGameplayAbilityTargetData_LocationInfo* CameraTargetData = new FGameplayAbilityTargetData_LocationInfo();
    CameraTargetData->SourceLocation.LocationType = EGameplayAbilityTargetingLocationType::LiteralTransform;
    CameraTargetData->SourceLocation.LiteralTransform = FTransform(CameraDirection.Rotation(), CameraLocation);
    CameraTargetData->TargetLocation.LocationType = EGameplayAbilityTargetingLocationType::LiteralTransform;
    CameraTargetData->TargetLocation.LiteralTransform = FTransform(CameraDirection.Rotation(), CameraTargetPoint);
    OutTargetData.Add(CameraTargetData);

    return true;
}

bool UPX_WeaponSystemComponent::Predict_FireOnce()
{
    UPX_WeaponItemInstance* WeaponItemInstance = PredictedCurrentWeaponItemInstance ? PredictedCurrentWeaponItemInstance.Get() : CurrentWeaponItemInstance.Get();
    if ( !WeaponItemInstance )
    {
        PX_LOG(Warning, TEXT("Invalid WeaponItemInstance"));
        return false;
    }

    UPX_WeaponDataAsset* WeaponDataAsset = WeaponItemInstance->GetWeaponDataAsset();
    if ( !WeaponDataAsset )
    {
        PX_LOG(Warning, TEXT("Invalid WeaponDataAsset"));
        return false;
    }

    int32 AmmoInMag = GetCurrentAmmoInMag();
    PX_LOG(Log, TEXT("AmmoInMag: %d"), AmmoInMag);
    if ( WeaponDataAsset->MagSize < AmmoInMag || AmmoInMag < 0 )
    {
        PX_LOG(Warning, TEXT("Invalid Predicted Ammo In Mag"));
        return false;
    }

    PX_LOG(Log, TEXT("Before PredictAmmo: %d"), AmmoInMag);
    PredictedFireAmmoRollback = AmmoInMag;
    bHasPredictedFireAmmoRollback = true;
    PredictedCurrentAmmoInMag = FMath::Clamp(AmmoInMag - 1, 0, WeaponDataAsset->MagSize);
    bHasPredictedCurrentAmmo = true;
    PX_LOG(Log, TEXT("After PredictAmmo: %d"), PredictedCurrentAmmoInMag);

    PX_LOG(Log, TEXT(""));

    // UI 업데이트
    HandleAmmoUpdated(PredictedCurrentAmmoInMag, 1000);

    return true;
}

bool UPX_WeaponSystemComponent::Predict_FireOnceEnd()
{
    PX_LOG(Log, TEXT(""));

    return true;
}

void UPX_WeaponSystemComponent::Predict_RejectFireOnce()
{
    if ( !bHasPredictedFireAmmoRollback )
    {
        return;
    }

    PredictedCurrentAmmoInMag = PredictedFireAmmoRollback;
    bHasPredictedCurrentAmmo = PredictedCurrentAmmoInMag != INDEX_NONE;
    PredictedFireAmmoRollback = INDEX_NONE;
    bHasPredictedFireAmmoRollback = false;

    HandleAmmoUpdated(GetCurrentAmmoInMag(), 1000);
}

void UPX_WeaponSystemComponent::Predict_CancelFireOnce()
{
    if ( !PredictedCurrentWeaponItemInstance && !CurrentWeaponItemInstance )
    {
        PX_LOG(Warning, TEXT("Invalid WeaponItemInstance"));
        return;
    }

    PX_LOG(Log, TEXT(""));

    bHasPredictedFireAmmoRollback = false;
    PredictedFireAmmoRollback = INDEX_NONE;

    // UI 업데이트
    HandleAmmoUpdated(GetCurrentAmmoInMag(), 1000);
}

void UPX_WeaponSystemComponent::Client_CancelFireOnce_Implementation()
{
    PX_LOG(Log, TEXT(""));

    Predict_CancelFireOnce();
}

bool UPX_WeaponSystemComponent::Predict_SwitchAttackMode()
{
    UPX_WeaponItemInstance* WeaponItemInstance = CurrentWeaponItemInstance ? CurrentWeaponItemInstance.Get() : PredictedCurrentWeaponItemInstance.Get();
    if ( !WeaponItemInstance )
    {
        PX_LOG(Warning, TEXT("Invalid WeaponItemInstance"));
        return false;
    }

    PX_LOG(Log, TEXT(""));

    const UPX_WeaponDataAsset* WeaponData = WeaponItemInstance->GetWeaponDataAsset();
    if ( !WeaponData || WeaponData->SupportedAttackModeTags.Num() == 0 )
    {
        PX_LOG(Warning, TEXT("Invalid WeaponData or SupportedAttackModeTags"));
        return false;
    }

    const FGameplayTag CurrentAttackModeTag = GetCurrentAttackModeTag();
    const int32 CurrentIndex = WeaponData->SupportedAttackModeTags.IndexOfByKey(CurrentAttackModeTag);
    const int32 NextIndex = (CurrentIndex == INDEX_NONE) ? 0 : (CurrentIndex + 1) % WeaponData->SupportedAttackModeTags.Num();
    const FGameplayTag NextAttackModeTag = WeaponData->SupportedAttackModeTags[NextIndex];
    if ( !NextAttackModeTag.IsValid() || NextAttackModeTag == CurrentAttackModeTag )
    {
        PX_LOG(Warning, TEXT("Invalid NextAttackModeTag"));
        return false;
    }

    PredictedAttackModeRollback = CurrentAttackModeTag;
    bHasPredictedAttackModeRollback = true;
    PredictedCurrentAttackModeTag = NextAttackModeTag;
    bHasPredictedCurrentAttackMode = true;

    // UI 업데이트
    HandleAttackModeUpdated(GetCurrentAttackModeTag());

    return true;
}

bool UPX_WeaponSystemComponent::Predict_SwitchAttackModeEnd()
{
    PX_LOG(Log, TEXT(""));

    return true;
}

void UPX_WeaponSystemComponent::Predict_CancelSwitchAttackMode()
{
    if ( !CurrentWeaponItemInstance && !PredictedCurrentWeaponItemInstance )
    {
        PX_LOG(Warning, TEXT("Invalid WeaponItemInstance"));
        return;
    }

    PX_LOG(Log, TEXT(""));

    bHasPredictedAttackModeRollback = false;
    PredictedAttackModeRollback = FGameplayTag();

    // UI 업데이트
    HandleAttackModeUpdated(GetCurrentAttackModeTag());
}

void UPX_WeaponSystemComponent::Multicast_EquipBySlot_Implementation(int32 SlotIndex)
{
    if ( !Character || (Character->HasAuthority() && Character->IsPlayerControlled()) || Character->IsLocallyControlled() )
    {
        //PX_LOG(Warning, TEXT("Invalid Character State. Character: %s, HasAuthority: %d, IsLocallyControlled: %d"), *GetNameSafe(Character), Character ? Character->HasAuthority() : -1, Character ? Character->IsLocallyControlled() : -1);
        return;
    }

    PX_LOG(Log, TEXT("SlotIndex: %d"), SlotIndex);

    if ( !Inventory || !Inventory->IsInventoryReady() )
    {
        PX_LOG(Warning, TEXT("Inventory is not ready"));
        return;
    }

    if ( SlotIndex == INDEX_NONE )
    {
        PX_LOG(Warning, TEXT("Invalid SlotIndex: %d"), SlotIndex);
        return;
    }

    // Setup Remote Current Weapon Info
    RemoteCurrentWeaponSlotIndex = (RemoteCurrentWeaponSlotIndex == INDEX_NONE) ? CurrentWeaponSlotIndex : RemoteCurrentWeaponSlotIndex;
    RemoteCurrentWeaponItemInstance = Cast<UPX_WeaponItemInstance>(Inventory->GetWeaponItemInstanceBySlot(RemoteCurrentWeaponSlotIndex));
    RemoteCurrentWeaponItemInstanceId = RemoteCurrentWeaponItemInstance ? RemoteCurrentWeaponItemInstance->GetInstanceId() : FGuid();
    // 기존 무기 숨김
    if ( RemoteCurrentWeapon )
    {
        PX_LOG(Log, TEXT("Hide Remote Current Weapon: %s"), *RemoteCurrentWeapon->GetName());
        RemoteCurrentWeapon->SetActorHiddenInGame(true);
        RemoteCurrentWeapon->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
        //RemoteCurrentWeapon->Destroy();
    }

    // Setup Remote Equipping Weapon Info
    int32 RemoteEquippingWeaponSlotIndex = SlotIndex;
    TObjectPtr<UPX_WeaponItemInstance> RemoteEquippingWeaponItemInstance = Cast<UPX_WeaponItemInstance>(Inventory->GetWeaponItemInstanceBySlot(RemoteEquippingWeaponSlotIndex));
    if ( !RemoteEquippingWeaponItemInstance )
    {
        PX_LOG(Warning, TEXT("Invalid RemoteEquippingWeaponItemInstance at SlotIndex: %d"), RemoteEquippingWeaponSlotIndex);
        return;
    }

    FGuid RemoteEquippingWeaponItemInstanceId = RemoteEquippingWeaponItemInstance->GetInstanceId();
    if ( !RemoteEquippingWeaponItemInstanceId.IsValid() )
    {
        PX_LOG(Warning, TEXT("Invalid RemoteEquippingWeaponInstanceId"));
        return;
    }

    UPX_WeaponDataAsset* RemoteEquippingWeaponDataAsset = RemoteEquippingWeaponItemInstance->GetWeaponDataAsset();
    if ( !RemoteEquippingWeaponDataAsset )
    {
        PX_LOG(Warning, TEXT("Invalid RemoteEquippingWeaponDataAsset for the new weapon instance in SlotIndex: %d"), SlotIndex);
        return;
    }

    // 같은 무기 재장착 방지는 이미 Server에서 완료

    TObjectPtr<APX_Weapon> RemoteEquippingWeapon = nullptr;
    // 이미 스폰된 PX_Weapon이면 RemoteSpawnedWeapons에서 찾아 가져옴
    if ( TObjectPtr<APX_Weapon>* Found = RemoteSpawnedWeapons.Find(RemoteEquippingWeaponItemInstanceId) )
    {
        RemoteEquippingWeapon = Found->Get();
        PX_LOG(Log, TEXT("Found Remote Equipping Weapon: %s in SpawnedWeapons for SlotIndex: %d"), *RemoteEquippingWeapon->GetName(), SlotIndex);
    }
    // RemoteSpawnedWeapons에 없으면 새로 스폰
    else
    {
        UWorld* World = GetWorld();
        if ( !World )
        {
            PX_LOG(Warning, TEXT("Invalid World"));
            return;
        }

        TSubclassOf<APX_Weapon> WeaponClass = APX_Weapon::StaticClass();
        if ( !WeaponClass )
        {
            PX_LOG(Warning, TEXT("Invalid WeaponClass"));
            return;
        }

        // 새로운 무기 스폰
        FActorSpawnParameters Params;
        Params.Owner = Character;
        Params.Instigator = Character->GetInstigator();
        Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        /*
        FString WeaponName = TEXT("Remote_") + RemoteEquippingWeaponDataAsset->ItemName.ToString();
        ULevel* LevelOuter = World ? World->PersistentLevel : nullptr;
        FName BaseName(*WeaponName);
        Params.Name = MakeUniqueObjectName(LevelOuter, APX_Weapon::StaticClass(), BaseName);
        */
        RemoteEquippingWeapon = World->SpawnActor<APX_Weapon>(WeaponClass, FTransform::Identity, Params);
        if ( !RemoteEquippingWeapon )
        {
            PX_LOG(Warning, TEXT("Failed to spawn Remote Equipping Weapon for SlotIndex: %d"), SlotIndex);
            return;
        }

        RemoteEquippingWeapon->InitializeFromData(RemoteEquippingWeaponDataAsset);
        RemoteSpawnedWeapons.Add(RemoteEquippingWeaponItemInstanceId, RemoteEquippingWeapon);
    }

    // 새로운 무기 부착
    if ( USkeletalMeshComponent* CharacterMesh = Character->GetMesh() )
    {
        const FName SocketName = RemoteEquippingWeaponDataAsset ? RemoteEquippingWeaponDataAsset->AttachSocketName : NAME_None;

        RemoteEquippingWeapon->SetActorHiddenInGame(false);
        RemoteEquippingWeapon->AttachToComponent(CharacterMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, SocketName);
    }

    // Setup Remote Current Weapon Info
    RemoteCurrentWeaponSlotIndex = RemoteEquippingWeaponSlotIndex;
    RemoteCurrentWeaponItemInstance = RemoteEquippingWeaponItemInstance;
    RemoteCurrentWeaponItemInstanceId = RemoteEquippingWeaponItemInstanceId;
    RemoteCurrentWeapon = RemoteEquippingWeapon;

    if ( APX_Character* PX_Character = Cast<APX_Character>(Character) )
    {
        PX_Character->SetHasEquippedWeapon(RemoteCurrentWeaponSlotIndex != 4);
    }

    /*
    // Setup Predicted Equipping Weapon Info
    PredictedEquippingWeaponSlotIndex = INDEX_NONE;
    PredictedEquippingWeaponItemInstance = nullptr;
    PredictedEquippingWeaponItemInstanceId = FGuid();
    PredictedEquippingWeapon = nullptr;
    */

    // Remote에서 몽타주 재생
    // Equip 몽타주 데이터 가져오기
    UAnimMontage* EquipMontage = RemoteEquippingWeaponDataAsset->EquipAction.CharacterMontage;
    if ( !EquipMontage ) return;

    Local_PlayCharacterMontage(RemoteEquippingWeaponDataAsset->CharacterAnimClass, EquipMontage, RemoteEquippingWeaponDataAsset->EquipAction.PlayRate);
}

void UPX_WeaponSystemComponent::Multicast_EquipBySlotEnd_Implementation()
{
    if ( !Character || (Character->HasAuthority() && Character->IsPlayerControlled()) || Character->IsLocallyControlled() )
    {
        //PX_LOG(Warning, TEXT("Invalid Character State. Character: %s, HasAuthority: %d, IsLocallyControlled: %d"), *GetNameSafe(Character), Character ? Character->HasAuthority() : -1, Character ? Character->IsLocallyControlled() : -1);
        return;
    }

    PX_LOG(Log, TEXT("RemoteCurrentWeaponSlotIndex = %d"), RemoteCurrentWeaponSlotIndex);

    /*
    if ( !RemoteCurrentWeapon )
    {
        PX_LOG(Warning, TEXT("No RemoteCurrentWeapon"));
        return;
    }

    // 이미 authoritative weapon으로 바뀐 경우 return
    if ( RemoteCurrentWeapon == CurrentWeapon )
    {
        // Predicted Current Weapon 숨김
        PX_LOG(Log, TEXT("Remote weapon already authoritative. Hide Remote weapon: %s"), *RemoteCurrentWeapon->GetName());
        RemoteCurrentWeapon->SetActorHiddenInGame(true);
        RemoteCurrentWeapon->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
        //PredictedCurrentWeapon->Destroy();
        RemoteCurrentWeapon = CurrentWeapon;
        return;
    }

    // Remote Current Weapon 유지
    PX_LOG(Log, TEXT("Remote weapon: %s is not yet authoritative"), *RemoteCurrentWeapon->GetName());
    */

    return;
}

void UPX_WeaponSystemComponent::Multicast_CancelEquipBySlot_Implementation()
{
    if ( !Character || (Character->HasAuthority() && Character->IsPlayerControlled()) || Character->IsLocallyControlled() )
    {
        //PX_LOG(Warning, TEXT("Invalid Character State. Character: %s, HasAuthority: %d, IsLocallyControlled: %d"), *GetNameSafe(Character), Character ? Character->HasAuthority() : -1, Character ? Character->IsLocallyControlled() : -1);
        return;
    }

    PX_LOG(Log, TEXT(""));

    /*
    if ( RemoteEquippingWeaponSlotIndex != INDEX_NONE )
    {
        Local_StopMontage(RemoteEquippingWeaponSlotIndex);
    }

    // 장착 중인 무기 숨김
    if ( RemoteEquippingWeapon )
    {
        RemoteEquippingWeapon->SetActorHiddenInGame(true);
        RemoteEquippingWeapon->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
    }

    // Setup Remote Equipping Weapon Info
    RemoteEquippingWeaponSlotIndex = INDEX_NONE;
    RemoteEquippingWeaponItemInstance = nullptr;
    RemoteEquippingWeaponItemInstanceId = FGuid();
    RemoteEquippingWeapon = nullptr;
    */

    if ( !RemoteCurrentWeaponItemInstance && Inventory && RemoteCurrentWeaponSlotIndex != INDEX_NONE )
    {
        RemoteCurrentWeaponItemInstance = Cast<UPX_WeaponItemInstance>(Inventory->GetWeaponItemInstanceBySlot(RemoteCurrentWeaponSlotIndex));
        RemoteCurrentWeaponItemInstanceId = RemoteCurrentWeaponItemInstance ? RemoteCurrentWeaponItemInstance->GetInstanceId() : FGuid();
    }

    if ( !RemoteCurrentWeaponItemInstance )
    {
        PX_LOG(Warning, TEXT("No WeaponItemInstance found in RemoteCurrentWeaponItemInstance: %d"), RemoteCurrentWeaponSlotIndex);
        return;
    }

    UPX_WeaponDataAsset* WeaponDataAsset = RemoteCurrentWeaponItemInstance->GetWeaponDataAsset();
    if ( !WeaponDataAsset )
    {
        PX_LOG(Warning, TEXT("Invalid Weapon Data"));
        return;
    }

    UAnimMontage* EquipMontage = WeaponDataAsset->EquipAction.CharacterMontage;
    if ( !EquipMontage )
    {
        PX_LOG(Warning, TEXT("Invalid Equip Montage"));
        return;
    }

    if ( EquipMontage )
    {
        Local_StopCharacterMontage(EquipMontage);
    }

    if ( !RemoteCurrentWeapon )
    {
        PX_LOG(Warning, TEXT("No RemoteCurrentWeapon to restore"));
        return;
    }

    // 기존 무기 부착
    if ( USkeletalMeshComponent* CharacterMesh = Character->GetMesh() )
    {
        //const UPX_WeaponDataAsset* WeaponDataAsset = RemoteCurrentWeaponItemInstance->GetWeaponDataAsset();
        const FName SocketName = WeaponDataAsset ? WeaponDataAsset->AttachSocketName : NAME_None;

        RemoteCurrentWeapon->SetActorHiddenInGame(false);
        RemoteCurrentWeapon->AttachToComponent(CharacterMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, SocketName);
    }
}

void UPX_WeaponSystemComponent::Multicast_Reload_Implementation()
{
    if ( !Character || (Character->HasAuthority() && Character->IsPlayerControlled()) || Character->IsLocallyControlled() )
    {
        //PX_LOG(Warning, TEXT("Invalid Character State. Character: %s, HasAuthority: %d, IsLocallyControlled: %d"), *GetNameSafe(Character), Character ? Character->HasAuthority() : -1, Character ? Character->IsLocallyControlled() : -1);
        return;
    }

    PX_LOG(Log, TEXT(""));

    // Reload 몽타주 데이터 가져오기
    if ( !RemoteCurrentWeaponItemInstance )
    {
        PX_LOG(Warning, TEXT("Invalid RemoteCurrentWeaponItemInstance"));
        return;
    }

    UPX_WeaponDataAsset* WeaponDataAsset = RemoteCurrentWeaponItemInstance->GetWeaponDataAsset();
    if ( !WeaponDataAsset )
    {
        PX_LOG(Warning, TEXT("Invalid WeaponDataAsset"));
        return;
    }

    UAnimMontage* ReloadMontage = WeaponDataAsset->ReloadAction.CharacterMontage;
    UAnimMontage* WeaponReloadMontage = nullptr;
    if ( const APX_Character* PXCharacter = Cast<APX_Character>(Character) )
    {
        if ( const UPX_DemoBotComponent* DemoBotComponent = PXCharacter->FindComponentByClass<UPX_DemoBotComponent>() )
        {
            if ( UAnimMontage* BotCharacterReloadMontage = DemoBotComponent->GetBotCharacterReloadMontage() )
            {
                ReloadMontage = BotCharacterReloadMontage;
            }

            WeaponReloadMontage = DemoBotComponent->GetBotWeaponReloadMontage();
        }
    }

    if ( !ReloadMontage )
    {
        PX_LOG(Warning, TEXT("Invalid ReloadMontage"));
        return;
    }

    Local_PlayCharacterMontage(WeaponDataAsset->CharacterAnimClass, ReloadMontage, WeaponDataAsset->ReloadAction.PlayRate);
    if ( WeaponReloadMontage )
    {
        Local_PlayWeaponMontage(WeaponDataAsset->WeaponAnimClass, WeaponReloadMontage, WeaponDataAsset->ReloadAction.PlayRate);
    }
}

void UPX_WeaponSystemComponent::Multicast_ReloadEnd_Implementation()
{
    if ( !Character || (Character->HasAuthority() && Character->IsPlayerControlled()) || Character->IsLocallyControlled() )
    {
        //PX_LOG(Warning, TEXT("Invalid Character State. Character: %s, HasAuthority: %d, IsLocallyControlled: %d"), *GetNameSafe(Character), Character ? Character->HasAuthority() : -1, Character ? Character->IsLocallyControlled() : -1);
        return;
    }

    PX_LOG(Log, TEXT(""));
}

void UPX_WeaponSystemComponent::Multicast_CancelReload_Implementation()
{
    if ( !Character || (Character->HasAuthority() && Character->IsPlayerControlled()) || Character->IsLocallyControlled() )
    {
        //PX_LOG(Warning, TEXT("Invalid Character State. Character: %s, HasAuthority: %d, IsLocallyControlled: %d"), *GetNameSafe(Character), Character ? Character->HasAuthority() : -1, Character ? Character->IsLocallyControlled() : -1);
        return;
    }

    // Reload 몽타주 데이터 가져오기
    if ( !RemoteCurrentWeaponItemInstance )
    {
        PX_LOG(Warning, TEXT("Invalid RemoteCurrentWeaponItemInstance"));
        return;
    }

    UPX_WeaponDataAsset* WeaponDataAsset = RemoteCurrentWeaponItemInstance->GetWeaponDataAsset();
    if ( !WeaponDataAsset )
    {
        PX_LOG(Warning, TEXT("Invalid WeaponDataAsset"));
        return;
    }

    UAnimMontage* ReloadMontage = WeaponDataAsset->ReloadAction.CharacterMontage;
    UAnimMontage* WeaponReloadMontage = nullptr;
    if ( const APX_Character* PXCharacter = Cast<APX_Character>(Character) )
    {
        if ( const UPX_DemoBotComponent* DemoBotComponent = PXCharacter->FindComponentByClass<UPX_DemoBotComponent>() )
        {
            if ( UAnimMontage* BotCharacterReloadMontage = DemoBotComponent->GetBotCharacterReloadMontage() )
            {
                ReloadMontage = BotCharacterReloadMontage;
            }

            WeaponReloadMontage = DemoBotComponent->GetBotWeaponReloadMontage();
        }
    }

    if ( !ReloadMontage )
    {
        PX_LOG(Warning, TEXT("Invalid ReloadMontage"));
        return;
    }

    Local_StopCharacterMontage(ReloadMontage);
    if ( WeaponReloadMontage )
    {
        Local_StopWeaponMontage(WeaponReloadMontage);
    }
}

void UPX_WeaponSystemComponent::Multicast_FireOnce_Implementation()
{
    if ( !Character || (Character->HasAuthority() && Character->IsPlayerControlled()) || Character->IsLocallyControlled() )
    {
        //PX_LOG(Warning, TEXT("Invalid Character State. Character: %s, HasAuthority: %d, IsLocallyControlled: %d"), *GetNameSafe(Character), Character ? Character->HasAuthority() : -1, Character ? Character->IsLocallyControlled() : -1);
        return;
    }

    PX_LOG(Log, TEXT(""));

    // Fire 몽타주 데이터 가져오기
    if ( !CurrentWeaponItemInstance )
    {
        PX_LOG(Warning, TEXT("Invalid CurrentWeaponItemInstance"));
        return;
    }

    UPX_WeaponDataAsset* WeaponDataAsset = CurrentWeaponItemInstance->GetWeaponDataAsset();
    if ( !WeaponDataAsset )
    {
        PX_LOG(Warning, TEXT("Invalid WeaponDataAsset"));
        return;
    }

    bool bDryFire = CurrentWeaponItemInstance->GetAmmo() <= 0;
    UAnimMontage* CharacterFireMontage = bDryFire ? WeaponDataAsset->DryFireAction.CharacterMontage : WeaponDataAsset->FireAction.CharacterMontage;
    if ( !CharacterFireMontage )
    {
        PX_LOG(Warning, TEXT("Invalid CharacterFireMontage"));
        return;
    }

    
    UAnimMontage* WeaponFireMontage = bDryFire ? WeaponDataAsset->DryFireAction.WeaponMontage : WeaponDataAsset->FireAction.WeaponMontage;
    if ( !bDryFire )
    {
        if ( const APX_Character* PXCharacter = Cast<APX_Character>(Character) )
        {
            if ( const UPX_DemoBotComponent* DemoBotComponent = PXCharacter->FindComponentByClass<UPX_DemoBotComponent>() )
            {
                if ( UAnimMontage* BotWeaponFireMontage = DemoBotComponent->GetBotWeaponFireMontage() )
                {
                    WeaponFireMontage = BotWeaponFireMontage;
                }
            }
        }
    }
    /*
    if ( !WeaponFireMontage )
    {
        PX_LOG(Warning, TEXT("Invalid WeaponFireMontage"));
        return;
    }
    */

    Local_PlayCharacterMontage(WeaponDataAsset->CharacterAnimClass, CharacterFireMontage, WeaponDataAsset->FireAction.PlayRate);
    Local_PlayWeaponMontage(WeaponDataAsset->WeaponAnimClass, WeaponFireMontage, WeaponDataAsset->FireAction.PlayRate);
}

void UPX_WeaponSystemComponent::Multicast_FireOnceEnd_Implementation()
{
    if ( !Character || (Character->HasAuthority() && Character->IsPlayerControlled()) || Character->IsLocallyControlled() )
    {
        //PX_LOG(Warning, TEXT("Invalid Character State. Character: %s, HasAuthority: %d, IsLocallyControlled: %d"), *GetNameSafe(Character), Character ? Character->HasAuthority() : -1, Character ? Character->IsLocallyControlled() : -1);
        return;
    }

    PX_LOG(Log, TEXT(""));
}

void UPX_WeaponSystemComponent::Multicast_CancelFireOnce_Implementation()
{
    if ( !Character || (Character->HasAuthority() && Character->IsPlayerControlled()) || Character->IsLocallyControlled() )
    {
        //PX_LOG(Warning, TEXT("Invalid Character State. Character: %s, HasAuthority: %d, IsLocallyControlled: %d"), *GetNameSafe(Character), Character ? Character->HasAuthority() : -1, Character ? Character->IsLocallyControlled() : -1);
        return;
    }

    // Fire 몽타주 데이터 가져오기
    if ( !RemoteCurrentWeaponItemInstance )
    {
        PX_LOG(Warning, TEXT("Invalid RemoteCurrentWeaponItemInstance"));
        return;
    }

    UPX_WeaponDataAsset* WeaponDataAsset = RemoteCurrentWeaponItemInstance->GetWeaponDataAsset();
    if ( !WeaponDataAsset )
    {
        PX_LOG(Warning, TEXT("Invalid WeaponDataAsset"));
        return;
    }

    UAnimMontage* CharacterFireMontage = WeaponDataAsset->FireAction.CharacterMontage;
    if ( !CharacterFireMontage )
    {
        PX_LOG(Warning, TEXT("Invalid CharacterFireMontage"));
        return;
    }

    bool bDryFire = RemoteCurrentWeaponItemInstance->GetAmmo() <= 0;
    UAnimMontage* WeaponFireMontage = bDryFire ? WeaponDataAsset->DryFireAction.WeaponMontage : WeaponDataAsset->FireAction.WeaponMontage;
    if ( !WeaponFireMontage )
    {
        PX_LOG(Warning, TEXT("Invalid WeaponFireMontage"));
        return;
    }

    PX_LOG(Log, TEXT(""));

    Local_StopCharacterMontage(CharacterFireMontage);
    Local_StopWeaponMontage(WeaponFireMontage);
}

void UPX_WeaponSystemComponent::Multicast_SwitchAttackMode_Implementation()
{
    if ( !Character || (Character->HasAuthority() && Character->IsPlayerControlled()) || Character->IsLocallyControlled() )
    {
        //PX_LOG(Warning, TEXT("Invalid Character State. Character: %s, HasAuthority: %d, IsLocallyControlled: %d"), *GetNameSafe(Character), Character ? Character->HasAuthority() : -1, Character ? Character->IsLocallyControlled() : -1);
        return;
    }

    PX_LOG(Log, TEXT(""));

}

void UPX_WeaponSystemComponent::Multicast_SwitchAttackModeEnd_Implementation()
{
    if ( !Character || (Character->HasAuthority() && Character->IsPlayerControlled()) || Character->IsLocallyControlled() )
    {
        //PX_LOG(Warning, TEXT("Invalid Character State. Character: %s, HasAuthority: %d, IsLocallyControlled: %d"), *GetNameSafe(Character), Character ? Character->HasAuthority() : -1, Character ? Character->IsLocallyControlled() : -1);
        return;
    }

    PX_LOG(Log, TEXT(""));
}

void UPX_WeaponSystemComponent::Multicast_CancelSwitchAttackMode_Implementation()
{
    if ( !Character || (Character->HasAuthority() && Character->IsPlayerControlled()) || Character->IsLocallyControlled() )
    {
        //PX_LOG(Warning, TEXT("Invalid Character State. Character: %s, HasAuthority: %d, IsLocallyControlled: %d"), *GetNameSafe(Character), Character ? Character->HasAuthority() : -1, Character ? Character->IsLocallyControlled() : -1);
        return;
    }

    // Fire 몽타주 데이터 가져오기
    if ( !RemoteCurrentWeaponItemInstance )
    {
        PX_LOG(Warning, TEXT("Invalid RemoteCurrentWeaponItemInstance"));
        return;
    }

    UPX_WeaponDataAsset* WeaponDataAsset = RemoteCurrentWeaponItemInstance->GetWeaponDataAsset();
    if ( !WeaponDataAsset )
    {
        PX_LOG(Warning, TEXT("Invalid WeaponDataAsset"));
        return;
    }

    UAnimMontage* CharacterFireMontage = WeaponDataAsset->FireAction.CharacterMontage;
    if ( !CharacterFireMontage )
    {
        PX_LOG(Warning, TEXT("Invalid CharacterFireMontage"));
        return;
    }

    bool bDryFire = RemoteCurrentWeaponItemInstance->GetAmmo() <= 0;
    UAnimMontage* WeaponFireMontage = bDryFire ? WeaponDataAsset->DryFireAction.WeaponMontage : WeaponDataAsset->FireAction.WeaponMontage;
    if ( !WeaponFireMontage )
    {
        PX_LOG(Warning, TEXT("Invalid WeaponFireMontage"));
        return;
    }

    PX_LOG(Log, TEXT(""));
}

void UPX_WeaponSystemComponent::Local_PlayCharacterMontage(TSubclassOf<UAnimInstance> InAnimClass, UAnimMontage* InMontage, float InPlayRate)
{
    if ( Character && Character->HasAuthority() && Character->IsPlayerControlled() ) return;

    if ( !InMontage )
    {
        PX_LOG(Warning, TEXT("Invalid Montage"));
        return;
    }

    PX_LOG(Log, TEXT("Local_PlayMontage: %s"), *InMontage->GetName());

    APX_Character* PX_Character = Cast<APX_Character>(Character);
    if ( !PX_Character )
    {
        PX_LOG(Warning, TEXT("Invalid PX_Character"));
        return;
    }

    USkeletalMeshComponent* MeshComp = Character->GetMesh();
    if ( !MeshComp )
    {
        PX_LOG(Warning, TEXT("Invalid Mesh Component"));
        return;
    }

    UAnimInstance* AnimInstance = MeshComp->GetAnimInstance();
    if ( !AnimInstance )
    {
        PX_LOG(Warning, TEXT("Invalid Anim Instance"));
        return;
    }

    if ( InAnimClass )
    {
        PX_Character->SetLayerAnimInstanceByClass(InAnimClass);
    }

    if ( InMontage )
    {
        AnimInstance->Montage_Play(InMontage, InPlayRate);
    }

}

void UPX_WeaponSystemComponent::Local_StopCharacterMontage(UAnimMontage* InMontage)
{
    if ( Character && Character->HasAuthority() && Character->IsPlayerControlled() ) return;

    if ( !InMontage )
    {
        PX_LOG(Warning, TEXT("Invalid Montage"));
        return;
    }

    USkeletalMeshComponent* MeshComp = Character->GetMesh();
    if ( !MeshComp ) return;

    UAnimInstance* AnimInstance = MeshComp->GetAnimInstance();
    if ( !AnimInstance ) return;

    PX_LOG(Log, TEXT("Local_StopMontage: %s"), *InMontage->GetName());

    AnimInstance->Montage_Stop(0.25f, InMontage);
}

void UPX_WeaponSystemComponent::Local_PlayWeaponMontage(TSubclassOf<UAnimInstance> InAnimClass, UAnimMontage* InMontage, float InPlayRate)
{
    if ( Character && Character->HasAuthority() && Character->IsPlayerControlled() ) return;

    if ( !InMontage )
    {
        PX_LOG(Log, TEXT("Invalid Montage.. Skip Play Weapon Montage"));
        return;
    }

    PX_LOG(Log, TEXT("Local_PlayWeaponMontage: %s"), *InMontage->GetName());

    if ( !CurrentWeapon )
    {
        PX_LOG(Warning, TEXT("Invalid CurrentWeapon"));
        return;
    }

    USkeletalMeshComponent* MeshComp = CurrentWeapon->GetMesh();
    if ( !MeshComp )
    {
        PX_LOG(Warning, TEXT("Invalid Mesh Component"));
        return;
    }

    UAnimInstance* AnimInstance = MeshComp->GetAnimInstance();
    if ( !AnimInstance )
    {
        PX_LOG(Warning, TEXT("Invalid Anim Instance"));
        return;
    }

    if ( InMontage )
    {
        AnimInstance->Montage_Play(InMontage, InPlayRate);
    }

}

void UPX_WeaponSystemComponent::Local_StopWeaponMontage(UAnimMontage* InMontage)
{
    if ( Character && Character->HasAuthority() && Character->IsPlayerControlled() ) return;

    if ( !InMontage )
    {
        PX_LOG(Warning, TEXT("Invalid Montage"));
        return;
    }

    if ( !CurrentWeapon ) return;

    USkeletalMeshComponent* MeshComp = CurrentWeapon->GetMesh();
    if ( !MeshComp ) return;

    UAnimInstance* AnimInstance = MeshComp->GetAnimInstance();
    if ( !AnimInstance ) return;

    PX_LOG(Log, TEXT("Local_StopWeaponMontage: %s"), *InMontage->GetName());

    AnimInstance->Montage_Stop(0.25f, InMontage);
}

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

    if ( Character->HasAuthority() )
    {
        Authoritative_EquipBySlot(4);
    }
    else if ( Character->IsLocallyControlled() )
    {
        ServerEquipBySlot(4);
    }
}

// PX_WeaponItemInstance의 OnAmmoUpdated 델리게이트 Handler
void UPX_WeaponSystemComponent::HandleAmmoUpdated(int32 InAmmoInMag, int32 InReserved)
{
    if ( CurrentWeaponSlotIndex == 4 ) return;  // BareHand
    OnCurrentAmmoUpdated.Broadcast(InAmmoInMag, InReserved);
}

// PX_WeaponItemInstance의 OnAttackModeUpdated 델리게이트 Handler
void UPX_WeaponSystemComponent::HandleAttackModeUpdated(FGameplayTag InAttackModeTag)
{
    const int32 EffectiveWeaponSlotIndex = (PredictedCurrentWeaponSlotIndex != INDEX_NONE) ? PredictedCurrentWeaponSlotIndex : CurrentWeaponSlotIndex;
    if ( EffectiveWeaponSlotIndex == 4 ) return;  // BareHand

    //PX_LOG(Log, TEXT("AttackMode: %s"), WeaponAttackModeToString(InAttackMode));
    OnAttackModeTagUpdated.Broadcast(InAttackModeTag);
}

void UPX_WeaponSystemComponent::OnRep_WeaponSlotIndexUpdated()
{
    //PX_LOG(Log, TEXT(""));
}

void UPX_WeaponSystemComponent::OnRep_WeaponItemInstanceIdUpdated()
{
    //PX_LOG(Log, TEXT(""));
}

void UPX_WeaponSystemComponent::OnRep_WeaponUpdated()
{
    //PX_LOG(Log, TEXT(""));

    // 아직 Predcited Current Weapon이 Authoritative Weapon으로 바뀌지 않은 경우 Predicted Current Weapon 숨김
    if ( IsValid(PredictedCurrentWeapon) && PredictedCurrentWeapon != CurrentWeapon )
    {
        // Predicted Current Weapon 숨김
        PX_LOG(Log, TEXT("Predicted weapon is not yet authoritative. Hide Predicted weapon"));
        PredictedCurrentWeapon->SetActorHiddenInGame(true);
        PredictedCurrentWeapon->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
        //PredictedCurrentWeapon->Destroy();
    }
    if ( IsValid(RemoteCurrentWeapon) && RemoteCurrentWeapon != CurrentWeapon )
    {
        // Predicted Current Weapon 숨김
        PX_LOG(Log, TEXT("Remote weapon is not yet authoritative. Hide Remote weapon"));
        RemoteCurrentWeapon->SetActorHiddenInGame(true);
        RemoteCurrentWeapon->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
        //PredictedCurrentWeapon->Destroy();
    }

    // Setup Predicted Current Weapon Info
    PredictedCurrentWeaponSlotIndex = CurrentWeaponSlotIndex;
    PredictedCurrentWeaponItemInstanceId = CurrentWeaponItemInstanceId;
    PredictedCurrentWeapon = CurrentWeapon;

    // Setup Remote Current Weapon Info
    RemoteCurrentWeaponSlotIndex = CurrentWeaponSlotIndex;
    RemoteCurrentWeaponItemInstanceId = CurrentWeaponItemInstanceId;
    RemoteCurrentWeapon = CurrentWeapon;

    /*
    // Setup Predicted Equipping Weapon Info
    PredictedEquippingWeaponSlotIndex = INDEX_NONE;
    PredictedEquippingWeaponItemInstance = nullptr;
    PredictedEquippingWeaponItemInstanceId = FGuid();
    PredictedEquippingWeapon = nullptr;

    // Setup Remote Equipping Weapon Info
    RemoteEquippingWeaponSlotIndex = INDEX_NONE;
    RemoteEquippingWeaponItemInstance = nullptr;
    RemoteEquippingWeaponItemInstanceId = FGuid();
    RemoteEquippingWeapon = nullptr;
    */

    if ( !Inventory )
    {
        PX_LOG(Warning, TEXT("Inventory is null while resolving replicated weapon. SlotIndex: %d, Weapon: %s"), CurrentWeaponSlotIndex, *GetNameSafe(CurrentWeapon));
        return;
    }

    CurrentWeaponItemInstance = Cast<UPX_WeaponItemInstance>(Inventory->GetWeaponItemInstanceBySlot(CurrentWeaponSlotIndex));
    if ( APX_Character* PX_Character = Cast<APX_Character>(Character) )
    {
        PX_Character->SetHasEquippedWeapon(CurrentWeaponSlotIndex != 4);
    }
    if ( !CurrentWeaponItemInstance ) return;

    PredictedCurrentWeaponItemInstance = CurrentWeaponItemInstance;
    RemoteCurrentWeaponItemInstance = CurrentWeaponItemInstance;
    SyncPredictedAmmoFromCurrent();
    SyncPredictedAttackModeFromCurrent();

    if ( !IsValid(CurrentWeapon) )
    {
        PX_LOG(Warning, TEXT("CurrentWeapon is not resolved yet. SlotIndex: %d, ItemInstanceId: %s"), CurrentWeaponSlotIndex, *CurrentWeaponItemInstanceId.ToString());
        return;
    }

    // 무기 부착
    UPX_WeaponDataAsset* WeaponDataAsset = CurrentWeaponItemInstance->GetWeaponDataAsset();
    if ( !WeaponDataAsset )
    {
        PX_LOG(Warning, TEXT("No WeaponDataAsset found for CurrentWeaponItemInstance in SlotIndex: %d"), CurrentWeaponSlotIndex);
        return ;
    }

    if ( !Character )
    {
        PX_LOG(Warning, TEXT("Character is null while attaching replicated weapon: %s"), *GetNameSafe(CurrentWeapon));
        return;
    }

    if ( USkeletalMeshComponent* CharacterMesh = Character->GetMesh() )
    {
        const FName SocketName = WeaponDataAsset ? WeaponDataAsset->AttachSocketName : NAME_None;

        PX_LOG(Log, TEXT("Attach Server Equipping Weapon: %s to Character Mesh at Socket: %s"), *GetNameSafe(CurrentWeapon), *SocketName.ToString());
        CurrentWeapon->SetActorHiddenInGame(false);
        CurrentWeapon->AttachToComponent(CharacterMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, SocketName);

        // 애님 인스턴스 클래스 설정
        UAnimInstance* AnimInstance = CharacterMesh->GetAnimInstance();
        if ( !AnimInstance ) return;

        APX_Character* PX_Character = Cast<APX_Character>(Character);
        if ( !PX_Character ) return;

        PX_Character->SetLayerAnimInstanceByClass(WeaponDataAsset->CharacterAnimClass);
    }

    // Broadacst
    OnCurrentWeaponUpdated.Broadcast(CurrentWeaponSlotIndex);

    HandleAmmoUpdated(GetCurrentAmmoInMag(), 1000);
    HandleAttackModeUpdated(GetCurrentAttackModeTag());

    /*
    APX_Character* PX_Character = Cast<APX_Character>(Character);
    if ( !PX_Character ) return;
    PX_Character->SetLayerAnimInstanceByClass(CurrentWeaponItemInstance->GetWeaponDataAsset()->CharacterAnimClass);
    */

}
