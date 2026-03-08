// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/Inventory/PX_ItemDataAsset.h"
#include "PX_ItemInstance.h"
#include "PX_WeaponItemInstance.h"

UPX_ItemInstance* UPX_ItemDataAsset::CreateItemInstance(UObject* Outer, int32 InQuantity, const FGuid& InInstanceId)
{
    if ( !InstanceClass )
    {
        UE_LOG(LogTemp, Warning, TEXT("InstanceClass is null in %s"), *GetName());
        return nullptr;
    }

    UPX_ItemInstance* NewInstance = NewObject<UPX_ItemInstance>(Outer, InstanceClass);
    
    if ( !NewInstance ) return nullptr;

    if ( InInstanceId.IsValid() )
    {
        NewInstance->InstanceId = InInstanceId;
    }
    else
    {
        NewInstance->InstanceId = FGuid::NewGuid();   // 서버에서만
        PX_LOG(Log, TEXT("New Item Instance is Created. Name : %s, Guid : %s"), *DisplayName.ToString(), *NewInstance->InstanceId.ToString());
    }
    
    NewInstance->InitializeFromData(this, InQuantity);

    return NewInstance;
}




