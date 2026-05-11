// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Tags/PX_GameplayTags.h"

namespace PX_GameplayTags
{
	// --- Input -----------------------------------------------------
	UE_DEFINE_GAMEPLAY_TAG(Input_Native_Move, "Input.Native.Move");
	UE_DEFINE_GAMEPLAY_TAG(Input_Native_Look, "Input.Native.Look");
	UE_DEFINE_GAMEPLAY_TAG(Input_Native_ToggleInventory, "Input.Native.ToggleInventory");
	UE_DEFINE_GAMEPLAY_TAG(Input_Locomotion_Jump, "Input.Locomotion.Jump");
	UE_DEFINE_GAMEPLAY_TAG(Input_Locomotion_Crouch, "Input.Locomotion.Crouch");
	UE_DEFINE_GAMEPLAY_TAG(Input_Locomotion_Walk, "Input.Locomotion.Walk");
	UE_DEFINE_GAMEPLAY_TAG(Input_Locomotion_Sprint, "Input.Locomotion.Sprint");
	UE_DEFINE_GAMEPLAY_TAG(Input_Interact, "Input.Interact");
	UE_DEFINE_GAMEPLAY_TAG(Input_Skill_E, "Input.Skill.E");
	UE_DEFINE_GAMEPLAY_TAG(Input_Combat_Attack, "Input.Combat.Attack");
	UE_DEFINE_GAMEPLAY_TAG(Input_Combat_Attack_Bow, "Input.Combat.Attack.Bow");
	UE_DEFINE_GAMEPLAY_TAG(Input_Combat_Attack_Gun, "Input.Combat.Attack.Gun");
	UE_DEFINE_GAMEPLAY_TAG(Input_Combat_Reload, "Input.Combat.Reload");
	UE_DEFINE_GAMEPLAY_TAG(Input_Combat_SwitchAttackMode, "Input.Combat.SwitchAttackMode");
	UE_DEFINE_GAMEPLAY_TAG(Input_Combat_ImbueBurn, "Input.Combat.Imbue.Burn");
	UE_DEFINE_GAMEPLAY_TAG(Input_Combat_ImbueSlow, "Input.Combat.Imbue.Slow");
	UE_DEFINE_GAMEPLAY_TAG(Input_Combat_ImbueStun, "Input.Combat.Imbue.Stun");
	UE_DEFINE_GAMEPLAY_TAG(Input_Combat_Aim, "Input.Combat.Aim");
	UE_DEFINE_GAMEPLAY_TAG(Input_Combat_Aim_ADS, "Input.Combat.Aim.ADS");
	UE_DEFINE_GAMEPLAY_TAG(Input_Combat_Aim_OTS, "Input.Combat.Aim.OTS");
	UE_DEFINE_GAMEPLAY_TAG(Input_Equip_BareHand, "Input.Equip.BareHand");
	UE_DEFINE_GAMEPLAY_TAG(Input_Equip, "Input.Equip");
	UE_DEFINE_GAMEPLAY_TAG(Input_Equip_Slot1, "Input.Equip.Slot1");
	UE_DEFINE_GAMEPLAY_TAG(Input_Equip_Slot2, "Input.Equip.Slot2");
	UE_DEFINE_GAMEPLAY_TAG(Input_Equip_Slot3, "Input.Equip.Slot3");
	UE_DEFINE_GAMEPLAY_TAG(Input_Equip_Slot4, "Input.Equip.Slot4");

	// --- Ability ---------------------------------------------------
	UE_DEFINE_GAMEPLAY_TAG(Ability_Interaction_Interact, "Ability.Interaction.Interact");

