// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/PX_GameplayAbility_Equip.h"

#include "AbilitySystemComponent.h"
#include "GameplayTagContainer.h"
#include "AbilitySystem/Tags/PX_GameplayTags.h"

#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"

#include "Entity/PX_Character.h"
#include "Component/Weapon/PX_WeaponDataAsset.h"
#include "Component/Weapon/PX_WeaponSystemComponent.h"
#include "Component/Inventory/PX_InventoryComponent.h"
#include "Component/Inventory/PX_WeaponItemInstance.h"

UPX_GameplayAbility_Equip::UPX_GameplayAbility_Equip()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
	//bRetriggerInstancedAbility = true;

	// 입력 태그
	InputTag = PX_GameplayTags::Input_Equip;

	// Ability 자체 태그
	AbilityTags.AddTag(PX_GameplayTags::Ability_Weapon_Equip);

	// Ability 활성 중 ASC에 부여될 태그
	ActivationOwnedTags.AddTag(PX_GameplayTags::State_Combat_Equipping);

	// 이 태그가 있으면 Ability 활성화 불가
	//ActivationBlockedTags.AddTag(PX_GameplayTags::State_Combat_Equipping);
	//ActivationBlockedTags.AddTag(PX_GameplayTags::State_Combat_Unequipping);
	//ActivationBlockedTags.AddTag(PX_GameplayTags::State_Combat_SwappingWeapon);

	ActivationBlockedTags.AddTag(PX_GameplayTags::State_Condition_Dead);
	ActivationBlockedTags.AddTag(PX_GameplayTags::State_Condition_Down);
	ActivationBlockedTags.AddTag(PX_GameplayTags::State_Condition_Disabled);
	ActivationBlockedTags.AddTag(PX_GameplayTags::State_Condition_Stunned);
	ActivationBlockedTags.AddTag(PX_GameplayTags::State_Condition_Silenced);

	// Trigger Tag 설정
	FAbilityTriggerData TriggerData;
	TriggerData.TriggerTag = PX_GameplayTags::Event_Weapon_Equip_Begin;
	TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	AbilityTriggers.Add(TriggerData);

	// PX_GameplayTags::Event_Weapon_Equip_Change는 등록하지 않음
	// Equip Ability를 새로 생성하는 것이 아니라 이미 생성되어 있는 Equip Ability에 전달해 내부적으로 처리하기 때문
}

bool UPX_GameplayAbility_Equip::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if ( !Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags) ) return false;

	const UPX_WeaponSystemComponent* WSC = GetWeaponSystemComponent(ActorInfo);
	if ( !WSC ) { PX_LOG(Log, TEXT("false")); return false; }

	APX_Character* Character = GetPXCharacter(ActorInfo);
	if ( !Character ) { PX_LOG(Log, TEXT("false")); return false; }

	const UPX_InventoryComponent* Inventory = Character->GetInventoryComponent();
	if ( !Inventory || !Inventory->IsInventoryReady() ) { PX_LOG(Log, TEXT("false")); return false; }

	PX_LOG(Log, TEXT("true"));

	return true;
}

