#pragma once

#include "Project_X.h"
#include "Component/Inventory/PX_ItemDataAsset.h"
#include "GameplayAbilitySpec.h"
#include "PX_EquippableItemDataAsset.generated.h"

USTRUCT(BlueprintType)
struct FPX_GrantedAbilityEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TSubclassOf<UGameplayAbility> AbilityClass;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FGameplayTag InputTag;
};

UCLASS()
class PROJECT_X_API UPX_EquippableItemDataAsset : public UPX_ItemDataAsset
{
    GENERATED_BODY()

public:
    // --- Data Variables ---------------------------------------------------
    // EquippableItem Type Tag
    UPROPERTY(EditDefaultsOnly, Category = "Equippable")
    FGameplayTagContainer GrantedTags;

    // 장착 시 부여할 Ability + InputTag들
    UPROPERTY(EditDefaultsOnly, Category = "Equippable")
    TArray<FPX_GrantedAbilityEntry> GrantedAbilities;

	// 이 슬롯에만 장착해야 함. (예: Head, Chest, Legs, Shoes)
    UPROPERTY(EditDefaultsOnly, Category = "Equippable")
    FGameplayTag ItemEquipSlotTag;

};