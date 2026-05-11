// Fill out your copyright notice in the Description page of Project Settings.


#include "PX_WeaponCommitSubsystem.h"
#include "Component/Weapon/PX_WeaponSystemComponent.h"

DEFINE_LOG_CATEGORY(PX_WeaponCommitSubsystem);

void UPX_WeaponCommitSubsystem::Register(UPX_WeaponSystemComponent* Component)
{
    if ( Component ) {
        Registered.Add(Component); PX_LOG(Log, TEXT(""));
    }

    
}

void UPX_WeaponCommitSubsystem::Unregister(UPX_WeaponSystemComponent* Component)
{
    if ( Component ) Registered.Remove(Component);
}

void UPX_WeaponCommitSubsystem::Tick(float DeltaTime)
{
    UWorld* World = GetWorld();
    if ( !World ) return;

    // Commit Only at Server
    if ( World->GetNetMode() == NM_Client ) return;

    AccumulatedDeltaTime += DeltaTime;

    int32 Steps = 0;
    while ( AccumulatedDeltaTime >= Step && MaxStepsPerTick > Steps )
    {
        AccumulatedDeltaTime -= Step;
        ++Steps;

        for ( auto It = Registered.CreateIterator(); It; ++It )
        {
            if ( !It->IsValid() )
            {
                It.RemoveCurrent();
                continue;
            }
            //It->Get()->ServerCommitAction((float)Step);
        }
    }
}


