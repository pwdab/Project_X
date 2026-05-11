// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/Inventory/PX_WeaponItemInstance.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/Abilities/PX_GA_WeaponStatusImbue.h"
#include "AbilitySystem/Tags/PX_GameplayTags.h"
#include "Component/Inventory/PX_ItemDataAsset.h"
#include "Component/Inventory/PX_EquippableItemDataAsset.h"
#include "Component/Weapon/PX_WeaponDataAsset.h"
#include "Entity/PX_Item.h"

void UPX_WeaponItemInstance::InitializeFromData(UPX_ItemDataAsset* InItemDataAsset, int32 InQuantity)
{
	Super::InitializeFromData(InItemDataAsset, InQuantity);

	const UPX_WeaponDataAsset* WeaponData = GetWeaponDataAsset();
	if (!WeaponData) return;

	AmmoInMag = 0;
	AttackModeTag = WeaponData->DefaultAttackModeTag;
	//Durability = WeaponData->Durability;
}

/*
UPX_WeaponItemInstance* UPX_WeaponItemInstance::ServerClone(UObject* NewOuter) const
{

	UPX_WeaponItemInstance* NewItemInstance = NewObject<UPX_WeaponItemInstance>(NewOuter, GetClass());
	NewItemInstance->ServerInitializeFromData(ItemDataAsset, Quantity);

	// 동적 데이터는 현재값 복사
	NewItemInstance->AmmoInMag = AmmoInMag;
	NewItemInstance->AttackMode = AttackMode;
	NewItemInstance->Durability = Durability;

	return NewItemInstance;
}
*/

FPX_ItemData UPX_WeaponItemInstance::MakeDropData() const
{
	FPX_ItemData ItemData = Super::MakeDropData();
	ItemData.AmmoInMag = AmmoInMag;
	ItemData.AttackModeTag = AttackModeTag;
	ItemData.Durability = Durability;

	return ItemData;
}

void UPX_WeaponItemInstance::ApplyDropData(const FPX_ItemData& Data)
{
	//ServerInitializeFromData(Data.ItemDataAsset, Data.Quantity);

	InstanceId = Data.InstanceId;
	Quantity = Data.Quantity;
	ItemDataAsset = Data.ItemDataAsset;
	AmmoInMag = Data.AmmoInMag;
	AttackModeTag = Data.AttackModeTag;
	Durability = Data.Durability;
}

void UPX_WeaponItemInstance::GiveAbilities(UAbilitySystemComponent* ASC)
{
	Super::GiveAbilities(ASC);

	if ( !ASC )
	{
		return;
	}

	const TSubclassOf<UGameplayAbility> StatusImbueAbilityClass = GetWeaponStatusImbueAbilityClass();
	if ( !StatusImbueAbilityClass )
	{
		return;
	}

	TArray<FGameplayAbilitySpecHandle> StatusImbueAbilityHandlesToClear;
	for ( const FGameplayAbilitySpec& ExistingSpec : ASC->GetActivatableAbilities() )
	{
		if ( ExistingSpec.Ability && ExistingSpec.Ability->GetClass()->IsChildOf(UPX_GA_WeaponStatusImbueBase::StaticClass()) )
		{
			StatusImbueAbilityHandlesToClear.Add(ExistingSpec.Handle);
		}
	}

	for ( const FGameplayAbilitySpecHandle& HandleToClear : StatusImbueAbilityHandlesToClear )
	{
		ASC->ClearAbility(HandleToClear);
		GivenAbilityHandles.Remove(HandleToClear);
	}

	for ( const FGameplayAbilitySpecHandle& ExistingHandle : GivenAbilityHandles )
	{
		if ( const FGameplayAbilitySpec* ExistingSpec = ASC->FindAbilitySpecFromHandle(ExistingHandle) )
		{
			if ( ExistingSpec->Ability && ExistingSpec->Ability->GetClass()->IsChildOf(StatusImbueAbilityClass) )
			{
				return;
			}
		}
	}

	FGameplayAbilitySpec Spec(StatusImbueAbilityClass, 1, INDEX_NONE, this);
	Spec.DynamicAbilityTags.AddTag(PX_GameplayTags::Input_Skill_E);

	const FGameplayAbilitySpecHandle Handle = ASC->GiveAbility(Spec);
	if ( Handle.IsValid() )
	{
		GivenAbilityHandles.Add(Handle);
	}
}

TSubclassOf<UGameplayAbility> UPX_WeaponItemInstance::GetWeaponStatusImbueAbilityClass() const
{
	const UPX_EquippableItemDataAsset* EquippableDataAsset = GetEquippableItemDataAsset();
	if ( !EquippableDataAsset )
	{
		return nullptr;
	}

	const FGameplayTagContainer& GrantedTags = EquippableDataAsset->GrantedTags;
	if ( GrantedTags.HasTag(PX_GameplayTags::Weapon_Type_Gun_Pistol) )
	{
		return UPX_GA_ImbueSlow::StaticClass();
	}
	if ( GrantedTags.HasTag(PX_GameplayTags::Weapon_Type_Gun_Rifle) )
	{
		return UPX_GA_ImbueBurn::StaticClass();
	}
	if ( GrantedTags.HasTag(PX_GameplayTags::Weapon_Type_Gun_Shotgun) )
	{
		return UPX_GA_ImbueStun::StaticClass();
	}

	return nullptr;
}

void UPX_WeaponItemInstance::SetAmmo(int32 Amount)
{
	const UPX_WeaponDataAsset* WeaponData = GetWeaponDataAsset();
	if (!WeaponData) return;

	AmmoInMag = FMath::Clamp(Amount, 0, WeaponData->MagSize);
}

void UPX_WeaponItemInstance::ConsumeAmmo(int32 Amount)
{
	if ( Amount <= 0 ) return;

	const UPX_WeaponDataAsset* WeaponData = GetWeaponDataAsset();
	if (!WeaponData) return;

	AmmoInMag = FMath::Clamp(AmmoInMag - Amount, 0, WeaponData->MagSize);
}

bool UPX_WeaponItemInstance::SwitchAttackMode()
{
	const UPX_WeaponDataAsset* WeaponData = GetWeaponDataAsset();
	if ( !WeaponData ) return false;
	if ( WeaponData->SupportedAttackModeTags.Num() == 0 ) return false;

	const int32 CurrentIndex = WeaponData->SupportedAttackModeTags.IndexOfByKey(AttackModeTag);
	const int32 NextIndex = (CurrentIndex == INDEX_NONE) ? 0 : (CurrentIndex + 1) % WeaponData->SupportedAttackModeTags.Num();

	return SetAttackMode(WeaponData->SupportedAttackModeTags[NextIndex]);
}

bool UPX_WeaponItemInstance::SetAttackMode(FGameplayTag NewAttackModeTag)
{
	if ( !NewAttackModeTag.IsValid() ) return false;

	const UPX_WeaponDataAsset* WeaponData = GetWeaponDataAsset();
	if ( !WeaponData ) return false;

	if ( !WeaponData->SupportedAttackModeTags.Contains(NewAttackModeTag) ) return false;
	if ( AttackModeTag == NewAttackModeTag ) return false;

	AttackModeTag = NewAttackModeTag;

	return true;
}
