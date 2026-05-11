// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/Inventory/PX_ItemInstance.h"
#include "Net/UnrealNetwork.h"
#include "Component/Inventory/PX_EquippableItemDataAsset.h"
#include "Component/Weapon/PX_WeaponDataAsset.h"

DEFINE_LOG_CATEGORY(PX_ItemInstance);

void UPX_ItemInstance::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UPX_ItemInstance, InstanceId);
	DOREPLIFETIME(UPX_ItemInstance, Quantity);
	DOREPLIFETIME(UPX_ItemInstance, ItemDataAsset);
}

/*
bool UPX_ItemInstance::HasServerAuthority() const
{
    // Outer Actor의 Authority로 판단
    const AActor* OuterActor = GetTypedOuter<AActor>();
    if ( OuterActor )
    {
        return OuterActor->HasAuthority();
    }

    // OuterActor를 못 찾는 경우 NetMode 기반으로 판단
    if ( const UWorld* World = GetWorld() )
    {
        return World->GetNetMode() != NM_Client;
    }

    return false;
}
*/

UPX_EquippableItemDataAsset* UPX_ItemInstance::GetEquippableItemDataAsset() const
{
    return Cast<UPX_EquippableItemDataAsset>(ItemDataAsset);
}

UPX_WeaponDataAsset* UPX_ItemInstance::GetWeaponDataAsset() const
{ 
    return Cast<UPX_WeaponDataAsset>(ItemDataAsset);
}