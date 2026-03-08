// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Project_X.h"
#include "Subsystems/WorldSubsystem.h"
#include "PX_WeaponCommitSubsystem.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(PX_WeaponCommitSubsystem, Log, All);

class UPX_WeaponSystemComponent;

/**
 * 
 */
UCLASS()
class PROJECT_X_API UPX_WeaponCommitSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override { RETURN_QUICK_DECLARE_CYCLE_STAT(UPX_WeaponCommitSubsystem, STATGROUP_Tickables); }
	
	void Register(UPX_WeaponSystemComponent* Comp);
	void Unregister(UPX_WeaponSystemComponent* Comp);
	
private:
	double AccumulatedDeltaTime = 0.0;
	static constexpr double Step = 1.0 / 60.0;
	int32 MaxStepsPerTick = 4; // 서버 과부화 방지

	TSet<TWeakObjectPtr<UPX_WeaponSystemComponent>> Registered;
	
};