void UPX_GameplayAbility_Equip::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	PX_LOG(Log, TEXT("RequestedSlot: %d"), FMath::RoundToInt(TriggerEventData->EventMagnitude));

	if ( !ActorInfo || !ActorInfo->AvatarActor.IsValid() )
	{
		PX_LOG(Warning, TEXT("Invalid ActorInfo"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// Slot Index를 TriggerEventData에서 가져오기
	EquipSlotIndex = INDEX_NONE;
	if ( TriggerEventData ) EquipSlotIndex = FMath::RoundToInt(TriggerEventData->EventMagnitude);

	if ( EquipSlotIndex == INDEX_NONE )
	{
		PX_LOG(Warning, TEXT("Invalid Slot Index"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// Equip 몽타주 데이터 가져오기
	APX_Character* Character = GetPXCharacter(ActorInfo);
	if ( !Character )
	{
		PX_LOG(Warning, TEXT("Invalid Character"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	UPX_InventoryComponent* Inventory = Character->GetInventoryComponent();
	if ( !Inventory || !Inventory->IsInventoryReady() )
	{
		PX_LOG(Warning, TEXT("Invalid Inventory"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	UPX_WeaponItemInstance* WeaponItemInstance = Cast<UPX_WeaponItemInstance>(Inventory->GetWeaponItemInstanceBySlot(EquipSlotIndex));
	if ( !WeaponItemInstance )
	{
		PX_LOG(Warning, TEXT("Invalid ItemInstance at Slot[%d]"), EquipSlotIndex);
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	UPX_WeaponDataAsset* WeaponDataAsset = WeaponItemInstance->GetWeaponDataAsset();
	if ( !WeaponDataAsset )
	{
		PX_LOG(Warning, TEXT("Invalid Weapon Data"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}


	UAnimMontage* EquipMontage = WeaponDataAsset->EquipAction.CharacterMontage;
	if ( !EquipMontage )
	{
		PX_LOG(Warning, TEXT("Invalid EquipMontage"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	/*
	USkeletalMeshComponent* MeshComp = Character->GetMesh();
	if ( !MeshComp )
	{
		PX_LOG(Warning, TEXT("Invalid Character Mesh"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	UAnimInstance* AnimInstance = MeshComp->GetAnimInstance();
	if ( !AnimInstance )
	{
		PX_LOG(Warning, TEXT("Invalid Character AnimInstance"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	*/

	if ( !HasAuthority(&ActivationInfo) && ActorInfo && ActorInfo->IsLocallyControlled() && !ShouldUseClientPrediction(ActorInfo, &ActivationInfo) )
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
		return;
	}

	// Commit Ability
	if ( !CommitAbility(Handle, ActorInfo, ActivationInfo) )
	{
		PX_LOG(Warning, TEXT("CommitAbility Failed"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// Equip 시도
	UPX_WeaponSystemComponent* WSC = GetWeaponSystemComponent(ActorInfo);
	if ( HasAuthority(&ActivationInfo) )
	{
		// Server Authoritative
		PX_LOG(Log, TEXT("Begin Server Equip"));

		if ( !WSC->Authoritative_EquipBySlot(EquipSlotIndex) )
		{
			PX_LOG(Warning, TEXT("Fail Server Equip"));
			EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
			return;
		}
		WSC->Multicast_EquipBySlot(EquipSlotIndex);

		const float PlayRate = FMath::Max(WeaponDataAsset->EquipAction.PlayRate, KINDA_SMALL_NUMBER);
		const float Duration = EquipMontage->GetPlayLength() / PlayRate;

		PX_LOG(Log, TEXT("Server Equip Set Timer. Duration: %f"), Duration);

		UWorld* World = GetWorld();
		if ( !World )
		{
			PX_LOG(Warning, TEXT("Invalid World"));
			return;
		}
		World->GetTimerManager().SetTimer(ServerEquipTimerHandle, this, &UPX_GameplayAbility_Equip::Authoritative_OnEquipEnd, Duration, false);

		return;
	}

	if ( ShouldUseClientPrediction(ActorInfo, &CurrentActivationInfo) )
	{
		// Local Prediction
		PX_LOG(Log, TEXT("Begin Client Predicted Equip"));
		
		if ( !WSC->Predict_EquipBySlot(EquipSlotIndex) )
		{
			PX_LOG(Warning, TEXT("Fail Client Predicted Equip"));
			EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
			return;
		}
		WSC->Local_PlayCharacterMontage(WeaponDataAsset->CharacterAnimClass, EquipMontage, WeaponDataAsset->EquipAction.PlayRate);
	}
	else if ( HasAuthority(&ActivationInfo) )
	{
		// Server Authoritative
		PX_LOG(Log, TEXT("Begin Server Equip"));
		
		if ( !WSC->Authoritative_EquipBySlot(EquipSlotIndex) )
		{
			PX_LOG(Warning, TEXT("Fail Server Equip"));
			EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
			return;
		}
		WSC->Multicast_EquipBySlot(EquipSlotIndex);

		const float PlayRate = FMath::Max(WeaponDataAsset->EquipAction.PlayRate, KINDA_SMALL_NUMBER);
		const float Duration = EquipMontage->GetPlayLength() / PlayRate;

		PX_LOG(Log, TEXT("Server Equip Set Timer. Duration: %f"), Duration);

		UWorld* World = GetWorld();
		if ( !World )
		{
			PX_LOG(Warning, TEXT("Invalid World"));
			return;
		}
		World->GetTimerManager().SetTimer(ServerEquipTimerHandle, this, &UPX_GameplayAbility_Equip::Authoritative_OnEquipEnd, Duration, false);

		return;
	}

	if ( ShouldUseClientPrediction(ActorInfo, &CurrentActivationInfo) )
	{
		// Local Prediction에서 몽타주 Notify가 Event를 보낼 때까지 대기
		if ( PredictWaitTask )
		{
			PredictWaitTask->ExternalCancel();
			PredictWaitTask = nullptr;
		}

		PredictWaitTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, PX_GameplayTags::Event_Weapon_Equip_End, nullptr, false, false);
		if ( PredictWaitTask )
		{
			PredictWaitTask->EventReceived.AddDynamic(this, &UPX_GameplayAbility_Equip::Predict_OnEquipEnd);
			PredictWaitTask->ReadyForActivation();
		}
	}

	/*
	// Equip Montage를 Replicate 하기 위해 몽타주 재생을 Ability Task로 처리
	UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, EquipMontage, WeaponDataAsset->EquipAction.PlayRate, NAME_None, false, 1.0f);
	if ( !MontageTask )
	{
		PX_LOG(Warning, TEXT("Failed to Play Equip Montage"));
		WSC->CancelEquip();
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	MontageTask->OnCompleted.AddDynamic(this, &UPX_GameplayAbility_Equip::OnMontageCompleted);
	MontageTask->OnInterrupted.AddDynamic(this, &UPX_GameplayAbility_Equip::OnMontageInterrupted);
	MontageTask->OnCancelled.AddDynamic(this, &UPX_GameplayAbility_Equip::OnMontageCancelled);
	MontageTask->ReadyForActivation();
	*/
}

void UPX_GameplayAbility_Equip::ChangeEquip(int32 NewSlotIndex)
{
	PX_LOG(Log, TEXT("ChangeEquip: %d -> %d"), EquipSlotIndex, NewSlotIndex);

	const FGameplayAbilityActorInfo* ActorInfo = GetCurrentActorInfo();
	if ( !ActorInfo )
	{
		PX_LOG(Warning, TEXT("Invalid ActorInfo"));
		return;
	}

	if ( !bEquipCancelable )
	{
		PX_LOG(Log, TEXT("ChangeEquip ignored: not cancelable"));
		return;
	}

	if ( NewSlotIndex == INDEX_NONE || NewSlotIndex == EquipSlotIndex )
	{
		PX_LOG(Log, TEXT("ChangeEquip ignored: invalid or same slot"));
		return;
	}

	UPX_WeaponSystemComponent* WSC = GetWeaponSystemComponent(ActorInfo);
	if ( !WSC )
	{
		PX_LOG(Warning, TEXT("Invalid WeaponSystemComponent"));
		return;
	}

	APX_Character* Character = GetPXCharacter(ActorInfo);
	if ( !Character )
	{
		PX_LOG(Warning, TEXT("Invalid Character"));
		return;
	}

	UPX_InventoryComponent* Inventory = Character->GetInventoryComponent();
	if ( !Inventory || !Inventory->IsInventoryReady() )
	{
		PX_LOG(Warning, TEXT("Invalid Inventory"));
		return;
	}

	UPX_WeaponItemInstance* WeaponItemInstance = Cast<UPX_WeaponItemInstance>(Inventory->GetWeaponItemInstanceBySlot(EquipSlotIndex));
	if ( !WeaponItemInstance )
	{
		PX_LOG(Warning, TEXT("Invalid ItemInstance at Slot[%d]"), EquipSlotIndex);
		return;
	}

	UPX_WeaponDataAsset* WeaponDataAsset = WeaponItemInstance->GetWeaponDataAsset();
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

	// 기존 진행 흐름 정리
	bEquipCancelable = false;
	if ( HasAuthority(&CurrentActivationInfo) )
	{
		PX_LOG(Log, TEXT("Server ChangeEquip"));

		if ( GetWorld() )
		{
			GetWorld()->GetTimerManager().ClearTimer(ServerEquipTimerHandle);
		}
	}
	else if ( ShouldUseClientPrediction(ActorInfo, &CurrentActivationInfo) )
	{
		PX_LOG(Log, TEXT("Client ChangeEquip"));
		if ( PredictWaitTask )
		{
			PredictWaitTask->ExternalCancel();
			PredictWaitTask = nullptr;
		}

		if ( EquipSlotIndex != INDEX_NONE )
		{
			WSC->Local_StopCharacterMontage(EquipMontage);
		}
	}
	else if ( HasAuthority(&CurrentActivationInfo) )
	{
		PX_LOG(Log, TEXT("Server ChangeEquip"));

		if ( GetWorld() )
		{
			GetWorld()->GetTimerManager().ClearTimer(ServerEquipTimerHandle);
		}
	}

	// Slot 변경
	EquipSlotIndex = NewSlotIndex;

	WeaponItemInstance = Cast<UPX_WeaponItemInstance>(Inventory->GetWeaponItemInstanceBySlot(EquipSlotIndex));
	if ( !WeaponItemInstance )
	{
		PX_LOG(Warning, TEXT("Invalid ItemInstance at Slot[%d]"), EquipSlotIndex);
		return;
	}

	WeaponDataAsset = WeaponItemInstance->GetWeaponDataAsset();
	if ( !WeaponDataAsset )
	{
		PX_LOG(Warning, TEXT("Invalid Weapon Data"));
		return;
	}

	EquipMontage = WeaponDataAsset->EquipAction.CharacterMontage;
	if ( !EquipMontage )
	{
		PX_LOG(Warning, TEXT("Invalid Equip Montage"));
		return;
	}

	// Equip 시도
	if ( ShouldUseClientPrediction(ActorInfo, &CurrentActivationInfo) )
	{
		// Local Prediction
		PX_LOG(Log, TEXT("Begin Client Predicted Equip"));

		if ( !WSC->Predict_EquipBySlot(EquipSlotIndex) )
		{
			PX_LOG(Warning, TEXT("Fail Client Predicted Equip"));
			return;
		}

		WSC->Local_PlayCharacterMontage(WeaponDataAsset->CharacterAnimClass, EquipMontage, WeaponDataAsset->EquipAction.PlayRate);
	}
	else if ( HasAuthority(&CurrentActivationInfo) )
	{
		// Server Authoritative
		PX_LOG(Log, TEXT("Begin Server Equip"));

		if ( !WSC->Authoritative_EquipBySlot(EquipSlotIndex) )
		{
			PX_LOG(Warning, TEXT("Fail Server Equip"));
			return;
		}
		WSC->Multicast_EquipBySlot(EquipSlotIndex);

		const float PlayRate = FMath::Max(WeaponDataAsset->EquipAction.PlayRate, KINDA_SMALL_NUMBER);
		const float Duration = EquipMontage->GetPlayLength() / PlayRate;

		PX_LOG(Log, TEXT("Server Equip Set Timer. Duration: %f"), Duration);

		UWorld* World = GetWorld();
		if ( !World )
		{
			PX_LOG(Warning, TEXT("Invalid World"));
			return;
		}
		World->GetTimerManager().SetTimer(ServerEquipTimerHandle, this, &UPX_GameplayAbility_Equip::Authoritative_OnEquipEnd, Duration, false);

		return;
	}

	if ( ShouldUseClientPrediction(ActorInfo, &CurrentActivationInfo) )
	{
		// WaitTask 재설정
		if ( PredictWaitTask )
		{
			PredictWaitTask->ExternalCancel();
			PredictWaitTask = nullptr;
		}

		PredictWaitTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, PX_GameplayTags::Event_Weapon_Equip_End, nullptr, false, false);
		if ( PredictWaitTask )
		{
			PredictWaitTask->EventReceived.AddDynamic(this, &UPX_GameplayAbility_Equip::Predict_OnEquipEnd);
			PredictWaitTask->ReadyForActivation();
		}
	}
}

void UPX_GameplayAbility_Equip::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	bEquipCancelable = false;

	if ( bWasCancelled ) CancelEquip(ActorInfo);

	PX_LOG(Log, TEXT(""));

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

/*
void UPX_GameplayAbility_Equip::Predict_OnEquipEnd()
{
	PX_LOG(Log, TEXT(""));

	const FGameplayAbilityActorInfo* ActorInfo = GetCurrentActorInfo();
	if ( !ActorInfo )
	{
		PX_LOG(Warning, TEXT("Invalid ActorInfo"));
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	UPX_WeaponSystemComponent* WSC = GetWeaponSystemComponent(ActorInfo);
	if ( !WSC )
	{
		PX_LOG(Warning, TEXT("Invalid Weapon System Component"));
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	if ( !ShouldUseClientPrediction(ActorInfo, &CurrentActivationInfo) )
	{
		PX_LOG(Warning, TEXT("Not Locally Controlled"));
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	if ( !WSC->Predict_EquipBySlotEnd(EquipSlotIndex) )
	{
		CancelEquip(ActorInfo);
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	//EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}
*/

void UPX_GameplayAbility_Equip::Predict_OnEquipEnd(FGameplayEventData Payload)
{
	PX_LOG(Log, TEXT(""));

	const FGameplayAbilityActorInfo* ActorInfo = GetCurrentActorInfo();
	if ( !ActorInfo )
	{
		PX_LOG(Warning, TEXT("Invalid ActorInfo"));
		return;
	}

	UPX_WeaponSystemComponent* WSC = GetWeaponSystemComponent(ActorInfo);
	if ( !WSC )
	{
		PX_LOG(Warning, TEXT("Invalid Weapon System Component"));
		return;
	}

	if ( !ShouldUseClientPrediction(CurrentActorInfo, &CurrentActivationInfo) ) return;

	if ( !WSC->Predict_EquipBySlotEnd() )
	{
		CancelEquip(ActorInfo);
		return;
	}

	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, false);
}

void UPX_GameplayAbility_Equip::Authoritative_OnEquipEnd()
{
	PX_LOG(Log, TEXT(""));

	const FGameplayAbilityActorInfo* ActorInfo = GetCurrentActorInfo();
	if ( !ActorInfo )
	{
		PX_LOG(Warning, TEXT("Invalid ActorInfo"));
		return;
	}

	UPX_WeaponSystemComponent* WSC = GetWeaponSystemComponent(ActorInfo);
	if ( !WSC )
	{
		PX_LOG(Warning, TEXT("Invalid Weapon System Component"));
		return;
	}

	if ( !HasAuthority(&CurrentActivationInfo) ) return;

	if ( !WSC->Authoritative_EquipBySlotEnd() )
	{
		CancelEquip(ActorInfo);
		return;
	}

	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UPX_GameplayAbility_Equip::CancelEquip(const FGameplayAbilityActorInfo* ActorInfo)
{
	PX_LOG(Log, TEXT(""));

	UPX_WeaponSystemComponent* WSC = GetWeaponSystemComponent(ActorInfo);
	if ( !WSC ) return;

	if ( ShouldUseClientPrediction(ActorInfo, &CurrentActivationInfo) )
	{
		// Local Prediction
		PX_LOG(Log, TEXT("Cancel Client Predicted Equip"));

		if ( PredictWaitTask )
		{
			PredictWaitTask->ExternalCancel();
		}

		WSC->Predict_CancelEquipBySlot();
	}
	else if ( HasAuthority(&CurrentActivationInfo) )
	{
		// Server Authoritative
		PX_LOG(Log, TEXT("Cancel Server Equip"));

		if ( GetWorld() )
		{
			GetWorld()->GetTimerManager().ClearTimer(ServerEquipTimerHandle);
		}

		WSC->Authoritative_CancelEquipBySlot();
	}

	//EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

/*
APX_Character* UPX_GameplayAbility_Equip::GetPXCharacter(const FGameplayAbilityActorInfo* ActorInfo) const
{
	if ( !ActorInfo || !ActorInfo->AvatarActor.IsValid() ) return nullptr;

	return Cast<APX_Character>(ActorInfo->AvatarActor.Get());
}

UPX_WeaponSystemComponent* UPX_GameplayAbility_Equip::GetWeaponSystemComponent(const FGameplayAbilityActorInfo* ActorInfo) const
{
	APX_Character* Character = GetPXCharacter(ActorInfo);
	if ( !Character ) return nullptr;

	return Character->GetWeaponSystemComponent();
}
*/