	UE_DEFINE_GAMEPLAY_TAG(Ability_Movement_Jump, "Ability.Movement.Jump");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Movement_Crouch, "Ability.Movement.Crouch");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Movement_Walk, "Ability.Movement.Walk");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Movement_Sprint, "Ability.Movement.Sprint");

	UE_DEFINE_GAMEPLAY_TAG(Ability_Weapon_Equip, "Ability.Weapon.Equip");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Weapon_Unequip, "Ability.Weapon.Unequip");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Weapon_Switch, "Ability.Weapon.Switch");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Weapon_Attack, "Ability.Weapon.Attack");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Weapon_Attack_Gun, "Ability.Weapon.Attack.Gun");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Weapon_SwitchAttackMode, "Ability.Weapon.SwitchAttackMode");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Weapon_Reload, "Ability.Weapon.Reload");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Weapon_Aim, "Ability.Weapon.Aim");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Weapon_Aim_ADS, "Ability.Weapon.Aim.ADS");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Weapon_Aim_OTS, "Ability.Weapon.Aim.OTS");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Weapon_ImbueBurn, "Ability.Weapon.Imbue.Burn");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Weapon_ImbueSlow, "Ability.Weapon.Imbue.Slow");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Weapon_ImbueStun, "Ability.Weapon.Imbue.Stun");

	// --- Event -----------------------------------------------------
	UE_DEFINE_GAMEPLAY_TAG(Event_Weapon_Equip_Begin, "Event.Weapon.Equip.Begin");
	UE_DEFINE_GAMEPLAY_TAG(Event_Weapon_Equip_End, "Event.Weapon.Equip.End");
	UE_DEFINE_GAMEPLAY_TAG(Event_Weapon_Reload_End, "Event.Weapon.Reload.End");

	// --- State -----------------------------------------------------
	UE_DEFINE_GAMEPLAY_TAG(State_Locomotion_Jogging, "State.Locomotion.Jogging");
	UE_DEFINE_GAMEPLAY_TAG(State_Locomotion_Crouching, "State.Locomotion.Crouching");
	UE_DEFINE_GAMEPLAY_TAG(State_Locomotion_Walking, "State.Locomotion.Walking");
	UE_DEFINE_GAMEPLAY_TAG(State_Locomotion_Sprinting, "State.Locomotion.Sprinting");
	UE_DEFINE_GAMEPLAY_TAG(State_Locomotion_Jumping, "State.Locomotion.Jumping");
	UE_DEFINE_GAMEPLAY_TAG(State_Locomotion_Falling, "State.Locomotion.Falling");
	UE_DEFINE_GAMEPLAY_TAG(State_Locomotion_Swimming, "State.Locomotion.Swimming");

	UE_DEFINE_GAMEPLAY_TAG(State_Action_Interacting, "State.Action.Interacting");

	UE_DEFINE_GAMEPLAY_TAG(State_Combat_Aiming, "State.Combat.Aiming");
	UE_DEFINE_GAMEPLAY_TAG(State_Combat_Aiming_HipFire, "State.Combat.AimMode.HipFire");
	UE_DEFINE_GAMEPLAY_TAG(State_Combat_Aiming_ADS, "State.Combat.AimMode.ADS");
	UE_DEFINE_GAMEPLAY_TAG(State_Combat_Aiming_OTS, "State.Combat.AimMode.OTS");
	UE_DEFINE_GAMEPLAY_TAG(State_Combat_Attacking, "State.Combat.Attacking");
	UE_DEFINE_GAMEPLAY_TAG(State_Combat_Attacking_Gun, "State.Combat.Attacking.Gun");
	UE_DEFINE_GAMEPLAY_TAG(State_Combat_SwitchAttackMode, "State.Combat.SwitchAttackMode");
	UE_DEFINE_GAMEPLAY_TAG(State_Combat_Reloading, "State.Combat.Reloading");
	UE_DEFINE_GAMEPLAY_TAG(State_Combat_Equipping, "State.Combat.Equipping");
	UE_DEFINE_GAMEPLAY_TAG(State_Combat_Unequipping, "State.Combat.Unequipping");
	UE_DEFINE_GAMEPLAY_TAG(State_Combat_SwappingWeapon, "State.Combat.SwappingWeapon");
	UE_DEFINE_GAMEPLAY_TAG(State_Combat_HitReact, "State.Combat.HitReact");
	UE_DEFINE_GAMEPLAY_TAG(State_Combat_Reviving, "State.Combat.Reviving");

	UE_DEFINE_GAMEPLAY_TAG(State_Condition_Dead, "State.Condition.Dead");
	UE_DEFINE_GAMEPLAY_TAG(State_Condition_Down, "State.Condition.Down");
	UE_DEFINE_GAMEPLAY_TAG(State_Condition_Disabled, "State.Condition.Disabled");
	UE_DEFINE_GAMEPLAY_TAG(State_Condition_Invincible, "State.Condition.Invincible");
	UE_DEFINE_GAMEPLAY_TAG(State_Condition_Stunned, "State.Condition.Stunned");
	UE_DEFINE_GAMEPLAY_TAG(State_Condition_Silenced, "State.Condition.Silenced");
	UE_DEFINE_GAMEPLAY_TAG(State_Condition_Bleeding, "State.Condition.Bleeding");
	UE_DEFINE_GAMEPLAY_TAG(State_Condition_Burning, "State.Condition.Burning");
	UE_DEFINE_GAMEPLAY_TAG(State_Condition_Slowed, "State.Condition.Slowed");
	UE_DEFINE_GAMEPLAY_TAG(State_Condition_Poisoned, "State.Condition.Poisoned");
	UE_DEFINE_GAMEPLAY_TAG(State_Condition_Paralyzed, "State.Condition.Paralyzed");

	// --- Cooldown --------------------------------------------------
	UE_DEFINE_GAMEPLAY_TAG(Cooldown_Movement_Jump, "Cooldown.Movement.Jump");

	UE_DEFINE_GAMEPLAY_TAG(Cooldown_Weapon_Attack, "Cooldown.Weapon.Attack");
	UE_DEFINE_GAMEPLAY_TAG(Cooldown_Weapon_Reload, "Cooldown.Weapon.Reload");
	UE_DEFINE_GAMEPLAY_TAG(Cooldown_Weapon_Switch, "Cooldown.Weapon.Switch");
	UE_DEFINE_GAMEPLAY_TAG(Cooldown_Weapon_ImbueBurn, "Cooldown.Weapon.Imbue.Burn");
	UE_DEFINE_GAMEPLAY_TAG(Cooldown_Weapon_ImbueSlow, "Cooldown.Weapon.Imbue.Slow");
	UE_DEFINE_GAMEPLAY_TAG(Cooldown_Weapon_ImbueStun, "Cooldown.Weapon.Imbue.Stun");

	// --- Item ------------------------------------------------------
	UE_DEFINE_GAMEPLAY_TAG(Item_Category_Equippable, "Item.Category.Equippable");
	UE_DEFINE_GAMEPLAY_TAG(Item_Category_Equippable_Weapon, "Item.Category.Equippable.Weapon");
	UE_DEFINE_GAMEPLAY_TAG(Item_Category_Equippable_Armor, "Item.Category.Equippable.Armor");
	UE_DEFINE_GAMEPLAY_TAG(Item_Category_Equippable_Accessory, "Item.Category.Equippable.Accessory");
	UE_DEFINE_GAMEPLAY_TAG(Item_Category_Consumable, "Item.Category.Consumable");
	UE_DEFINE_GAMEPLAY_TAG(Item_Category_Ammo, "Item.Category.Ammo");
	UE_DEFINE_GAMEPLAY_TAG(Item_Category_Material, "Item.Category.Material");
	UE_DEFINE_GAMEPLAY_TAG(Item_Category_Quest, "Item.Category.Quest");

	UE_DEFINE_GAMEPLAY_TAG(Item_Inventory_BareHand, "Item.Inventory.BareHand");
	UE_DEFINE_GAMEPLAY_TAG(Item_Inventory_Weapon, "Item.Inventory.Weapon");
	UE_DEFINE_GAMEPLAY_TAG(Item_Inventory_Armor, "Item.Inventory.Armor");
	UE_DEFINE_GAMEPLAY_TAG(Item_Inventory_Accessory, "Item.Inventory.Accessory");
	UE_DEFINE_GAMEPLAY_TAG(Item_Inventory_General, "Item.Inventory.General");

	UE_DEFINE_GAMEPLAY_TAG(Item_Type_Stackable, "Item.Type.Stackable");
	UE_DEFINE_GAMEPLAY_TAG(Item_Type_Droppable, "Item.Type.Droppable");
	UE_DEFINE_GAMEPLAY_TAG(Item_Type_NotDroppable, "Item.Type.NotDroppable");
	UE_DEFINE_GAMEPLAY_TAG(Item_Type_Exchangeable, "Item.Type.Exchangeable");
	UE_DEFINE_GAMEPLAY_TAG(Item_Type_NotExchangeable, "Item.Type.NotExchangeable");

	UE_DEFINE_GAMEPLAY_TAG(Item_EquipSlot_BareHand, "Item.EquipSlot.BareHand");
	UE_DEFINE_GAMEPLAY_TAG(Item_EquipSlot_Head, "Item.EquipSlot.Head");
	UE_DEFINE_GAMEPLAY_TAG(Item_EquipSlot_Chest, "Item.EquipSlot.Chest");
	UE_DEFINE_GAMEPLAY_TAG(Item_EquipSlot_Legs, "Item.EquipSlot.Legs");
	UE_DEFINE_GAMEPLAY_TAG(Item_EquipSlot_Shoes, "Item.EquipSlot.Shoes");

	// --- Weapon ----------------------------------------------------
	UE_DEFINE_GAMEPLAY_TAG(Weapon_Type_Gun, "Weapon.Type.Gun");
	UE_DEFINE_GAMEPLAY_TAG(Weapon_Type_Gun_Pistol, "Weapon.Type.Gun.Pistol");
	UE_DEFINE_GAMEPLAY_TAG(Weapon_Type_Gun_Rifle, "Weapon.Type.Gun.Rifle");
	UE_DEFINE_GAMEPLAY_TAG(Weapon_Type_Gun_Shotgun, "Weapon.Type.Gun.Shotgun");
	UE_DEFINE_GAMEPLAY_TAG(Weapon_Type_Gun_SMG, "Weapon.Type.Gun.SMG");
	UE_DEFINE_GAMEPLAY_TAG(Weapon_Type_Gun_Sniper, "Weapon.Type.Gun.Sniper");

	UE_DEFINE_GAMEPLAY_TAG(Weapon_Type_Bow, "Weapon.Type.Bow");

	UE_DEFINE_GAMEPLAY_TAG(Weapon_Type_Melee, "Weapon.Type.Melee");
	UE_DEFINE_GAMEPLAY_TAG(Weapon_Type_Melee_OneHandedSword, "Weapon.Type.Melee.OneHandedSword");
	UE_DEFINE_GAMEPLAY_TAG(Weapon_Type_Melee_TwoHandedSword, "Weapon.Type.Melee.TwoHandedSword");
	UE_DEFINE_GAMEPLAY_TAG(Weapon_Type_Melee_Spear, "Weapon.Type.Melee.Spear");

	UE_DEFINE_GAMEPLAY_TAG(Weapon_Type_Throwing, "Weapon.Type.Throwing");
	UE_DEFINE_GAMEPLAY_TAG(Weapon_Type_Throwing_Frag, "Weapon.Type.Throwing.Frag");
	UE_DEFINE_GAMEPLAY_TAG(Weapon_Type_Throwing_Smoke, "Weapon.Type.Throwing.Smoke");

	UE_DEFINE_GAMEPLAY_TAG(Weapon_AttackMode_Single, "Weapon.AttackMode.Single");
	UE_DEFINE_GAMEPLAY_TAG(Weapon_AttackMode_Burst, "Weapon.AttackMode.Burst");
	UE_DEFINE_GAMEPLAY_TAG(Weapon_AttackMode_Auto, "Weapon.AttackMode.Auto");
	UE_DEFINE_GAMEPLAY_TAG(Weapon_AttackMode_Charge, "Weapon.AttackMode.Charge");
	UE_DEFINE_GAMEPLAY_TAG(Weapon_AttackMode_Combo, "Weapon.AttackMode.Combo");
}
